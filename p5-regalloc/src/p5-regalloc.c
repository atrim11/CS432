/**
 * @file p5-regalloc.c
 * @brief Compiler phase 5: register allocation
 */
#include "p5-regalloc.h"
#include <limits.h>

#define INVALID -1
#define MAX_PHYSICAL_REGS 4
#define INFINITY INT_MAX

// Global and local data structures
int name[MAX_PHYSICAL_REGS];  // Map physical register ID to virtual register ID
int offset[MAX_VIRTUAL_REGS]; // Map virtual register ID to stack offset

//
void reset_mappings(int num_physical_registers);
int ensure(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int num_physical_registers);
int allocate(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int num_physical_registers);
void spill(int pr, ILOCInsn* prev_insn, ILOCInsn* local_allocator);
int dist(int vr, ILOCInsn* current);


/**
 * @brief Replace a virtual register id with a physical register id
 * 
 * Every virtual register operand with ID "vr" will be replaced by a physical
 * register operand with ID "pr" in the given instruction.
 * 
 * @param vr Virtual register id that should be replaced
 * @param pr Physical register id that it should be replaced by
 * @param isnsn Instruction to modify
 */
void replace_register(int vr, int pr, ILOCInsn* insn)
{
    for (int i = 0; i < 3; i++) {
        if (insn->op[i].type == VIRTUAL_REG && insn->op[i].id == vr) {
            insn->op[i].type = PHYSICAL_REG;
            insn->op[i].id = pr;
        }
    }
}

/**
 * @brief Insert a store instruction to spill a register to the stack
 * 
 * We need to allocate a new slot in the stack from for the current
 * function, so we need a reference to the local allocator instruction.
 * This instruction will always be the third instruction in a function
 * and will be of the form "add SP, -X => SP" where X is the current
 * stack frame size.
 * 
 * @param pr Physical register id that should be spilled
 * @param prev_insn Reference to an instruction; the new instruction will be
 * inserted directly after this one
 * @param local_allocator Reference to the local frame allocator instruction
 * @returns BP-based offset where the register was spilled
 */
int insert_spill(int pr, ILOCInsn* prev_insn, ILOCInsn* local_allocator)
{
    /* adjust stack frame size to add new spill slot */
    int bp_offset = local_allocator->op[1].imm - WORD_SIZE;
    local_allocator->op[1].imm = bp_offset;

    /* create store instruction */
    ILOCInsn* new_insn = ILOCInsn_new_3op(STORE_AI,
            physical_register(pr), base_register(), int_const(bp_offset));

    /* insert into code */
    new_insn->next = prev_insn->next;
    prev_insn->next = new_insn;

    return bp_offset;
}

/**
 * @brief Insert a load instruction to load a spilled register
 * 
 * @param bp_offset BP-based offset where the register value is spilled
 * @param pr Physical register where the value should be loaded
 * @param prev_insn Reference to an instruction; the new instruction will be
 * inserted directly after this one
 */
void insert_load(int bp_offset, int pr, ILOCInsn* prev_insn)
{
    /* create load instruction */
    ILOCInsn* new_insn = ILOCInsn_new_3op(LOAD_AI,
            base_register(), int_const(bp_offset), physical_register(pr));

    /* insert into code */
    new_insn->next = prev_insn->next;
    prev_insn->next = new_insn;
}

// Allocate a physical register for a virtual register
int allocate(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int num_physical_registers) {
    
    for (int pr = 0; pr < num_physical_registers; pr++) {
        if (name[pr] == INVALID) {
            name[pr] = vr;
            return pr;
        }
    }
        //     find pr that maximizes dist(name[pr])   // otherwise, find register to spill
        // spill(pr)                               // spill value to stack
        // name[pr] = vr                           // reallocate it
        // return pr                               // and use it
    int spill_pr = 0;
    int max_dist = 0;
    for (int pr = 0; pr < num_physical_registers; pr++) {
        int d = dist(name[pr], prev_insn);
        // find pr that maximizes dist(name[pr])  
        // otherwise, find register to spill
        if (d > max_dist) {
            max_dist = d;
            spill_pr = pr;
        }
    }

    spill(spill_pr, prev_insn, local_allocator);
    name[spill_pr] = vr;
    return spill_pr;
}

