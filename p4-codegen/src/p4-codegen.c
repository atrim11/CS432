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

void CodeGenVisitor_gen_funcdecl (NodeVisitor* visitor, ASTNode* node)
{
    /* every function begins with the corresponding call label */
    EMIT1OP(LABEL, call_label(node->funcdecl.name));

    /* BOILERPLATE: TODO: implement prologue */
    EMIT2OP(I2I,stack_register(), base_register());
    /* copy code from parameters */
    // FOR_EACH(Symbol*, param, node->funcdecl.parameters) {
    //     /* generate code for the parameter */
    //     Operand reg = base_register();
    //     ASTNode_emit_insn(node, ILOCInsn_new_2op(LOAD_AI, var_offset(node, param), reg));
    // }


    /* copy code from body */
    ASTNode_copy_code(node, node->funcdecl.body);

    EMIT1OP(LABEL, DATA->current_epilogue_jump_label);
    /* BOILERPLATE: TODO: implement epilogue */
    EMIT0OP(RETURN);
}

void CodeGenVisitor_gen_block (NodeVisitor* visitor, ASTNode* node)
{

    /* copy code from each statement */  
    FOR_EACH(ASTNode*, stmt, node->block.statements) {
        ASTNode_copy_code(node, stmt);
    }  

}

void CodeGenVisitor_gen_return (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the return value */
    ASTNode_copy_code(node, node->funcreturn.value);

    //TODO: Come back and finish the routine by checking to make sure threi is actually a retunr value before doing the following

    // emit a new i2i instruction that copies the valie from thetemporay registed associated with the return value ast node (use astnode_get tempreg) tp the special RET register (return_register())
    EMIT2OP(I2I, ASTNode_get_temp_reg(node->funcreturn.value), return_register());
}

void CodeGenVisitor_gen_literal (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the literal value */
    Operand reg = virtual_register();
    EMIT2OP(LOAD_I, int_const(node->literal.integer), reg);
    ASTNode_set_temp_reg(node, reg);
}

void CodeGenvisitor_gen_post_binaryop (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the binary operation */
    Operand reg = virtual_register();
    // Copy code from the left and right operands
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
    // /* generate code for the unary operation */
    // Operand reg = virtual_register();
    // ASTNode_copy_code(node, node->unaryop.operator);
    // UnaryOpType op = node->unaryop.operator;
    // switch (op)
    // {
    // case NEGOP:
    //     EMIT3OP(NEG, ASTNode_get_temp_reg(node->unaryop.operator), empty_operand(), reg);
    //     break;
    // case NOTOP:
    //     EMIT3OP(NOT, ASTNode_get_temp_reg(node->unaryop.operator), int_const(1), reg);
    //     break;
    // default:
    //     break;
    // }
    // ASTNode_set_temp_reg(node, reg);
}

void CodeGenVisitor_gen_while (NodeVisitor* visitor, ASTNode* node)
{
    /* generate code for the while loop */
    Operand loop_label = anonymous_label();
    Operand end_label = anonymous_label();
    EMIT1OP(LABEL, loop_label);
    ASTNode_copy_code(node, node->whileloop.condition);
    EMIT3OP(CBR, ASTNode_get_temp_reg(node->whileloop.condition), loop_label, end_label);
    ASTNode_copy_code(node, node->whileloop.body);
    EMIT1OP(LABEL, end_label);
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
    // v->postvisit_unaryop     = CodeGenVisitor_gen_unaryop;
    v->postvisit_whileloop   = CodeGenVisitor_gen_while;

    /* generate code into AST attributes */
    NodeVisitor_traverse_and_free(v, tree);

    /* copy generated code into new list (the AST may be deallocated before
     * the ILOC code is needed) */
    FOR_EACH(ILOCInsn*, i, (InsnList*)ASTNode_get_attribute(tree, "code")) {
        InsnList_add(iloc, ILOCInsn_copy(i));
    }
    return iloc;
}

