/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 * @Author: Aidan Trimmer & Walker Todd
 */

#include "p2-parser.h"

/*
 * helper functions
 */

/**
 * @brief Look up the source line of the next token in the queue.
 * 
 * @param input Token queue to examine
 * @returns Source line
 */
int get_next_token_line (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    return TokenQueue_peek(input)->line;
}

/**
 * @brief Check next token for a particular type and text and discard it
 * 
 * Throws an error if there are no more tokens or if the next token in the
 * queue does not match the given type or text.
 * 
 * @param input Token queue to modify
 * @param type Expected type of next token
 * @param text Expected text of next token
 */
void match_and_discard_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected \'%s\')\n", text);
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != type || !token_str_eq(token->text, text)) {
        Error_throw_printf("Expected \'%s\' but found '%s' on line %d\n",
                text, token->text, get_next_token_line(input));
    }
    Token_free(token);
}

/**
 * @brief Remove next token from the queue
 * 
 * Throws an error if there are no more tokens.
 * 
 * @param input Token queue to modify
 */
void discard_next_token (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input\n");
    }
    Token_free(TokenQueue_remove(input));
}

/**
 * @brief Look ahead at the type of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @returns True if the next token is of the expected type, false if not
 */
bool check_next_token_type (TokenQueue* input, TokenType type)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type);
}

/**
 * @brief Look ahead at the type and text of the next token
 * 
 * @param input Token queue to examine
 * @param type Expected type of next token
 * @param text Expected text of next token
 * @returns True if the next token is of the expected type and text, false if not
 */
bool check_next_token (TokenQueue* input, TokenType type, const char* text)
{
    if (TokenQueue_is_empty(input)) {
        return false;
    }
    Token* token = TokenQueue_peek(input);
    return (token->type == type) && (token_str_eq(token->text, text));
}

/**
 * @brief Parse and return a Decaf type
 * 
 * @param input Token queue to modify
 * @returns Parsed type (it is also removed from the queue)
 */
DecafType parse_type (TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected type)\n");
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != KEY) {
        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    DecafType t = VOID;
    if (token_str_eq("int", token->text)) {
        t = INT;
    } else if (token_str_eq("bool", token->text)) {
        t = BOOL;
    } else if (token_str_eq("void", token->text)) {
        t = VOID;
    } else {
        Error_throw_printf("Invalid type '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    Token_free(token);
    return t;
}

/**
 * @brief Parse and return a Decaf identifier
 * 
 * @param input Token queue to modify
 * @param buffer String buffer for parsed identifier (should be at least
 * @c MAX_TOKEN_LEN characters long)
 */
void parse_id (TokenQueue* input, char* buffer)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected identifier)\n");
    }
    Token* token = TokenQueue_remove(input);
    if (token->type != ID) {
        Error_throw_printf("Invalid ID '%s' on line %d\n", token->text, get_next_token_line(input));
    }
    snprintf(buffer, MAX_ID_LEN, "%s", token->text);
    Token_free(token);
}


/*
* Parse Var Decl  
*/
ASTNode* parse_vardecl(TokenQueue* input)
{
    DecafType temp = parse_type(input);
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);
    match_and_discard_next_token(input, SYM, ";");
    return VarDeclNode_new(buffer, temp, false, 0,0);

}

ASTNode* parse_loc (TokenQueue* input) {
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);

    if (check_next_token(input, SYM, "[")) {
        match_and_discard_next_token(input, SYM, "[");
        ASTNode* index = parse(input);
        match_and_discard_next_token(input, SYM, "]");
        return LocationNode_new(buffer, index, get_next_token_line(input));
    } else {
        return LocationNode_new(buffer, NULL, get_next_token_line(input));
    }
}

// ASTNode* parse_lit (TokenQueue* input) {
//     if (TokenQueue_is_empty(input)) {
//         Error_throw_printf("Unexpected end of input (expected literal)\n");
//     }
//     Token* token = TokenQueue_remove(input);
//     ASTNode* node = NULL;
//     switch (token->type) {
     
//         // case LITERAL:
//         //     node = LiteralNode_new_bool(token -> text, token->line);
//         //     break;
//         case STRLIT:
//             node = LiteralNode_new_string(token->text, token->line);
//             break;

//         case DECLIT:
//             node = LiteralNode_new_int(atoi(token->text), token->line);
//             break;
//         case HEXLIT:
//             node = LiteralNode_new_int(strtol(token->text, NULL, 16), token->line);
//             break;
//         default:
//             Error_throw_printf("Invalid literal '%s' on line %d\n", token->text, token->line);
//     }
//     Token_free(token);
//     return node;
// }

