/**
 * @file p4-codegen.c
 * @brief Compiler phase 4: code generation
 */
#include "p4-codegen.h"

/**
 * @brief State/data for the code generator visitor
 */
typedef struct CodeGenData
{
    /**
     * @brief Reference to the epilogue jump label for the current function
     */
    Operand current_epilogue_jump_label;

    Operand loop_label;
    Operand body_label;
    Operand end_label;


    /* add any new desired state information (and clean it up in CodeGenData_free) */
} CodeGenData;

/**
 * @brief Allocate memory for code gen data
 * 
 * @returns Pointer to allocated structure
 */
CodeGenData* CodeGenData_new (void)
{
    CodeGenData* data = (CodeGenData*)calloc(1, sizeof(CodeGenData));
    CHECK_MALLOC_PTR(data);
    data->current_epilogue_jump_label = empty_operand();
    return data;
}

/**
 * @brief Deallocate memory for code gen data
 * 
 * @param data Pointer to the structure to be deallocated
 */
void CodeGenData_free (CodeGenData* data)
{
    /* free everything in data that is allocated on the heap */

    /* free "data" itself */
    free(data);
}

/**
 * @brief Macro for more convenient access to the error list inside a @c visitor
 * data structure
 */
#define DATA ((CodeGenData*)visitor->data)

/**
 * @brief Fills a register with the base address of a variable.
 * 
 * @param node AST node to emit code into (if needed)
 * @param variable Desired variable
 * @returns Virtual register that contains the base address
 */
Operand var_base (ASTNode* node, Symbol* variable)
{
    Operand reg = empty_operand();
    switch (variable->location) {
        case STATIC_VAR:
            reg = virtual_register();
            ASTNode_emit_insn(node,
                    ILOCInsn_new_2op(LOAD_I, int_const(variable->offset), reg));
            break;
        case STACK_PARAM:
        case STACK_LOCAL:
            reg = base_register();
            break;
        default:
            break;
    }
    return reg;
}

/**
 * @brief Calculates the offset of a scalar variable reference and fills a register with that offset.
 * 
 * @param node AST node to emit code into (if needed)
 * @param variable Desired variable
 * @returns Virtual register that contains the base address
 */
Operand var_offset (ASTNode* node, Symbol* variable)
{
    Operand op = empty_operand();
    switch (variable->location) {
        case STATIC_VAR:    op = int_const(0); break;
        case STACK_PARAM:
        case STACK_LOCAL:   op = int_const(variable->offset);
        default:
            break;
    }
    return op;
}

#ifndef SKIP_IN_DOXYGEN

/*
 * Macros for more convenient instruction generation
 */

#define EMIT0OP(FORM)             ASTNode_emit_insn(node, ILOCInsn_new_0op(FORM))
#define EMIT1OP(FORM,OP1)         ASTNode_emit_insn(node, ILOCInsn_new_1op(FORM,OP1))
#define EMIT2OP(FORM,OP1,OP2)     ASTNode_emit_insn(node, ILOCInsn_new_2op(FORM,OP1,OP2))
#define EMIT3OP(FORM,OP1,OP2,OP3) ASTNode_emit_insn(node, ILOCInsn_new_3op(FORM,OP1,OP2,OP3))

void CodeGenVisitor_gen_program (NodeVisitor* visitor, ASTNode* node)
{
    /*
     * make sure "code" attribute exists at the program level even if there are
     * no functions (although this shouldn't happen if static analysis is run
     * first); also, don't include a print function here because there's not
     * really any need to re-print all the functions in the program node *
     */
    ASTNode_set_attribute(node, "code", InsnList_new(), (Destructor)InsnList_free);

    /* copy code from each function */
    FOR_EACH(ASTNode*, func, node->program.functions) {
        ASTNode_copy_code(node, func);
    }
}

void CodeGenVisitor_previsit_funcdecl (NodeVisitor* visitor, ASTNode* node)
{
    /* generate a label reference for the epilogue that can be used while
     * generating the rest of the function (e.g., to be used when generating
     * code for a "return" statement) */
    DATA->current_epilogue_jump_label = anonymous_label();
}


