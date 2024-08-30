/**
 * @file p1-lexer.c
 * @brief Compiler phase 1: lexer
 * Authors: Aidan Trimmer & Walker Todd
 */
#include "p1-lexer.h"

TokenQueue* lex (const char* text)
{
    TokenQueue* tokens = TokenQueue_new();
 
    /* compile regular expressions */
    Regex* whitespace = Regex_new("^[ \n]");
    Regex* letter = Regex_new("^([a-zA-Z][a-zA-Z_0-9]*)");
    Regex* symbol = Regex_new("^([*]|[+]|[(]|[)])");
    Regex* num = Regex_new("^(0|[1-9][0-9]*)");
    int line = 1;
    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    while (*text != '\0') {
 
        /* match regular expressions */
        if (Regex_match(whitespace, text, match)) {
            line = line + 1;
        } else if (Regex_match(letter, text, match)) {
            TokenQueue_add(tokens, Token_new(ID, match, line));
        } else if (Regex_match(symbol, text, match))
        {
            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(num, text, match))
        {
            TokenQueue_add(tokens, Token_new(DECLIT, match, line));
        } else {
            Error_throw_printf("Invalid token!\n");
        }
 
        /* skip matched text to look for next token */
        text += strlen(match);
    }
 
    /* clean up */
    Regex_free(whitespace);
    Regex_free(letter);
 
    return tokens;
}