// ASTNode* parse_unaryExpr (TokenQueue* input) {
//     if (check_next_token(input, SYM, "-") || check_next_token(input, SYM, "!")) {
//         Token* token = TokenQueue_remove(input);
//         ASTNode* child = parse_unaryExpr(input);
//         return UnaryOpNode_new(token->text, child, token->line);
//     } else {
//         return parse_loc(input);
//     }
// }

// ASTNode* parse_binaryexpression (TokenQueue* input) {
//     ASTNode* left = parse_unaryExpr(input);
//     if (check_next_token(input, SYM, "+") || check_next_token(input, SYM, "-") || check_next_token(input, SYM, "*") || check_next_token(input, SYM, "/") || check_next_token(input, SYM, "%") || check_next_token(input, SYM, "<") || check_next_token(input, SYM, ">") || check_next_token(input, SYM, "<=") || check_next_token(input, SYM, ">=") || check_next_token(input, SYM, "==") || check_next_token(input, SYM, "!=") || check_next_token(input, SYM, "&&") || check_next_token(input, SYM, "||")) {
//         Token* token = TokenQueue_remove(input);
//         ASTNode* right = parse_expr(input);
//         return BinaryOpNode_new(token->text, left, right, token->line);
//     } else {
//         return left;
//     }

// }

// ASTNode* parse_expr (TokenQueue* input) {
//     return parse_binaryexpression(input);
// }


/**
 * @brief Parse and return a block of statements
 * 
 * @param input Token queue to modify
 * @returns Parsed block of statements
 * 
 */
ASTNode* parse_stmts (TokenQueue* input) {

   if (check_next_token(input, KEY, "break")) {
        match_and_discard_next_token(input, KEY, "break");
        match_and_discard_next_token(input, SYM, ";");
        return BreakNode_new(get_next_token_line(input));

   } else if (check_next_token(input, KEY, "return ")) {
        match_and_discard_next_token(input, KEY, "return");
        ASTNode* expr = parse(input);
        match_and_discard_next_token(input, SYM, ";");
        return ReturnNode_new(expr, get_next_token_line(input));
   } else if (check_next_token(input, KEY, "continue")) {
        match_and_discard_next_token(input, KEY, "continue");
        match_and_discard_next_token(input, SYM, ";");
        return ContinueNode_new(get_next_token_line(input));
   } else if (check_next_token(input, KEY, "while")) {
        match_and_discard_next_token(input, KEY, "while");
        match_and_discard_next_token(input, SYM, "(");
        ASTNode* cond = parse_expr(input);
        match_and_discard_next_token(input, SYM, ")");
        ASTNode* body = parse_block(input);
        return WhileNode_new(cond, body, get_next_token_line(input));
   }

   

}

/**
 * @brief Parse and return a block of statements
 * 
 * @param input Token queue to modify
 * @returns Parsed block of statements
 */
ASTNode* parse_block (TokenQueue* input) {
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected '{')\n");
    }
    
    NodeList* vars = NodeList_new();
    NodeList* stmts = NodeList_new();
    int line = get_next_token_line(input);
    match_and_discard_next_token(input, SYM, "{");
    // check if its an int void or bool
    while (!check_next_token(input, SYM, "}")) {
        if (check_next_token(input, KEY, "int") || check_next_token(input, KEY, "bool") || check_next_token(input, KEY, "void")) {
            NodeList_add(vars, parse_vardecl(input));
        }
        
    }

    match_and_discard_next_token(input, SYM, "}");
    return BlockNode_new(vars, stmts, line);
}

ASTNode* parse_funcdecl (TokenQueue* input) {
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected 'def')\n");
    }
    int line = get_next_token_line(input);
    match_and_discard_next_token(input, KEY, "def");
    DecafType return_type = parse_type(input);
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);
    // ParameterList* params = ParameterList_new();
    // ignore paramters for c level

    match_and_discard_next_token(input, SYM, "(");
    match_and_discard_next_token(input, SYM, ")");

    return FuncDeclNode_new(buffer, return_type, NULL, parse_block(input), line);
}



/*
 * node-level parsing functions
 */

ASTNode* parse_program (TokenQueue* input)
{
    NodeList* vars = NodeList_new();
    NodeList* funcs = NodeList_new();
    while (!TokenQueue_is_empty(input)) {
        if (check_next_token(input, KEY, "def")) {
            NodeList_add(funcs, parse_funcdecl(input));
        } else {
            NodeList_add(vars, parse_vardecl(input));
        }
    }
       
    // NodeList_add(vars, VarDeclNode_new("test", INT, false, 0,0));
    return ProgramNode_new(vars, funcs);
}

ASTNode* parse (TokenQueue* input)
{
    return parse_program(input);
}