// spill(pr):
    // emit store from pr onto the stack at some offset x
    // offset[name[pr]] = x
    // name[pr] = INVALID
void spill(int pr, ILOCInsn* prev_insn, ILOCInsn* local_allocator) {
    int vr = name[pr];
    int bp_offset = insert_spill(pr, prev_insn, local_allocator);
    offset[vr] = bp_offset;
    name[pr] = INVALID;
}


int dist(int vr, ILOCInsn* current) {
    int distance = 0;
    for (ILOCInsn* i = current; i != NULL; i = i->next, distance++) {
        ILOCInsn* read_regs = ILOCInsn_get_read_registers(i);
        for (int op = 0; op < 3; op++) {
            if (read_regs->op[op].type == VIRTUAL_REG && read_regs->op[op].id == vr) {
                // free(read_regs);
                return distance;  
            }
        }
        free(read_regs);
    }
    return INT_MAX;  
}

void allocate_registers(InsnList* list, int num_physical_registers) {
    if (list == NULL) {
        return list;
    }

    reset_mappings(num_physical_registers);
    ILOCInsn* local_allocator = NULL;
    ILOCInsn* prev_insn = NULL;

    FOR_EACH(ILOCInsn*, i, list) {
        //    save reference to stack allocator instruction if i is a call label
        if (i->form == CALL_LABEL) {
            local_allocator = i->next->next->next;
        }

        //         for each read vr in i:
        // pr = ensure(vr)                     // make sure vr is in a phys reg
        // replace vr with pr in i             // change register id
        // if dist(vr) == INFINITY:            // if no future use
        //     name[pr] = INVALID              // then free pr
        ILOCInsn* read_regs = ILOCInsn_get_read_registers(i);
        for (int op = 0; op < 3; op++) {
            Operand vr = read_regs->op[op];
            if (vr.type == VIRTUAL_REG) {
                int pr = ensure(vr.id, prev_insn, local_allocator, num_physical_registers);
                replace_register(vr.id, pr, i);

                if (dist(vr.id, i->next) == INFINITY) {
                    name[pr] = INVALID;
                }
            }
        }
        free(read_regs);

        // for each written vr in i:
        for (int op = 0; op < 3; op++) {
            Operand vr = i->op[op];
            if (vr.type == VIRTUAL_REG) {
                int pr = allocate(vr.id, prev_insn, local_allocator, num_physical_registers);
                replace_register(vr.id, pr, i);
            }
        }

        // spill any live registers before procedure calls
        // if i is a CALL instruction:
        //     for each pr where name[pr] != INVALID:
        //         spill(pr)

        // Recursiveness Check
        if (i->form == CALL) {
            for (int pr = 0; pr < num_physical_registers; pr++) {
                if (name[pr] != INVALID) {
                    spill(pr, prev_insn, local_allocator);
                }
            }
        }
        // save reference to i to facilitate spilling before next instruction
        prev_insn = i;
    }
}


// Reset the register mappings and offsets
void reset_mappings(int num_physical_registers) {
    for (int i = 0; i < num_physical_registers; i++) {
        name[i] = INVALID;  // Mark all registers as free
    }
    for (int i = 0; i < num_physical_registers; i++) {
        offset[i] = INVALID;  // Initialize all offsets to invalid
    }
}


// Ensure a virtual register is in a physical register
int ensure(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int num_physical_registers) {
    for (int pr = 0; pr < num_physical_registers; pr++) {
        if (name[pr] == vr) {
            return pr;  
        }
    }

    int pr = allocate(vr, prev_insn, local_allocator, num_physical_registers);
    // if offset[vr] is valid:                 // if vr was spilled, load it
    if (offset[vr] != INVALID) {
        insert_load(offset[vr], pr, prev_insn);
    }

    return pr;
}





