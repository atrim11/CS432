/**
 * @file p5-regalloc.c
 * @brief Compiler phase 5: register allocation
 */
#include "p5-regalloc.h"

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

void allocate_registers (InsnList* list, int num_physical_registers)
{
    // for each instruction i in program:
    FOR_EACH(ILOCInsn*, i, list)
    {
        printf("allocating ");
        ILOCInsn_print(i, stdout);

        // for each read vr in i:
        ILOCInsn* read_regs = ILOCInsn_get_read_registers(i);
        for (int op = 0; op < 3; op++)
        {
            Operand vr = read_regs->op[op];
            if (vr.type == VIRTUAL_REG)
            {
                printf(" read from ");
                Operand_print(vr, stdout);

                //     pr = ensure(vr)                     // make sure vr is in a phys reg
                int pr = vr.id; // TODO: CHANGE THIS CURRENTLY FAKE FROM CLASS
                //     replace vr with pr in i             // change register id
                replace_register(vr.id, pr, i);
                
                // "this is the part that lets us reuse registers"
                //     if dist(vr) == INFINITY:            // if no future use
                //         name[pr] = INVALID              // then free pr
                
            }
        }
        
        free(read_regs);
        
        Operand vr = ILOCInsn_get_write_register(i);
        // for each written vr in i:
        //     pr = allocate(vr)                   // make sure phys reg is available
        int pr = vr.id; // TODO: CHANGE THIS CURRENTLY FAKE FROM CLASS
        //     replace vr with pr in i             // change register id and type
        replace_register(vr.id, pr, i);
        
        printf("\n");
    }
}

void ensure(int vr)
{
    // if name[pr] == vr for some pr:              // if the vr is in a phys reg
    //     return pr                               // then use it
    // else
    //     pr = allocate(vr)                       // otherwise, allocate a phys reg
    //     if offset[vr] is valid:                 // if vr was spilled, load it
    //         emit load into pr from offset[vr]
    //     return pr                               // and use it
}

void allocate(int vr)
{
    //  if name[pr] == INVALID for some pr:         // if there's a free register
    //     name[pr] = vr                           // then allocate it
    //     return pr                               // and use it
    // else:
    //     find pr that maximizes dist(name[pr])   // otherwise, find register to spill
    //     spill(pr)                               // spill value to stack
    //     name[pr] = vr                           // reallocate it
    //     return pr                               // and use it
}

void dist(int vr)
{
    // return number of instructions until vr is next used (INFINITY if no use)
}


// void ensure(int vr)
// {
//     // if name[pr] == vr for some pr:              // if the vr is in a phys reg
//     //     return pr                               // then use it
//     // else
//     //     pr = allocate(vr)                       // otherwise, allocate a phys reg
//     //     if offset[vr] is valid:                 // if vr was spilled, load it
//     //         emit load into pr from offset[vr]
//     //     return pr                               // and use it
// }

// void allocate(int vr)
// {
//     //  if name[pr] == INVALID for some pr:         // if there's a free register
//     //     name[pr] = vr                           // then allocate it
//     //     return pr                               // and use it
//     // else:
//     //     find pr that maximizes dist(name[pr])   // otherwise, find register to spill
//     //     spill(pr)                               // spill value to stack
//     //     name[pr] = vr                           // reallocate it
//     //     return pr                               // and use it
// }

// void dist(int vr)
// {
//     // return number of instructions until vr is next used (INFINITY if no use)
// }