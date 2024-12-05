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
int ensure(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator);
int allocate(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator);
void spill(int pr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int* offset, int* name);
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
int allocate(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator) {
    for (int pr = 0; pr < MAX_PHYSICAL_REGS; pr++) {
        if (name[pr] == INVALID) {
            name[pr] = vr;
            return pr;
        }
    }

    int max_dist = -1, spill_pr = 0;
    for (int pr = 0; pr < MAX_PHYSICAL_REGS; pr++) {
        int current_vr = name[pr];
        int distance = dist(current_vr, prev_insn->next);
        if (distance > max_dist) {
            max_dist = distance;
            spill_pr = pr;
        }
    }

    spill(spill_pr, prev_insn, local_allocator, offset, name);
    name[spill_pr] = vr;
    return spill_pr;
}

// Spill a physical register to the stack
void spill(int pr, ILOCInsn* prev_insn, ILOCInsn* local_allocator, int* offset, int* name) {
    int vr = name[pr];
    offset[vr] = insert_spill(pr, prev_insn, local_allocator);
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
        // Track the local allocator
        if (i->op[0].type == CALL_LABEL) {
            local_allocator = i;
        }

        // Ensure all read registers are reloaded if spilled
        ILOCInsn* read_regs = ILOCInsn_get_read_registers(i);
        for (int op = 0; op < 3; op++) {
            Operand vr = read_regs->op[op];
            if (vr.type == VIRTUAL_REG) {
                int pr = ensure(vr.id, prev_insn, local_allocator);
                replace_register(vr.id, pr, i);

                if (dist(vr.id, i->next) == INT_MAX) {
                    name[pr] = INVALID;
                }
            }
        }
        free(read_regs);

        // Allocate and reload write registers if necessary
        Operand vr = ILOCInsn_get_write_register(i);
        if (vr.type == VIRTUAL_REG) {
            int pr = allocate(vr.id, prev_insn, local_allocator);
            replace_register(vr.id, pr, i);
        }

        // spill any live registers before procedure calls
        // if i is a CALL instruction:
        //     for each pr where name[pr] != INVALID:
        //         spill(pr)

        if (i->op->type == CALL) {
            for (int pr = 0; pr < MAX_PHYSICAL_REGS; pr++) {
                if (name[pr] != INVALID) {
                    spill(pr, prev_insn, local_allocator, offset, name);
                }
            }
            
            // Reload spilled registers after a CALL if they are needed again
            for (int vr = 0; vr < MAX_VIRTUAL_REGS; vr++) {
                if (offset[vr] != INVALID) {  
                    int pr = ensure(vr, prev_insn, local_allocator);
                }
            }
        }
        
        prev_insn = i;
    }
}




// Reset the register mappings and offsets
void reset_mappings(int num_physical_registers) {
    for (int i = 0; i < MAX_PHYSICAL_REGS; i++) {
        name[i] = INVALID;  // Mark all registers as free
    }
    for (int i = 0; i < MAX_VIRTUAL_REGS; i++) {
        offset[i] = INVALID;  // Initialize all offsets to invalid
    }
}


// Ensure a virtual register is in a physical register
int ensure(int vr, ILOCInsn* prev_insn, ILOCInsn* local_allocator) {
    for (int pr = 0; pr < MAX_PHYSICAL_REGS; pr++) {
        if (name[pr] == vr) {
            return pr;  
        }
    }

    int pr = allocate(vr, prev_insn, local_allocator);
    if (offset[vr] != INVALID) {
        insert_load(offset[vr], pr, prev_insn);
    }

    return pr;
}