void CodeGenVisitor_gen_funcdecl(NodeVisitor* visitor, ASTNode* node)
{
    /* Every function begins with the corresponding call label */
    EMIT1OP(LABEL, call_label(node->funcdecl.name));

    /* Prologue */
    EMIT1OP(PUSH, base_register());                      // push BP
    EMIT2OP(I2I, stack_register(), base_register());     // i2i SP => BP

    /* Check if there are any local variables by looking for 'localSize' */
    int localSize = ASTNode_get_int_attribute(node, "localSize");
    if (localSize > 0) {
        EMIT3OP(ADD_I, stack_register(), int_const(-localSize), stack_register());  // addI SP, -localSize => SP
    }

    /* Generate code for function body */
    ASTNode_copy_code(node, node->funcdecl.body);

    /* Epilogue label */
    EMIT1OP(LABEL, DATA->current_epilogue_jump_label);

    /* Epilogue */
    EMIT2OP(I2I, base_register(), stack_register());     // i2i BP => SP
    EMIT1OP(POP, base_register());                       // pop BP

    /* Return instruction */
    EMIT0OP(RETURN);
}




// void CodeGenVisitor_gen_funcdecl (NodeVisitor* visitor, ASTNode* node)
// {
//     /* every function begins with the corresponding call label */
//     EMIT1OP(LABEL, call_label(node->funcdecl.name));

//     /* Prologue */
//     EMIT1OP(PUSH, base_register());                      // push BP
//     EMIT2OP(I2I, stack_register(), base_register());     // i2i SP => BP
//     EMIT3OP(ADD_I, stack_register(), int_const(0), stack_register());  // addI SP, 0 => SP

//     /* copy code from parameters */
//     // FOR_EACH(Symbol*, param, node->funcdecl.parameters) {
//     //     /* generate code for the parameter */
//     //     Operand reg = base_register();
//     //     ASTNode_emit_insn(node, ILOCInsn_new_2op(LOAD_AI, var_offset(node, param), reg));
//     // }
//     /* Generate code for function body */
//     ASTNode_copy_code(node, node->funcdecl.body);

//     /* Epilogue label */
//     EMIT1OP(LABEL, DATA->current_epilogue_jump_label);
//     /* Epilogue */
//     EMIT2OP(I2I, base_register(), stack_register());     // i2i BP => SP
//     EMIT1OP(POP, base_register());                       // pop BP

//     /* Return instruction */
//     EMIT0OP(RETURN);
// }

void CodeGenVisitor_gen_block (NodeVisitor* visitor, ASTNode* node)
{

    /* copy code from each statement */  
    FOR_EACH(ASTNode*, stmt, node->block.statements) {
        ASTNode_copy_code(node, stmt);
    }  

}

void CodeGenVisitor_gen_return(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the return value, if it exists */
    if (node->funcreturn.value) {
        ASTNode_copy_code(node, node->funcreturn.value);
        
        /* Move the return value to RET */
        Operand result_reg = ASTNode_get_temp_reg(node->funcreturn.value);
        EMIT2OP(I2I, result_reg, return_register());  // Only final result goes to RET
    }

    /* Jump to the function's epilogue to handle cleanup */
    EMIT1OP(JUMP, DATA->current_epilogue_jump_label);
}




void CodeGenVisitor_gen_literal(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the literal value */
    Operand reg = virtual_register(); // Use a temporary register
    EMIT2OP(LOAD_I, int_const(node->literal.integer), reg);
    ASTNode_set_temp_reg(node, reg);  // Set the temp register for the node
}



