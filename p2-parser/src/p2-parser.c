/**
 * @file p2-parser.c
 * @brief Compiler phase 2: parser
 * @Author: Aidan Trimmer & Walker Todd
 */

#include "p2-parser.h"
bool check_next_token (TokenQueue* input, TokenType type, const char* text);
ASTNode* parse_lit(TokenQueue* input);
ASTNode* parse_expr(TokenQueue* input);
ASTNode* parse(TokenQueue* input);
/*
 * helper functions
 */

BinaryOpType helper_get_binary_op_type (TokenQueue* Input) 
{
    Token* token = TokenQueue_peek(Input);
    if (strcmp(token->text, "||") == 0) {
        return OROP;
    } else if (strcmp(token->text, "&&") == 0) {
        return ANDOP;
    } else if (strcmp(token->text, "==") == 0) {
        return EQOP;
    } else if (strcmp(token->text, "!=") == 0) {
        return NEQOP;
    } else if (strcmp(token->text, "<") == 0) {
        return LTOP;
    } else if (strcmp(token->text, "<=") == 0) {
        return LEOP;
    } else if (strcmp(token->text, ">=") == 0) {
        return GEOP;
    } else if (strcmp(token->text, ">") == 0) {
        return GTOP;
    } else if (strcmp(token->text, "+") == 0) {
        return ADDOP;
    } else if (strcmp(token->text, "-") == 0) {
        return SUBOP;
    } else if (strcmp(token->text, "*") == 0) {
        return MULOP;
    } else if (strcmp(token->text, "/") == 0) {
        return DIVOP;
    } else if (strcmp(token->text, "%") == 0) {
        return MODOP;
    } else {
        Error_throw_printf("Invalid binary operator '%s' on line %d\n", TokenQueue_peek(Input)->text, get_next_token_line(Input));
    }
}
Token* peek_2_ahead(TokenQueue* input);

bool isBinOP(TokenQueue* Input) {
    Token* token = peek_2_ahead(Input);
    if (strcmp(token->text, "||") == 0) {
        return true;
    } else if (strcmp(token->text, "&&") == 0) {
        return true;
    } else if (strcmp(token->text, "==") == 0) {
        return true;
    } else if (strcmp(token->text, "!=") == 0) {
        return true;
    } else if (strcmp(token->text, "<") == 0) {
        return true;
    } else if (strcmp(token->text, "<=") == 0) {
        return true;
    } else if (strcmp(token->text, ">=") == 0) {
        return true;
    } else if (strcmp(token->text, ">") == 0) {
        return true;
    } else if (strcmp(token->text, "+") == 0) {
        return true;
    } else if (strcmp(token->text, "-") == 0) {
        return true;
    } else if (strcmp(token->text, "*") == 0) {
        return true;
    } else if (strcmp(token->text, "/") == 0) {
        return true;
    } else if (strcmp(token->text, "%") == 0) {
        return true;
    } else {
        false;
    }
}

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
    // printf("%s\n", token->text);
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

Token* peek_2_ahead(TokenQueue* input);
ASTNode* parse_stmts (TokenQueue* input);

/*
* Parse Var Decl  
*/
ASTNode* parse_vardecl(TokenQueue* input)
{
    // this is wrong
    // for example like this
    //     int a[10];
    // a[0] = 5;
    // the second line it thinks a should be int when it can be a 

    //checking next token type if its a key or not

    DecafType temp = parse_type(input);

    char buffer[MAX_ID_LEN];

    parse_id(input, buffer);
    int line = get_next_token_line(input);

    bool is_array = false;
    int array_length = 0;
    if (check_next_token(input, SYM, "[")) {
        match_and_discard_next_token(input, SYM, "[");
        Token* token = TokenQueue_remove(input);
        array_length = atoi(token->text);
        is_array = true;
        match_and_discard_next_token(input, SYM, "]");
    }
    match_and_discard_next_token(input, SYM, ";");
    return VarDeclNode_new(buffer, temp, is_array, array_length, line);
}

