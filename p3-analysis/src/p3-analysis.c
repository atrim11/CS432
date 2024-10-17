/**
 * @file p3-analysis.c
 * @brief Compiler phase 3: static analysis
 * @name Aidan Trimmer & Walker Todd
 */
#include "p3-analysis.h"

void AnalysisVisitor_check_vardecl(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_check_location(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_check_funcDecl(NodeVisitor* visitor, ASTNode* node);
void AnalysisVisitor_check_mainExistProgram(NodeVisitor* visitor, ASTNode* node);

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
 */
#define SET_INFERRED_TYPE(T) ASTNode_set_printable_attribute(node, "type", (void*)(T), \
                                 type_attr_print, dummy_free)

/**
 * @brief Macro for shorter retrieval of the inferred @c type attribute
 */
#define GET_INFERRED_TYPE(N) (DecafType)(long)ASTNode_get_attribute(N, "type")

ErrorList* analyze (ASTNode* tree)
{
    /* allocate analysis structures */
    NodeVisitor* v = NodeVisitor_new();
    v->data = (void*)AnalysisData_new();
    v->dtor = (Destructor)AnalysisData_free;
    v->previsit_vardecl = AnalysisVisitor_check_vardecl;
    v->previsit_location = AnalysisVisitor_check_location;
    v->previsit_funcdecl = AnalysisVisitor_check_funcDecl;
    v->previsit_program = AnalysisVisitor_check_mainExistProgram;

    /* BOILERPLATE: TODO: register analysis callbacks */
    

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
void AnalysisVisitor_check_vardecl(NodeVisitor* visitor, ASTNode* node)
{
    if (node->vardecl.type == VOID) {
        ErrorList_printf(ERROR_LIST, "Invalid: Variable '%s' declared as void on line %d", node->vardecl.name, node->source_line);
    }

    // Check if the variable is an array and if the array size is 0
    if (node->vardecl.is_array && node->vardecl.array_length == 0) {
        ErrorList_printf(ERROR_LIST, "Array '%s' on line %d must have positive non-zero length", node->vardecl.name, node->source_line);
    }
}

/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_check_location(NodeVisitor* visitor, ASTNode* node)
{
    // Look up the symbol for the location
    Symbol* symbol = lookup_symbol_with_reporting(visitor, node, node->location.name);

    // If the symbol exists, check if it's an array or not
    if (symbol != NULL) {
        // If the symbol is being indexed (array access)
        if (node->location.index != NULL) {
            // Check if the symbol is an array
            if (symbol->symbol_type != ARRAY_SYMBOL) {
                ErrorList_printf(ERROR_LIST, "Non-array '%s' accessed as an array on line %d", node->location.name, node->source_line);
            }

            // Now check the type of the index (must be INT)
            ASTNode* index_node = node->location.index;

            // Check if the index is a literal and of type INT
            if (index_node->type == LITERAL) {
                if (index_node->literal.type != INT) {
                    ErrorList_printf(ERROR_LIST, "Type mismatch: int expected but found non-integer type on line %d", node->source_line);
                }
            } else {
                // If it's not a literal, we must check if the type is inferred correctly
                // You may have a helper function to ensure type inference (e.g., from a previous phase)
                DecafType index_type = GET_INFERRED_TYPE(index_node);
                if (index_type != INT) {
                    ErrorList_printf(ERROR_LIST, "Array index for '%s' must be of type INT on line %d", node->location.name, node->source_line);
                }
            }
        } else if (symbol->symbol_type == ARRAY_SYMBOL) {
            // If the symbol is an array but is not being indexed
            ErrorList_printf(ERROR_LIST, "Array '%s' accessed without index on line %d", node->location.name, node->source_line);
        }
    }
}





/**
 * @brief 
 * 
 * @param visitor 
 * @param node 
 */
void AnalysisVisitor_check_funcDecl(NodeVisitor* visitor, ASTNode* node)
{
    Symbol* symbol = lookup_symbol(node, "main");
    if (symbol != NULL) {
        if (symbol->parameters->size != 0) {
            ErrorList_printf(ERROR_LIST, "Main function must have no parameters on line %d", node->source_line);
        }
    } else {
        ErrorList_printf(ERROR_LIST, "Main function not found on line %d", node->source_line);
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
    // you can make a variable named main 
    Symbol* symbol = lookup_symbol(node, "main");
    if (symbol == NULL) {
        ErrorList_printf(ERROR_LIST, "Main function not found on line %d", node->source_line);
    }
}


// 
// every possible thing needs to be tested 