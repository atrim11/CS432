/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 * @Author: Aidan Trimmer & Walker Todd
 */

#include "p2-parser.h"
ASTNode* parse_vardecl(TokenQueue* input);
ASTNode* parse_expr(TokenQueue* input);
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
* Parse Lit
*/
ASTNode* parse_lit(TokenQueue* input)
{
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected literal)\n");
    }

    Token* token = TokenQueue_remove(input);
    int line = get_next_token_line(input);

    ASTNode* node = NULL;

    if (token->type == DECLIT) {
        int temp = atoi(token->text);
        match_and_discard_next_token(input, SYM, ";");
        node = LiteralNode_new_int(temp, line);

    } else if (token->type == HEXLIT) {
        int temp = strtol(token->text, NULL, 16);
        match_and_discard_next_token(input, SYM, ";");
        node = LiteralNode_new_int(temp, line);

    } else if (strcmp(token->text, "true") == 0) {
        match_and_discard_next_token(input, SYM, ";");
        node = LiteralNode_new_bool(true, line);

    } else if (strcmp(token->text, "false") == 0) {
        match_and_discard_next_token(input, SYM, ";");
        node = LiteralNode_new_bool(false, line);

    } else if (token->type == STRLIT) {
        match_and_discard_next_token(input, SYM, ";");
        node = LiteralNode_new_string(token->text, line);

    } else {
        Token_free(token);
        Error_throw_printf("Invalid literal '%s' on line %d\n", token->text, line);
    }

    Token_free(token);
    return node;
}


/* 
* Parse Args
*/
ASTNode* parse_args(TokenQueue* input)
{
    // Expr (',' Expr)*
    return parse_expr(input);
}


/* 
* Parse Func Call
*/
ASTNode* parse_funcCall(TokenQueue* input)
{
   
}

/* 
* Parse Loc
*/
ASTNode* parse_loc(TokenQueue* input)
{
   
}


/* 
* Parse Base Expr
*/
ASTNode* parse_baseExpr(TokenQueue* input)
{

}


/* 
* Parse Unary Expr
*/
ASTNode* parse_unaryExpr(TokenQueue* input)
{
    
}


/* 
* Parse Bin Expr
*/
ASTNode* parse_binExpr(TokenQueue* input)
{
    
}


/* 
* Parse Expr
*/
ASTNode* parse_expr(TokenQueue* input)
{
    
}


/* 
* Parse Stmt
*/
ASTNode* parse_stmts(TokenQueue* input)
{
   
}


/* 
* Parse Block
*/
ASTNode* parse_block(TokenQueue* input)
{
   
}


/* 
* Parse Params
*/
ASTNode* parse_params(TokenQueue* input)
{
   
}


/* 
* Parse Func Decl
*/
ASTNode* parse_funcdecl(TokenQueue* input)
{
   
}

/*
* Parse Var Decl  
*/
ASTNode* parse_vardecl(TokenQueue* input)
{
    DecafType temp = parse_type(input);
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);
    int line = get_next_token_line(input);
    match_and_discard_next_token(input, SYM, ";");
    return VarDeclNode_new(buffer, temp, false, 0, line);
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