// if (check_next_token(input, SYM, "[")) {
//         match_and_discard_next_token(input, SYM, "[");
//         if (check_next_token(input, SYM, "]")) {
//             match_and_discard_next_token(input, SYM, "]");
//             match_and_discard_next_token(input, SYM, ";");
//             return VarDeclNode_new(buffer, temp, true, 0, line);
//         } else {
//             ASTNode* size = parse_lit(input);
//             match_and_discard_next_token(input, SYM, "]");
//             match_and_discard_next_token(input, SYM, ";");
//             return VarDeclNode_new(buffer, temp, true, size, line);
//         }
//     } else {
//         match_and_discard_next_token(input, SYM, ";");
//         return VarDeclNode_new(buffer, temp, false, 0, line);
//     }

ASTNode* parse_loc (TokenQueue* input) {
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);

    if (check_next_token(input, SYM, "[")) {
        match_and_discard_next_token(input, SYM, "[");
        ASTNode* index = parse_expr(input);
        // ArrayAssignment checker
        if (check_next_token(input, SYM, "[")) {
            match_and_discard_next_token(input, SYM, "[");
            ASTNode* index2 = parse_expr(input);
            match_and_discard_next_token(input, SYM, "]");
            match_and_discard_next_token(input, SYM, "]");
            return LocationNode_new(buffer, BinaryOpNode_new(ADDOP, index, index2, get_next_token_line(input)), get_next_token_line(input));
        }
        match_and_discard_next_token(input, SYM, "]");
        return LocationNode_new(buffer, index, get_next_token_line(input));
    } 
    
    return LocationNode_new(buffer, NULL, get_next_token_line(input));
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
        printf("Token Type: %d\n", token->type);
        int temp = atoi(token->text);
        node = LiteralNode_new_int(temp, line);

    } else if (token->type == HEXLIT) {
        int temp = strtol(token->text, NULL, 16);
        node = LiteralNode_new_int(temp, line);

    } else if (strcmp(token->text, "true") == 0) {
        node = LiteralNode_new_bool(true, line);

    } else if (strcmp(token->text, "false") == 0) {
        node = LiteralNode_new_bool(false, line);

    } else if (token->type == STRLIT) {
        char* temp = token->text;
        if (temp[0] == '"') {
            temp++;
        } 
        if (temp[strlen(temp) - 1] == '"') {
            temp[strlen(temp) - 1] = '\0';
        }
        
        for (int i = 0; i < strlen(temp); i++) {
            if (temp[i] == '\n' || temp[i] == '\r') {
                // Unescaped newlines or carriage returns are not allowed
                Error_throw_printf("Error: Unescaped newline or carriage return in string literal on line %d\n", line);
            }
            
            if (temp[i] == '\\') {
                if (temp[i + 1] == 'n') {
                    temp[i] = '\n';
                } else if (temp[i + 1] == 't') {
                    temp[i] = '\t';
                } else if (temp[i + 1] == 'r') {
                    temp[i] = '\r';
                } else if (temp[i + 1] == '0') {
                    temp[i] = '\0';
                } else if (temp[i + 1] == '\\') {
                    temp[i] = '\\';
                } else if (temp[i + 1] == '"') {
                    temp[i] = '"';
                } else {
                    // Invalid escape sequence
                    Error_throw_printf("Error: Invalid escape sequence \\%c\n on line %d\n", temp[i + 1], line);
                }
                
                // Shift the rest of the string to the left to remove the extra character
                for (int j = i + 1; j < strlen(temp); j++) {
                    temp[j] = temp[j + 1];
                }
            }
        }

        node = LiteralNode_new_string(temp, line);
    } else {
        Token_free(token);
        // print out the token type
        // printf("Token Type: %d\n", token->type);
        Error_throw_printf("Invalid literal '%s' on line %d\n", token->text, line);

    }

    Token_free(token);
    return node; 
}

ASTNode* parse_funcCall (TokenQueue* input);

ASTNode* parse_baseExpr (TokenQueue* input) {
    Token* token = peek_2_ahead(input);
    char buffer[MAX_ID_LEN];
    if (check_next_token(input, SYM, "(")) {
        match_and_discard_next_token(input, SYM, "(");
        ASTNode* expr = parse_expr(input);
        match_and_discard_next_token(input, SYM, ")");
        return expr;
    } else if (check_next_token_type(input, ID)) {
        // printf("Token Type: %d\n", input->head->type);
        // printf("Token Text: %s\n", input->head->text);
        // printf("next token: %s\n", token->text);
        if (strcmp(token->text, "(") == 0) {
            printf("Parsing FuncCall in baseexpression\n");
            ASTNode* func = parse_funcCall(input);
            return func;
        }
        printf("Parsing Location in base expression\n");
        ASTNode* loc = parse_loc(input);
        return loc;
    } else {
        printf("Parsing Literal in Base Expression\n");
        return parse_lit(input);
    }
}

