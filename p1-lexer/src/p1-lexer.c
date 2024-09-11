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
    Regex* symbol = Regex_new("^([-%*+()[{}}=])");
    Regex* extra_symbol = Regex_new("^([]\\;])");
    Regex* double_symbol = Regex_new("^([!><=]=)");
    Regex* hex = Regex_new("^(0x[0-9a-fA-F]+)");
    Regex* num = Regex_new("^(0|[1-9][0-9]*)");
    Regex* str_lit = Regex_new("^\"([^\"]*)\"");
    // Im not sure why but when the ^ is removed from the keyword regex it starts printing again
    Regex* keyword = Regex_new("(int|def|return)");
    Regex* comment = Regex_new("^//.*");
    Regex* illegal = Regex_new("(for)");
    int line = 1;
    /* read and handle input */
    char match[MAX_TOKEN_LEN];
    char temp[MAX_TOKEN_LEN];
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
            // finding a keyword literal
        }else if (Regex_match(letter, text, match)) {
            if (Regex_match(keyword, match, temp) && strcmp(match, temp) == 0) {
                TokenQueue_add(tokens, Token_new(KEY, temp, line));
            } else if (Regex_match(illegal, match, temp)  && strcmp(match, temp) == 0){
                Error_throw_printf("Invalid token!");
            } else {
                TokenQueue_add(tokens, Token_new(ID, match, line));
            }
            // Should we check for keywords as well?
            // I think we may need to instead of having the keyword and illegal before this should be in this if else

        } else if (Regex_match(symbol, text, match)) {
            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(num, text, match)){
            TokenQueue_add(tokens, Token_new(DECLIT, match, line));
        } else if (Regex_match(comment, text, match)) {
            // skip comments
        } else {
            //printf(text);
            // This is causing a memory leak for some reason
            Error_throw_printf("Invalid token!");
            //printf("Invalid token!\n");
            // print hte invalid token
            // printf("%s\n", tokens->head->text);
            // printf("%s\n", text);
            
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
    Regex_free(keyword);
    Regex_free(illegal);
    Regex_free(comment);

 
    return tokens;
}