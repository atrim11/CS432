/**
 * @file p3-analysis.c
 * @brief Compiler phase 3: static analysis
 * @name Aidan Trimmer & Walker Todd
 */
#include "p3-analysis.h"

void AnalysisVisitor_check_mainExistProgram(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_check_whileloop(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_exit_whileloop(NodeVisitor* visitor, ASTNode* node);
//void type_mismatch_variable_check(NodeVisitor* visitor, Symbol* symbol, ASTNode* node);
void AnalysisVisitor_postvisit_check_return(NodeVisitor* visitor, ASTNode* node);

void AnalysisVisitor_postvisit_vardecl(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_funcdecl(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_funcdecl(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_literal(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_literal(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_assignment(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_location(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previst_break(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previst_continue(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_conditional(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_funcCall(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_funcCall(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_binaryop(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_binaryop(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previsit_block(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_previst_unaryop(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvist_unaryop(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_location(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_postvisit_block(NodeVisitor* visitor, ASTNode* node);

DecafType helper(ASTNode* node);
/**
 * @brief State/data for static analysis visitor
 */
typedef struct AnalysisData
{
    /**
     * @brief List of errors detected
     */
    ErrorList* errors;

    /* BOILERPLATE: TODO: add any new desired state information (and clean it up in AnalysisData_free) */
    int loop_depth;
    char* current_function;

    //List of current scope variables
    char* current_scope[100];
    int scope_index;


    char* functions_declared[100];
    int functions_index;
    bool inside_function;  // Add this flag to track if we're in a function


    //List of parameters for the current function
    char* current_parameters[100];

} AnalysisData;

/**
 * @brief Allocate memory for analysis data
 * 
 * @returns Pointer to allocated structure
 */
AnalysisData* AnalysisData_new (void)
{
    AnalysisData* data = (AnalysisData*)calloc(1, sizeof(AnalysisData));
    CHECK_MALLOC_PTR(data);
    data->errors = ErrorList_new();
    return data;
}

/**
 * @brief Deallocate memory for analysis data
 * 
 * @param data Pointer to the structure to be deallocated
 */
void AnalysisData_free (AnalysisData* data)
{
    /* free everything in data that is allocated on the heap except the error
     * list; it needs to be returned after the analysis is complete */


    /* free "data" itself */
    free(data);
}

/**
 * @brief Macro for more convenient access to the data inside a @ref AnalysisVisitor
 * data structure
 */
#define DATA ((AnalysisData*)visitor->data)

/**
 * @brief Macro for more convenient access to the error list inside a
 * @ref AnalysisVisitor data structure
 */
#define ERROR_LIST (((AnalysisData*)visitor->data)->errors)

/**
 * @brief Wrapper for @ref lookup_symbol that reports an error if the symbol isn't found
 * 
 * @param visitor Visitor with the error list for reporting
 * @param node AST node to begin the search at
 * @param name Name of symbol to find
 * @returns The @ref Symbol if found, otherwise @c NULL
 */
Symbol* lookup_symbol_with_reporting(NodeVisitor* visitor, ASTNode* node, const char* name)
{
    Symbol* symbol = lookup_symbol(node, name);
    if (symbol == NULL) {
        ErrorList_printf(ERROR_LIST, "Symbol '%s' undefined on line %d", name, node->source_line);
    }
    return symbol;
}

/**
 * @brief Macro for shorter storing of the inferred @c type attribute
 * in previst
 */
#define SET_INFERRED_TYPE(T) ASTNode_set_printable_attribute(node, "type", (void*)(T), \
                                 type_attr_print, dummy_free)

/**
 * @brief Macro for shorter retrieval of the inferred @c type attribute
 * in postvisit
 */
#define GET_INFERRED_TYPE(N) (DecafType)(long)ASTNode_get_attribute(N, "type")

ErrorList* analyze (ASTNode* tree)
{
    if (tree == NULL) {
        return;
    }
    /* allocate analysis structures */
    NodeVisitor* v = NodeVisitor_new();
    v->data = (void*)AnalysisData_new();
    v->dtor = (Destructor)AnalysisData_free;
    v->previsit_funcdecl = AnalysisVisitor_previsit_funcdecl;
    v->postvisit_funcdecl = AnalysisVisitor_postvisit_funcdecl;

    v->previsit_program = AnalysisVisitor_check_mainExistProgram;
    v->postvisit_return = AnalysisVisitor_postvisit_check_return;

    v->previsit_literal = AnalysisVisitor_previsit_literal;
    v->postvisit_literal = AnalysisVisitor_postvisit_literal;

    v->postvisit_vardecl = AnalysisVisitor_postvisit_vardecl;

    v->previsit_location = AnalysisVisitor_previsit_location;
    v->postvisit_location = AnalysisVisitor_postvisit_location;

    v->previsit_whileloop = AnalysisVisitor_check_whileloop;
    v->postvisit_assignment = AnalysisVisitor_postvisit_assignment;

    v->previsit_break = AnalysisVisitor_previst_break;
    v->previsit_continue = AnalysisVisitor_previst_continue;

    v->postvisit_conditional = AnalysisVisitor_postvisit_conditional;

    v->previsit_funccall = AnalysisVisitor_previsit_funcCall;
    v->postvisit_funccall = AnalysisVisitor_postvisit_funcCall;

    v->previsit_binaryop = AnalysisVisitor_previsit_binaryop;
    v->postvisit_binaryop = AnalysisVisitor_postvisit_binaryop;

    v->previsit_block = AnalysisVisitor_previsit_block;

    v->previsit_unaryop = AnalysisVisitor_previst_unaryop;
    v->postvisit_unaryop = AnalysisVisitor_postvist_unaryop;
    /* perform analysis, save error list, clean up, and return errors */
    NodeVisitor_traverse(v, tree);
    ErrorList* errors = ((AnalysisData*)v->data)->errors;
    NodeVisitor_free(v);
    return errors;
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_check_return(NodeVisitor* visitor, ASTNode* node) {
     if (node->funcreturn.value != NULL) {

        DecafType return_type = GET_INFERRED_TYPE(node->funcreturn.value);
        if (return_type == UNKNOWN) {
            return;
        }
        Symbol* symbol = lookup_symbol_with_reporting(visitor, node, DATA->current_function);

        if (symbol != NULL && symbol->type != return_type) {
            ErrorList_printf(ERROR_LIST, "Type mismatch in return statement on line %d: expected %s, got %s",
                             node->source_line, DecafType_to_string(symbol->type), DecafType_to_string(return_type));
        }
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_literal(NodeVisitor* visitor, ASTNode* node) {
    // Set the inferred type of the literal
    SET_INFERRED_TYPE(node->literal.type);
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_literal(NodeVisitor* visitor, ASTNode* node) {
    // Check if the literal is a void type
    if (GET_INFERRED_TYPE(node) == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Literal of type void on line %d", node->source_line);
    }
}

// keyworks reserved word list
char* reserved[] = {"int", "bool", "void", "if", "else", "while", "return", "true", "false", "break", "continue", "main", "for", "callout", "class", "interface", "extends", "implements", "new", "this", "string", "float", "double", "null"};

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_vardecl(NodeVisitor* visitor, ASTNode* node) {
    if (node->vardecl.type == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' declared as void on line %d", node->vardecl.name, node->source_line);
    }

    // Check if the variable is an array and if the array size is 0
    if (node->vardecl.is_array && node->vardecl.array_length == 0) {
        ErrorList_printf(ERROR_LIST, "Array '%s' on line %d must have positive non-zero length", node->vardecl.name, node->source_line);
    }
    // check for all reserved keywords
    for (int i = 0; i < (sizeof(reserved) / sizeof(reserved[0])); i++) {
        if (strcmp(node->vardecl.name, reserved[i]) == 0) {
            ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' declared as reserved keyword on line %d", node->vardecl.name, node->source_line);
        }
    }

    // Check if array is declared inside a function, which is not allowed
    if (DATA->inside_function && node->vardecl.is_array) {
        ErrorList_printf(ERROR_LIST, "Array '%s' declared inside function '%s' on line %d is not allowed",
                         node->vardecl.name, DATA->current_function, node->source_line);
    }

    // Check if the variable is already declared in the current scope
    for (int i = 0; i < DATA->scope_index; i++) {
        if (strcmp(node->vardecl.name, DATA->current_scope[i]) == 0) {
            ErrorList_printf(ERROR_LIST, "Variable '%s' redeclared on line %d", node->vardecl.name, node->source_line);
        }
    }

    // Add the variable to the current scope
    DATA->current_scope[DATA->scope_index] = node->vardecl.name;
    DATA->scope_index++;
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_funcdecl(NodeVisitor* visitor, ASTNode* node) {
    DATA->inside_function = true;
    // Set the current function name
    DATA->current_function = node->funcdecl.name;
    if (strcmp(node->funcdecl.name, "main") == 0) {
        if (node->funcdecl.return_type == VOID) {
            ErrorList_printf(ERROR_LIST, "Main function on line %d must return int", node->source_line);
        }
        if (node->funcdecl.parameters->size != 0) {
            ErrorList_printf(ERROR_LIST, "Main function on line %d must have no parameters", node->source_line);
        }
    }
    
    // add each parameter to a list of parameters
    Parameter* param = node->funcdecl.parameters->head;
    for (int i = 0; i < node->funcdecl.parameters->size; i++) {
        DATA->current_parameters[i] = param->name;
        param = param->next;
    }

}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_funcdecl(NodeVisitor* visitor, ASTNode* node) {
    // Reset the current function name
    DATA->current_function = "";
    DATA->inside_function = false;
    // checking if 2 functions have the same name
    // Duplicate symbols named 'foo' in scope started on line 1  for error checking
    for (int i = 0; i < DATA->functions_index; i++) {
        if (strcmp(node->funcdecl.name, DATA->functions_declared[i]) == 0) {
            ErrorList_printf(ERROR_LIST, "Duplicate symbols named '%s' in scope started on line 1", node->funcdecl.name);
        }
    }
    // Add the function to the list of declared functions
    DATA->functions_declared[DATA->functions_index] = node->funcdecl.name;
    DATA->functions_index++;

    // check if their is any duplicate parameters in the function
    for (int i = 0; i < node->funcdecl.parameters->size; i++) {
        for (int j = i + 1; j < node->funcdecl.parameters->size; j++) {
            if (strcmp(DATA->current_parameters[i], DATA->current_parameters[j]) == 0) {
                // Duplicate symbols named 'x' in scope started on line 1
                ErrorList_printf(ERROR_LIST, "Duplicate symbols named '%s' in scope started on line %d",
                                 DATA->current_parameters[i], node->source_line);
            }
        }
    }
    
    // clear the parameters list
    for (int i = 0; i < 100; i++) {
        DATA->current_parameters[i] = "";
    }

}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_assignment(NodeVisitor* visitor, ASTNode* node) {
    DecafType lhs_type = GET_INFERRED_TYPE(node->assignment.location);
    DecafType rhs_type = GET_INFERRED_TYPE(node->assignment.value);


    // Check if the types of the left and right hand sides of the assignment match
    if (lhs_type != rhs_type) {
        ErrorList_printf(ERROR_LIST, "Type mismatch in assignment on line %d: expected %s, got %s",
                         node->source_line, DecafType_to_string(lhs_type), DecafType_to_string(rhs_type));
    }

 
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_location(NodeVisitor* visitor, ASTNode* node) {
    Symbol* symbol = lookup_symbol(node, node->location.name);
    if (symbol != NULL) {
        SET_INFERRED_TYPE(symbol->type);

    } else {
        ErrorList_printf(ERROR_LIST, "Error: Variable '%s' used without being defined on line %d", 
                         node->location.name, node->source_line);
        SET_INFERRED_TYPE(UNKNOWN);
    }

}


/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_check_mainExistProgram(NodeVisitor* visitor, ASTNode* node)
{
    Symbol* symbol = lookup_symbol(node, "main");
    if (symbol == NULL) {
        ErrorList_printf(ERROR_LIST, "Program does not contain a 'main' function");
    }
}


/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_check_whileloop(NodeVisitor* visitor, ASTNode* node)
{
    // Increment the loop depth to indicate we're inside a loop
    DATA->loop_depth++;
    //printf("Entering loop: current loop depth = %d\n", DATA->loop_depth); // Debugging statement
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_exit_whileloop(NodeVisitor* visitor, ASTNode* node)
{
    // Decrement the loop depth when leaving the loop
    DATA->loop_depth--;
    //printf("Exiting loop: current loop depth = %d\n", DATA->loop_depth); // Debugging statement
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previst_break(NodeVisitor* visitor, ASTNode* node)
{
    // If loop_depth is 0, it means we're not inside any loops
    if (DATA->loop_depth == 0) {
        ErrorList_printf(ERROR_LIST, "Invalid 'break' outside loop on line %d", node->source_line);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previst_continue(NodeVisitor* visitor, ASTNode* node)
{
    // If loop_depth is 0, it means we're not inside any loops
    if (DATA->loop_depth == 0) {
        ErrorList_printf(ERROR_LIST, "Invalid 'continue' outside loop on line %d", node->source_line);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_conditional(NodeVisitor* visitor, ASTNode* node)
{
    DecafType condition = GET_INFERRED_TYPE(node->conditional.condition);
    // Check if the conditional expression is a boolean
    if (condition != BOOL) {
        ErrorList_printf(ERROR_LIST, "Invalid: Conditional expression on line %d must be of type bool", node->source_line);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_funcCall(NodeVisitor* visitor, ASTNode* node)
{
    Symbol* symbol = lookup_symbol_with_reporting(visitor, node, node->funccall.name);
    if (symbol != NULL) {
        // Set the inferred type of the function call to the return type of the function
        SET_INFERRED_TYPE(symbol->type);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_funcCall(NodeVisitor* visitor, ASTNode* node)
{
    Symbol* rec = lookup_symbol(node, node->funccall.name);
    // Check if the function call has the correct number of arguments
    if (rec->parameters->size != node->funccall.arguments->size) {
        ErrorList_printf(ERROR_LIST, "Invalid: Function call on line %d has incorrect number of arguments", node->source_line);
    }
    Parameter* param = rec->parameters->head;
    ASTNode* arg = node->funccall.arguments->head;
    // Invalid call to non-function 'foo' on line 3
    if (rec->symbol_type != FUNCTION_SYMBOL) {
        ErrorList_printf(ERROR_LIST, "Invalid call to non-function '%s' on line %d", node->funccall.name, node->source_line);
        return;
    }
    // Check if the function call has the correct types of arguments
    for (int i = 0; i < node->funccall.arguments->size; i++) {
 
        DecafType arg_type = helper(arg);
        if (param->type != arg_type)  {
            ErrorList_printf(ERROR_LIST, "Type mismatch in parameter %d of call to '%s': expected %s but found %s on line %d",
                             i, node->funccall.name, 
                              DecafType_to_string(param->type), DecafType_to_string(arg_type), node->source_line);
        }
        param = param->next;
        arg = arg->next;
    }
}



DecafType helper(ASTNode* node) {
    if (node->type == LITERAL) {
        return node->literal.type;
    } else if (node->type == LOCATION) {
        Symbol* symbol = lookup_symbol(node, node->location.name);
        if (symbol != NULL) {
            return symbol->type;
        }
    } else if (node->type == FUNCCALL) {
        Symbol* symbol = lookup_symbol(node, node->funccall.name);
        if (symbol != NULL) {
            return symbol->type;
        }
    } else if (node->type == BINARYOP) {
        return GET_INFERRED_TYPE(node);
    } else if (node->type == UNARYOP) {
        return GET_INFERRED_TYPE(node);
    }
    return UNKNOWN;
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_binaryop(NodeVisitor* visitor, ASTNode* node)
{
    BinaryOpType binop = node->binaryop.operator;
    if (binop == OROP || binop == ANDOP || binop == EQOP || binop == NEQOP || binop == LTOP || binop == LEOP || binop == GEOP || binop == GTOP) {
        SET_INFERRED_TYPE(BOOL);
    } else {
        SET_INFERRED_TYPE(INT);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_binaryop(NodeVisitor* visitor, ASTNode* node)
{
    DecafType lhs_type = GET_INFERRED_TYPE(node->binaryop.left);
    DecafType rhs_type = GET_INFERRED_TYPE(node->binaryop.right);
    DecafType op_type = GET_INFERRED_TYPE(node);
    BinaryOpType binop = node->binaryop.operator;

    // check if either types are unknown
    if (lhs_type == UNKNOWN || rhs_type == UNKNOWN) {
        return;
    }
    
    if (binop == OROP || binop == ANDOP) 
    {
        if (lhs_type != BOOL ) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(BOOL), DecafType_to_string(lhs_type), node->source_line);
        }
        if (rhs_type != BOOL) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(BOOL), DecafType_to_string(lhs_type), node->source_line);
        }
    } else if (binop == EQOP || binop == NEQOP) 
    {
        if (lhs_type != rhs_type) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(lhs_type), DecafType_to_string(rhs_type), node->source_line);
        }
    } else if (binop == LTOP || binop == LEOP || binop == GEOP || binop == GTOP) 
    {
        if (lhs_type != INT) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(INT), DecafType_to_string(lhs_type), node->source_line);
        }
        if (rhs_type != INT) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(INT), DecafType_to_string(rhs_type), node->source_line);
        }
    } else if (binop == ADDOP || binop == SUBOP || binop == MULOP || binop == DIVOP || binop == MODOP) 
    {
        if (lhs_type != INT) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(INT), DecafType_to_string(lhs_type), node->source_line);
        }
        if (rhs_type != INT) {
            ErrorList_printf(ERROR_LIST, "Type mismatch: %s expected but %s found on line %d",
                             DecafType_to_string(INT), DecafType_to_string(rhs_type), node->source_line);
        }
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previsit_block(NodeVisitor* visitor, ASTNode* node)
{
    // Increment the scope index to indicate we're entering a new scope
    DATA->scope_index = 0;
    // Clear the whole array when we go to a new block
    for (int i = 0; i < 100; i++) {
        DATA->current_scope[i] = "";
    }
}


/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_previst_unaryop(NodeVisitor* visitor, ASTNode* node)
{
    UnaryOpType unop = node->unaryop.operator;
    if (unop == NOTOP) {
        SET_INFERRED_TYPE(BOOL);
    } else {
        SET_INFERRED_TYPE(INT);
    }
}


/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvist_unaryop(NodeVisitor* visitor, ASTNode* node)
{
    DecafType child_type = GET_INFERRED_TYPE(node->unaryop.child);
    DecafType op_type = GET_INFERRED_TYPE(node);
    // Check if the types of the left and right hand sides of the binary operation match
    if (child_type != op_type) {
        ErrorList_printf(ERROR_LIST, "Type mismatch in unary operation on line %d: expected %s, got %s",
                         node->source_line, DecafType_to_string(child_type), DecafType_to_string(op_type));
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_postvisit_location(NodeVisitor* visitor, ASTNode* node)
{
    if (GET_INFERRED_TYPE(node) == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Location of type void on line %d", node->source_line);
    }

    // if they are accessing an array, check if they are using an index
    if (node->location.index == NULL) {
        Symbol* symbol = lookup_symbol(node, node->location.name);
        if (symbol != NULL && symbol->symbol_type == ARRAY_SYMBOL) {
            ErrorList_printf(ERROR_LIST, "Invalid: Array '%s' on line %d accessed without an index", node->location.name, node->source_line);
        }    
    }

    // if someone is trying to access an array with an index, check if the index is an integer
    if (node->location.index != NULL) {
        DecafType index_type = GET_INFERRED_TYPE(node->location.index);
        if (index_type != INT) {
            ErrorList_printf(ERROR_LIST, "Invalid: Array index on line %d must be of type int", node->source_line);
        }
    }

    // Accessing a non array like an array
    if (node->location.index != NULL) {
        Symbol* symbol = lookup_symbol(node, node->location.name);
        if (symbol != NULL && symbol->symbol_type == SCALAR_SYMBOL) {
            ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' on line %d accessed as an array", node->location.name, node->source_line);
        }
    }
}