ASTNode* parse_unaryExpr (TokenQueue* input) {
    Token* token = TokenQueue_peek(input);
    if (strcmp(token->text, "-") == 0) {
        match_and_discard_next_token(input, SYM, "-");
        return UnaryOpNode_new(NEGOP, parse_baseExpr(input), get_next_token_line(input));
    } else if (strcmp(token->text, "!") == 0) {
        match_and_discard_next_token(input, SYM, "!");
        return UnaryOpNode_new(NOTOP, parse_baseExpr(input), get_next_token_line(input));
    } else {
        printf("Parsing Base Expression\n");
        return parse_baseExpr(input);
    }
    
}

ASTNode* parse_binaryexpression (TokenQueue* input) {
    
    if (!isBinOP(input)) {
        printf("Unary Expression:\n");
        return parse_unaryExpr(input);
    } else {
        while (!check_next_token(input, SYM, ";")) {
            printf("Binary Expression:\n");
            ASTNode* leftExpr = parse_unaryExpr(input);
            BinaryOpType operatorToken = helper_get_binary_op_type(input);
            TokenQueue_remove(input);
                
            // printf("Binary Expression: %s\n", TokenQueue_peek(input)->text);
            ASTNode* rightExpr = parse_binaryexpression(input);
            return BinaryOpNode_new(operatorToken, leftExpr, rightExpr, get_next_token_line(input)); 
        }
    }
}


ASTNode* parse_expr (TokenQueue* input) {
    return parse_binaryexpression(input);
}



ASTNode* parse_funcCall (TokenQueue* input) {
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);
    match_and_discard_next_token(input, SYM, "(");
    NodeList* args = NodeList_new();
    while (!check_next_token(input, SYM, ")")) {
        NodeList_add(args, parse_expr(input));
        if (check_next_token(input, SYM, ",")) {
            match_and_discard_next_token(input, SYM, ",");
        }
    }
    match_and_discard_next_token(input, SYM, ")");
    return FuncCallNode_new(buffer, args, get_next_token_line(input));
}

ASTNode* parse_block (TokenQueue* input);

//  method to peek 2 ahead to see if the next one is either a [ or (
//  if it is then we know its a loc or funccall
//  if its not then we know its an assignment
// queue->head->next
Token* peek_2_ahead(TokenQueue* input) {
    Token* token = TokenQueue_peek(input);
    return token->next;
}


/**
 * @brief Parse and return a block of statements
 * 
 * @param input Token queue to modify
 * @returns Parsed block of statements
 * 
 */
