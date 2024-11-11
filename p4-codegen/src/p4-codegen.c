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

    int while_count;
    int if_count;

    Operand loop_label[10];
    Operand body_label[10];
    Operand end_label[10];

    Operand if_body_label[10];
    Operand if_end_label[10];




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
    /* Function prologue */
    EMIT1OP(LABEL, call_label(node->funcdecl.name));
    EMIT1OP(PUSH, base_register());
    EMIT2OP(I2I, stack_register(), base_register());
    EMIT3OP(ADD_I, stack_register(), int_const(0), base_register());

    /* Generate function body */
    ASTNode_copy_code(node, node->funcdecl.body);

    /* Epilogue */
    EMIT1OP(LABEL, DATA->current_epilogue_jump_label);
    EMIT2OP(I2I, base_register(), stack_register());
    EMIT1OP(POP, base_register());
    EMIT0OP(RETURN);
}


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
        
        /* Try to get the result register for the return value */
        Operand result_reg = ASTNode_get_temp_reg(node->funcreturn.value);

        if (result_reg.id == -1) {
            // If result_reg is invalid, load the value from [BP-8] to RET
            result_reg = virtual_register();
            EMIT3OP(LOAD_AI, base_register(), int_const(-8), result_reg);
            
        }
        /* Move the result into RET register */
        EMIT2OP(I2I, result_reg, return_register());
    }

    /* Jump to the function's epilogue to handle cleanup */
    EMIT1OP(JUMP, DATA->current_epilogue_jump_label);
}


