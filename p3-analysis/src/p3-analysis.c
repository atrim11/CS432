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

    v->previsit_whileloop = AnalysisVisitor_check_whileloop;
    v->postvisit_assignment = AnalysisVisitor_postvisit_assignment;

    v->previsit_break = AnalysisVisitor_previst_break;
    v->previsit_continue = AnalysisVisitor_previst_continue;

    v->postvisit_conditional = AnalysisVisitor_postvisit_conditional;

    v->previsit_funccall = AnalysisVisitor_previsit_funcCall;
    v->postvisit_funccall = AnalysisVisitor_postvisit_funcCall;
    /* perform analysis, save error list, clean up, and return errors */
    NodeVisitor_traverse(v, tree);
    ErrorList* errors = ((AnalysisData*)v->data)->errors;
    NodeVisitor_free(v);
    return errors;
}

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

void AnalysisVisitor_previsit_literal(NodeVisitor* visitor, ASTNode* node) {
    // Set the inferred type of the literal
    SET_INFERRED_TYPE(node->literal.type);
}

void AnalysisVisitor_postvisit_literal(NodeVisitor* visitor, ASTNode* node) {
    // Check if the literal is a void type
    if (GET_INFERRED_TYPE(node) == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Literal of type void on line %d", node->source_line);
    }
}

// keyworks reserved word list
char* reserved[] = {"int", "bool", "void", "if", "else", "while", "return", "true", "false", "break", "continue", "main"};

void AnalysisVisitor_postvisit_vardecl(NodeVisitor* visitor, ASTNode* node) {

    if (node->vardecl.type == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' declared as void on line %d", node->vardecl.name, node->source_line);
    }

    // Check if the variable is an array and if the array size is 0
    if (node->vardecl.is_array && node->vardecl.array_length == 0) {
        ErrorList_printf(ERROR_LIST, "Array '%s' on line %d must have positive non-zero length", node->vardecl.name, node->source_line);
    }
    // check for all reserved keywords
    for (int i = 0; i < 12; i++) {
        if (strcmp(node->vardecl.name, reserved[i]) == 0) {
            ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' declared as reserved keyword on line %d", node->vardecl.name, node->source_line);
        }
    }


    
}

void AnalysisVisitor_previsit_funcdecl(NodeVisitor* visitor, ASTNode* node) {
    // Set the current function name
    DATA->current_function = node->funcdecl.name;
}

void AnalysisVisitor_postvisit_funcdecl(NodeVisitor* visitor, ASTNode* node) {
    // Reset the current function name
    DATA->current_function = "";
}


void AnalysisVisitor_postvisit_assignment(NodeVisitor* visitor, ASTNode* node) {
    DecafType lhs_type = GET_INFERRED_TYPE(node->assignment.location);
    DecafType rhs_type = GET_INFERRED_TYPE(node->assignment.value);

    // Check if the types of the left and right hand sides of the assignment match
    if (lhs_type != rhs_type) {
        ErrorList_printf(ERROR_LIST, "Type mismatch in assignment on line %d: expected %s, got %s",
                         node->source_line, DecafType_to_string(lhs_type), DecafType_to_string(rhs_type));
    }
}

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
        ErrorList_printf(ERROR_LIST, "Main function not found on line %d", node->source_line);
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

void AnalysisVisitor_previst_break(NodeVisitor* visitor, ASTNode* node)
{
    // If loop_depth is 0, it means we're not inside any loops
    if (DATA->loop_depth == 0) {
        ErrorList_printf(ERROR_LIST, "Invalid 'break' outside loop on line %d", node->source_line);
    }
}

void AnalysisVisitor_previst_continue(NodeVisitor* visitor, ASTNode* node)
{
    // If loop_depth is 0, it means we're not inside any loops
    if (DATA->loop_depth == 0) {
        ErrorList_printf(ERROR_LIST, "Invalid 'continue' outside loop on line %d", node->source_line);
    }
}

void AnalysisVisitor_postvisit_conditional(NodeVisitor* visitor, ASTNode* node)
{
    DecafType condition = GET_INFERRED_TYPE(node->conditional.condition);
    // Check if the conditional expression is a boolean
    if (condition != BOOL) {
        ErrorList_printf(ERROR_LIST, "Invalid: Conditional expression on line %d must be of type bool", node->source_line);
    }
}

void AnalysisVisitor_previsit_funcCall(NodeVisitor* visitor, ASTNode* node)
{
    Symbol* symbol = lookup_symbol_with_reporting(visitor, node, node->funccall.name);
    if (symbol != NULL) {
        // Set the inferred type of the function call to the return type of the function
        SET_INFERRED_TYPE(symbol->type);
    }
}

void AnalysisVisitor_postvisit_funcCall(NodeVisitor* visitor, ASTNode* node)
{
    // Check if the function call is a void type
    if (GET_INFERRED_TYPE(node) == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Function call on line %d returns void", node->source_line);
    }

    // Check if the function call has the correct number of arguments

}