void CodeGenvisitor_gen_post_binaryop (NodeVisitor* visitor, ASTNode* node)
{
     /* Generate code for the binary operation */
    Operand reg = virtual_register();  // Use a new temporary register
    ASTNode_copy_code(node, node->binaryop.left);
    ASTNode_copy_code(node, node->binaryop.right);
    // Need to have a switch statement to determine the correct binary operation
    BinaryOpType op = node->binaryop.operator;
    switch (op)
    {
    
    case OROP:
        EMIT3OP(OR, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case ANDOP:
        EMIT3OP(AND, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case EQOP:
        EMIT3OP(CMP_EQ, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case NEQOP:
        EMIT3OP(CMP_NE, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case LTOP:
        EMIT3OP(CMP_LT, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case LEOP:
        EMIT3OP(CMP_LE, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case GEOP:
        EMIT3OP(CMP_GE, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case GTOP:
        EMIT3OP(CMP_GT, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case ADDOP:
        EMIT3OP(ADD, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case SUBOP:
        EMIT3OP(SUB, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case MULOP:
        EMIT3OP(MULT, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case DIVOP:
        EMIT3OP(DIV, ASTNode_get_temp_reg(node->binaryop.left), ASTNode_get_temp_reg(node->binaryop.right), reg);
        break;
    case MODOP:
        break;
    default:
        break;
    }

    
    // Set temp register for binary op
    ASTNode_set_temp_reg(node, reg);
}

void CodeGenVisitor_gen_unaryop (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the unary operation */
    Operand reg = virtual_register();
    ASTNode_copy_code(node, node->unaryop.child);
    UnaryOpType op = node->unaryop.operator;
    switch (op)
    {
    case NEGOP:
        EMIT2OP(NEG, ASTNode_get_temp_reg(node->unaryop.child), reg);
        break;
    case NOTOP:
        EMIT2OP(NOT, ASTNode_get_temp_reg(node->unaryop.child), reg);
        break;
    default:
        break;
    }
    ASTNode_set_temp_reg(node, reg);
}

void GenCodeVisitor_gen_pre_while (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the while loop */
    DATA->loop_label = anonymous_label();
    DATA->body_label = anonymous_label();
    DATA->end_label = anonymous_label();
}

void CodeGenVisitor_gen_post_while (NodeVisitor* visitor, ASTNode* node)
{
    EMIT1OP(LABEL, DATA->loop_label);
    ASTNode_copy_code(node, node->whileloop.condition);
    EMIT3OP(CBR, ASTNode_get_temp_reg(node->whileloop.condition), DATA->body_label, DATA->end_label);
    EMIT1OP(LABEL, DATA->body_label);
    ASTNode_copy_code(node, node->whileloop.body);

    // Jumping back to conditinals
    EMIT1OP(JUMP, DATA->loop_label);

    // End of while loop
    EMIT1OP(LABEL, DATA->end_label);
}

void GenCodeVisitor_gen_post_break (NodeVisitor* visitor, ASTNode* node)
{
    EMIT1OP(JUMP, DATA->end_label);
}

void GenCodeVisitor_gen_post_continue (NodeVisitor* visitor, ASTNode* node)
{
    EMIT1OP(JUMP, DATA->loop_label);
}

void CodeGenVisitor_gen_assign(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the RHS expression (value) */
    ASTNode_copy_code(node, node->assignment.value);

    /* Retrieve the result register of the RHS expression */
    Operand rhs_reg = ASTNode_get_temp_reg(node->assignment.value);

    /* Get the base address and offset for the LHS variable (location) */
    Operand lhs_base = base_register();  // BP is the base for stack variables
    Operand lhs_offset = int_const(-8);  // Offset for local variable "a"

    /* Store the RHS result into the LHS variable's location */
    EMIT3OP(STORE_AI, rhs_reg, lhs_base, lhs_offset);  // storeAI r4 => [BP-8]

    /* Set the temporary register for the LHS variable */
    ASTNode_set_temp_reg(node, rhs_reg);  // Set temp reg for assignment node
    ASTNode_set_temp_reg(node->assignment.location, rhs_reg);  // Set temp reg for `a` itself
}





#endif
InsnList* generate_code (ASTNode* tree)
{
    // Null check - A Test Case
    if (tree == NULL) {
        return NULL;
    }

    InsnList* iloc = InsnList_new();

    NodeVisitor* v = NodeVisitor_new();
    v->data = CodeGenData_new();
    v->dtor = (Destructor)CodeGenData_free;
    v->postvisit_program     = CodeGenVisitor_gen_program;
    v->previsit_funcdecl     = CodeGenVisitor_previsit_funcdecl;
    v->postvisit_funcdecl    = CodeGenVisitor_gen_funcdecl;
    v->postvisit_block       = CodeGenVisitor_gen_block;
    v->postvisit_return      = CodeGenVisitor_gen_return;
    v->postvisit_literal     = CodeGenVisitor_gen_literal;
    v->postvisit_binaryop    = CodeGenvisitor_gen_post_binaryop;
    v->postvisit_unaryop     = CodeGenVisitor_gen_unaryop;
    v ->previsit_whileloop   = GenCodeVisitor_gen_pre_while;
    v->postvisit_whileloop   = CodeGenVisitor_gen_post_while;
    v->postvisit_break       = GenCodeVisitor_gen_post_break;
    v->postvisit_continue    = GenCodeVisitor_gen_post_continue;
    v->postvisit_assignment  = CodeGenVisitor_gen_assign;

    /* generate code into AST attributes */
    NodeVisitor_traverse_and_free(v, tree);

    /* copy generated code into new list (the AST may be deallocated before
     * the ILOC code is needed) */
    FOR_EACH(ILOCInsn*, i, (InsnList*)ASTNode_get_attribute(tree, "code")) {
        InsnList_add(iloc, ILOCInsn_copy(i));
    }
    return iloc;
}