ASTNode* parse_stmts (TokenQueue* input) {
    // Assignment
    char buffer[MAX_ID_LEN];            
    Token* token = peek_2_ahead(input);
    
    printf("parsing statments: %s\n", TokenQueue_peek(input)->text);
    if ((token->type == SYM && strcmp(token->text, "=") == 0)  || strcmp(token->text, "[") == 0) {
        printf("Parsing Location\n");
        ASTNode* loc = parse_loc(input);
        match_and_discard_next_token(input, SYM, "=");
        ASTNode* expr = parse_expr(input);
        match_and_discard_next_token(input, SYM, ";");
        return AssignmentNode_new(loc, expr, get_next_token_line(input));
        // also check if hte last token wasnt if
    } else if (token->type == SYM && strcmp(token->text, "(") == 0  && !check_next_token(input, KEY, "if") && !check_next_token(input, KEY, "while")) {
        printf("Parsing FuncCall\n");
        ASTNode* func = parse_funcCall(input);
        match_and_discard_next_token(input, SYM, ";");
        return func;
    } else if (check_next_token(input, KEY, "break")) {
        match_and_discard_next_token(input, KEY, "break");
        match_and_discard_next_token(input, SYM, ";");
        return BreakNode_new(get_next_token_line(input));
    } else if (check_next_token(input, KEY, "continue")) {
        match_and_discard_next_token(input, KEY, "continue");
        match_and_discard_next_token(input, SYM, ";");
        return ContinueNode_new(get_next_token_line(input));
    } else if (check_next_token(input, KEY, "return")) {

        match_and_discard_next_token(input, KEY, "return");
        // Checking if the next token is a semicolon or if its not we need ot parse expression
        if (check_next_token(input, SYM, ";")) {
            match_and_discard_next_token(input, SYM, ";");
            return ReturnNode_new(NULL, get_next_token_line(input));
        }
        ASTNode* expr = parse_expr(input);
        match_and_discard_next_token(input, SYM, ";");
        return ReturnNode_new(expr, get_next_token_line(input));
    } else if (check_next_token(input, KEY, "while")) {
        match_and_discard_next_token(input, KEY, "while");
        match_and_discard_next_token(input, SYM, "(");
        ASTNode* condition = parse_expr(input);
        match_and_discard_next_token(input, SYM, ")");
        ASTNode* body = parse_block(input);
        return WhileLoopNode_new(condition, body, get_next_token_line(input));
        // checking for funccall
    
    } else if (check_next_token(input, KEY, "if")) {
        match_and_discard_next_token(input, KEY, "if");
        match_and_discard_next_token(input, SYM, "(");
        ASTNode* condition = parse_expr(input);
        match_and_discard_next_token(input, SYM, ")");
        ASTNode* if_block = parse_block(input);
        ASTNode* else_block = NULL;
        if (check_next_token(input, KEY, "else")) {
            match_and_discard_next_token(input, KEY, "else");
            else_block = parse_block(input);
        }
        return ConditionalNode_new(condition, if_block, else_block, get_next_token_line(input));
    }

    return NULL;
}


/**
 * @brief Parse and return a block of statements
 * 
 * @param input Token queue to modify
 * @returns Parsed block of statements
 */
ASTNode* parse_block (TokenQueue* input) {
    match_and_discard_next_token(input, SYM, "{");

    NodeList* vars = NodeList_new();
    NodeList* stmts = NodeList_new();
    int line = get_next_token_line(input);
    // check if its an int void or bool
    while (check_next_token(input, KEY, "int") || check_next_token(input, KEY, "bool") || check_next_token(input, KEY, "void")) {
            NodeList_add(vars, parse_vardecl(input));
        } 

    // This is causing an infinite loop
    while (!check_next_token(input, SYM, "}")) {
        NodeList_add(stmts, parse_stmts(input));
    }


    match_and_discard_next_token(input, SYM, "}");
    return BlockNode_new(vars, stmts, line);
}

// ASTNode* parse_param (TokenQueue* input) {
//     DecafType type = parse_type(input);
//     char buffer[MAX_ID_LEN];
//     parse_id(input, buffer);
//     return ParamNode_new(buffer, type, get_next_token_line(input));
// }


ASTNode* parse_funcdecl (TokenQueue* input) {
    if (TokenQueue_is_empty(input)) {
        Error_throw_printf("Unexpected end of input (expected 'def')\n");
    }
    int line = get_next_token_line(input);
    match_and_discard_next_token(input, KEY, "def");
    DecafType return_type = parse_type(input);
    char buffer[MAX_ID_LEN];
    parse_id(input, buffer);

    match_and_discard_next_token(input, SYM, "(");

    // parse_params
    ParameterList* params = ParameterList_new();
    if (!check_next_token(input, SYM, ")")) {
        DecafType param_type = parse_type(input);
        char param_buffer[MAX_ID_LEN];
        parse_id(input, param_buffer);
        ParameterList_add_new(params, param_buffer, param_type);
        while (check_next_token(input, SYM, ",")) {
            discard_next_token(input);
            param_type = parse_type(input);
            parse_id(input, param_buffer);
            ParameterList_add_new(params, param_buffer, param_type);
        }
    }

    match_and_discard_next_token(input, SYM, ")");

    return FuncDeclNode_new(buffer, return_type, params, parse_block(input), line);
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
    // Private:A_null_token_queue:0: (after this point) Received signal 11 (Segmentation fault)
    if (input == NULL) {
        Error_throw_printf("Null Input\n");
    }

    return parse_program(input);
}