void CodeGenVisitor_gen_literal(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the literal value */
    Operand reg = virtual_register();  // Use a new temporary register
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

void GenCodeVisitor_gen_pre_while(NodeVisitor* visitor, ASTNode* node)
{
    DATA->while_count++;
    int current_index = DATA->while_count;

    DATA->loop_label[current_index] = anonymous_label();
    DATA->body_label[current_index] = anonymous_label();
    DATA->end_label[current_index] = anonymous_label();

    // ASTNode_set_attribute(node, "loop_label", DATA->loop_label[current_index], NULL);
    // ASTNode_set_attribute(node, "body_label", DATA->body_label[current_index], NULL);
    // ASTNode_set_attribute(node, "end_label", DATA->end_label[current_index], NULL);
}


void CodeGenVisitor_gen_post_while(NodeVisitor* visitor, ASTNode* node)
{
    int current_index = DATA->while_count;

    EMIT1OP(LABEL, DATA->loop_label[current_index]);
    ASTNode_copy_code(node, node->whileloop.condition);
    EMIT3OP(CBR, ASTNode_get_temp_reg(node->whileloop.condition), 
            DATA->body_label[current_index], 
            DATA->end_label[current_index]);

    EMIT1OP(LABEL, DATA->body_label[current_index]);
    ASTNode_copy_code(node, node->whileloop.body);
    EMIT1OP(JUMP, DATA->loop_label[current_index]);

    EMIT1OP(LABEL, DATA->end_label[current_index]);

    DATA->while_count--;
}



void GenCodeVisitor_gen_post_break(NodeVisitor* visitor, ASTNode* node)
{
    if (DATA->while_count < 0) {
        fprintf(stderr, "Error: 'break' statement used outside of a loop\n");
        return;
    }

    EMIT1OP(JUMP, DATA->end_label[DATA->while_count]);
}



void GenCodeVisitor_gen_post_continue(NodeVisitor* visitor, ASTNode* node)
{
    if (DATA->while_count < 0) {
        fprintf(stderr, "Error: 'continue' statement used outside of a loop\n");
        return;
    }

    EMIT1OP(JUMP, DATA->loop_label[DATA->while_count]);
}


void CodeGenVisitor_gen_assign(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the RHS expression (value) */
    ASTNode_copy_code(node, node->assignment.value);

    /* Retrieve the result register of the RHS expression */
    Operand rhs_reg = ASTNode_get_temp_reg(node->assignment.value);
    /* Get the base address and offset for the LHS variable (location) */
    Symbol *var = lookup_symbol(node, node->assignment.location->location.name);
    Operand lhs_base = var_base(node, var);
    Operand lhs_offset = var_offset(node, var);

    /* Store the RHS result into the LHS variable's location */
    EMIT3OP(STORE_AI, rhs_reg, lhs_base, lhs_offset);  // storeAI r4 => [BP-8]

    /* Set the temporary register for the assignment node */
    // ASTNode_set_temp_reg(node, rhs_reg);
    
}


void GenCodeVisitor_gen_pre_conditional(NodeVisitor* visitor, ASTNode* node)
{
    DATA->if_count++;
    int current_index = DATA->if_count;

    DATA->if_body_label[current_index] = anonymous_label();
    DATA->if_end_label[current_index] = anonymous_label();

    // ASTNode_set_attribute(node, "if_body_label", DATA->if_body_label[current_index], NULL);
    // ASTNode_set_attribute(node, "if_end_label", DATA->if_end_label[current_index], NULL);
}


void GenCodeVisitor_gen_post_conditional(NodeVisitor* visitor, ASTNode* node)
{
    int current_index = DATA->if_count;

    ASTNode_copy_code(node, node->conditional.condition);
    EMIT3OP(CBR, ASTNode_get_temp_reg(node->conditional.condition), 
            DATA->if_body_label[current_index], 
            DATA->if_end_label[current_index]);

    EMIT1OP(LABEL, DATA->if_body_label[current_index]);
    ASTNode_copy_code(node, node->conditional.if_block);

    if (node->conditional.else_block != NULL) {
        Operand else_end_label = anonymous_label();
        EMIT1OP(JUMP, else_end_label);
        EMIT1OP(LABEL, DATA->if_end_label[current_index]);
        ASTNode_copy_code(node, node->conditional.else_block);
        EMIT1OP(LABEL, else_end_label);
    } else {
        EMIT1OP(LABEL, DATA->if_end_label[current_index]);
    }

    DATA->if_count--;
}



void CodeGenVisitor_gen_post_funccall(NodeVisitor* visitor, ASTNode* node)
{
    /* Generate code for the function call */
    FOR_EACH(ASTNode*, arg, node->funccall.arguments) {
        ASTNode_copy_code(node, arg);
        EMIT1OP(PUSH, ASTNode_get_temp_reg(arg));
    }
    EMIT1OP(CALL, call_label(node->funccall.name));
    EMIT3OP(ADD_I, stack_register(), int_const(node->funccall.arguments->size * 8), stack_register());

    /* Directly use the return value (RET) */
    Operand ret_reg = return_register();
    Operand temp_reg = virtual_register();
    EMIT2OP(I2I, ret_reg, temp_reg);
    ASTNode_set_temp_reg(node, temp_reg);
}

void CodeGenVisitor_gen_post_location (NodeVisitor* visitor, ASTNode* node)
{
    ASTNode *parent = (ASTNode *)ASTNode_get_attribute(node, "parent");
    if (parent != NULL && parent->type == ASSIGNMENT) {
        if (parent->assignment.location == node) {
            return;
        }
    }
    /* Get the base address and offset for the variable */
    Symbol *var = lookup_symbol(node, node->location.name);
    Operand base = var_base(node, var);
    Operand offset = var_offset(node, var);

    /* Load the value from the variable's location */
    Operand reg = virtual_register();

    //Array check
    if(var->symbol_type == ARRAY_SYMBOL) {
        ASTNode_copy_code(node, node->location.index);

        Operand index_reg = ASTNode_get_temp_reg(node->location.index);
        Operand temp_reg = virtual_register();
        EMIT3OP(MULT_I, index_reg, int_const(8), temp_reg);

        EMIT3OP(LOAD_AO, base, temp_reg, reg);
    } else {
        EMIT3OP(LOAD_AI, base, offset, reg);
    }
    ASTNode_set_temp_reg(node, reg);
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
    // v->postvisit_vardecl     = CodeGenVisitor_gen_vardecl;

    v->postvisit_binaryop    = CodeGenvisitor_gen_post_binaryop;
    v->postvisit_unaryop     = CodeGenVisitor_gen_unaryop;

    v->previsit_whileloop    = GenCodeVisitor_gen_pre_while;
    v->postvisit_whileloop   = CodeGenVisitor_gen_post_while;
    v->postvisit_break       = GenCodeVisitor_gen_post_break;
    v->postvisit_continue    = GenCodeVisitor_gen_post_continue;

    // v->previsit_assignment   = CodeGenVisitor_gen_pre_location;
    v->postvisit_assignment  = CodeGenVisitor_gen_assign;

    v->postvisit_location    = CodeGenVisitor_gen_post_location;

    v->previsit_conditional  = GenCodeVisitor_gen_pre_conditional;
    v->postvisit_conditional = GenCodeVisitor_gen_post_conditional;

    // v->previsit_funccall     = CodeGenVisitor_gen_pre_funccall;
    v->postvisit_funccall    = CodeGenVisitor_gen_post_funccall;

    /* generate code into AST attributes */
    NodeVisitor_traverse_and_free(v, tree);

    /* copy generated code into new list (the AST may be deallocated before
     * the ILOC code is needed) */
    FOR_EACH(ILOCInsn*, i, (InsnList*)ASTNode_get_attribute(tree, "code")) {
        InsnList_add(iloc, ILOCInsn_copy(i));
    }
    return iloc;
}

