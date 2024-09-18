/**
 * @file p1-lexer.c
 * @brief Compiler phase 1: lexer
 * Authors: Aidan Trimmer and Walker Todd
 * AI was used to try and and think of more edge cases while testing code.
 */
#include "p1-lexer.h"

TokenQueue* lex (const char* text)
{
    if (text == NULL) {
        Error_throw_printf("No text");
    }
    TokenQueue* tokens = TokenQueue_new();

    /* compile regular expressions */
    Regex* whitespace = Regex_new("^[ \t]+");
    Regex* new_line = Regex_new("^\n");
    Regex* letter = Regex_new("^([a-zA-Z][a-zA-Z_0-9]*)");
    Regex* symbol = Regex_new("^([-%*+()[{}}=]|[]\\;]|[!><=]=)");

    Regex* hex = Regex_new("^(0x[0-9a-fA-F]+)");
    Regex* num = Regex_new("^(0|[1-9][0-9]*)");
    Regex* str_lit = Regex_new("^\"([^\"]*)\"");
    Regex* keyword = Regex_new("^(def|if|else|while|return|break|continue|int|bool|void|true|false)");
    Regex* comment = Regex_new("^//.*");
    // Adding reserved words as illegal
    Regex* reserved =
        Regex_new("^(for|callout|class|interface|extends|implements|new|this|string|float|double|null)");
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
            // finding a hex literal
            TokenQueue_add(tokens, Token_new(HEXLIT, match, line));
        } else if (Regex_match(str_lit, text, match)) {
            // finding a string literal
            TokenQueue_add(tokens, Token_new(STRLIT, match, line));
        } else if (Regex_match(letter, text, match)) {
            // finding a keyword literal and comparing it to the reserved words
            if (Regex_match(keyword, match, temp) && strcmp(match, temp) == 0) {
                TokenQueue_add(tokens, Token_new(KEY, temp, line));
                // Error if reserved word is found
            } else if (Regex_match(reserved, match, temp)  && strcmp(match, temp) == 0) {
                Error_throw_printf("Resvered Word found at line %d", line, line);
            } else {
                TokenQueue_add(tokens, Token_new(ID, match, line));
            }
        } else if (Regex_match(symbol, text, match)) {
            // finding a symbol
            TokenQueue_add(tokens, Token_new(SYM, match, line));
        } else if (Regex_match(num, text, match)) {
            // finding a decimal literal
            TokenQueue_add(tokens, Token_new(DECLIT, match, line));
        } else if (Regex_match(comment, text, match)) {
            // skip comments
        } else {
            Error_throw_printf("Invalid token at line %d", line, line);
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
    Regex_free(new_line);
    Regex_free(keyword);
    Regex_free(reserved);
    Regex_free(comment);

    return tokens;
}