/**
 * @file p1-lexer.c
 * @brief Compiler phase 1: lexer
 * Authors: Aidan Trimmer and Walker Todd
 */
#include "p1-lexer.h"

TokenQueue* lex (const char* text)
{
    if (text == NULL) {
        Error_throw_printf("No text");
    }
    TokenQueue* tokens = TokenQueue_new();
 
    /* compile regular expressions */
    Regex* whitespace = Regex_new("^[ ]");
    Regex* new_line = Regex_new("^\n");
    Regex* letter = Regex_new("^([a-zA-Z][a-zA-Z_0-9]*)");
    // We need to add [] to the symbol regex for some reason its not catching them
    Regex* symbol = Regex_new("^([-%*+()[{}}])");
    Regex* extra_symbol = Regex_new("^([]])");
    Regex* double_symbol = Regex_new("^([!><=]=)");
    Regex* hex = Regex_new("^(0x[0-9a-fA-F]+)");
    Regex* num = Regex_new("^(0|[1-9][0-9]*)");
    Regex* str_lit = Regex_new("^\"([^\"]*)\"");
    int line = 1;
    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    while (*text != '\0') {
        /* match regular expressions */
        if (Regex_match(whitespace, text, match)) {
            // skip whitespace
        } else if (Regex_match(new_line, text, match)) {
            // New line check
            line++;
        } else if (Regex_match(hex, text, match)) {
            TokenQueue_add(tokens, Token_new(HEXLIT, match, line));
            // had to add extra in order to find ]
        } else if (Regex_match(extra_symbol, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(double_symbol, text, match)) {

            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(str_lit, text, match)) {
            TokenQueue_add(tokens, Token_new(STRLIT, match, line));
        } else if (Regex_match(letter, text, match)) {
            TokenQueue_add(tokens, Token_new(ID, match, line));
        } else if (Regex_match(symbol, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(num, text, match))
        {
            TokenQueue_add(tokens, Token_new(DECLIT, match, line));
        } else {
            // I had to get rid of the old one because it was causing stuff to break for osme reason
            // Error_throw_printf("Invalid token!");
            // This is causing a memory leak for some reason
            Error_throw_printf("Invalid token!");
        }
 
        /* skip matched text to look for next token */
        text += strlen(match);
    }
 
    /* clean up */
    Regex_free(whitespace);
    Regex_free(letter);
    Regex_free(symbol);
    Regex_free(num);
    Regex_free(hex);
    Regex_free(str_lit);
    Regex_free(double_symbol);
    Regex_free(extra_symbol);
    Regex_free(new_line);

 
    return tokens;
}