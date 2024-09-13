/**
 * @file public.c
 * @brief Public test cases (and location for new tests)
 *
 * This file provides a few basic sanity test cases and a location to add new
 * tests.
 */

#include "testsuite.h"

#ifndef SKIP_IN_DOXYGEN

TEST_1TOKEN (D_symbol,           "+",       SYM,    "+")
TEST_1TOKEN (D_single_digit,     "1",       DECLIT, "1")
TEST_1TOKEN (D_single_letter_id, "a",       ID,     "a")
TEST_1TOKEN (D_multi_letter_id,  "abc",     ID,     "abc")
TEST_INVALID(D_invalid_symbol,   "^")
TEST_INVALID(D_invalid_id,       "_true")
TEST_2TOKENS(D_multi_tokens,     "foo*", ID, "foo", SYM, "*")
// Added Tests



TEST_1TOKEN (C_string,           "\"hi\"",  STRLIT, "\"hi\"")
TEST_1TOKEN (C_symbol,           "-",       SYM,    "-")
TEST_INVALID(C_invalid_symbol,   "@")
TEST_1TOKEN (C_hex,              "0xabcd",  HEXLIT, "0xabcd")
TEST_2TOKENS(C_multi_tokens,     "123 foo", DECLIT, "123", ID, "foo")

TEST_1TOKEN (B_keyword,          "def",     KEY,    "def")
TEST_INVALID(B_invalid_for,      "for")
TEST_1TOKEN (B_symbol_equal,     "==",      SYM,    "==")

TEST_2TOKENS(B_multi_tokens1,    "def  ab", KEY,    "def", ID, "ab")

Token multi_tokens2[] = { { .type = KEY, .text = "def", .line = 1 },
                          { .type = ID,  .text = "foo", .line = 1 },
                          { .type = SYM, .text = ";",   .line = 1 } };
TEST_TOKENS (B_multi_tokens2, "def foo;", 3, multi_tokens2)

TEST_0TOKENS(A_comments,         "// test")
TEST_1TOKEN (A_keyword_id,       "int3",    ID,     "int3")
TEST_2TOKENS(A_multi_dec_dec,    "0123",    DECLIT, "0", DECLIT, "123")

// Added Tests
TEST_1TOKEN(Lab_symbol,   "\(",    SYM,    "\(")
TEST_1TOKEN(Lab_symbol2,   "\)",    SYM,    "\)")
TEST_INVALID(Lab_symbol_inv,   "@")
TEST_INVALID(Lab_symbol_inv2,   "$")
TEST_1TOKEN(Lab_dec,   "42", DECLIT, "42")
TEST_1TOKEN(Lab_dec2,   "1337", DECLIT, "1337")
TEST_1TOKEN(Lab_id,   "strangely_long_var_name", ID, "strangely_long_var_name")
TEST_1TOKEN(Lab_id2,   "bar05", ID, "bar05")
TEST_INVALID(Lab_id_inv,   "bar.baz")
TEST_INVALID(Lab_id2_inv,   "_foo")
TEST_INVALID (Lab_reserved_word, "class")
TEST_INVALID(Lab_reserved_word_interface, "interface")
TEST_1TOKEN (Lab_valid_mixed_case, "Foo", ID, "Foo")
TEST_1TOKEN(Lab_valid_hex, "0x1A3f", HEXLIT, "0x1A3f")
TEST_1TOKEN(Lab_valid_less_equal, "<=", SYM, "<=")
TEST_1TOKEN(Lab_valid_not_equal, "!=", SYM, "!=")
TEST_0TOKENS(Lab_comment_single, "// This is a comment")
TEST_1TOKEN(Lab_valid_bool_true, "true", KEY, "true")
TEST_1TOKEN(Lab_valid_bool_false, "false", KEY, "false")
TEST_2TOKENS(Lab_negative_number, "-123", SYM, "-", DECLIT, "123")

#endif

/**
 * @brief Register all test cases
 * @param s Test suite to which the tests should be added
 */
void public_tests (Suite *s)
{
    TCase *tc = tcase_create ("Public");
    TEST(D_symbol);
    TEST(D_single_digit);
    TEST(D_single_letter_id);
    TEST(D_multi_letter_id);
    TEST(D_invalid_symbol);
    TEST(D_invalid_id);
    TEST(D_multi_tokens);
    TEST(C_string);
    TEST(C_symbol);
    TEST(C_invalid_symbol);
    TEST(C_hex);
    TEST(C_multi_tokens);
    TEST(B_keyword);
    TEST(B_invalid_for);
    TEST(B_symbol_equal);
    TEST(B_multi_tokens1);
    TEST(B_multi_tokens2);
    TEST(A_comments);
    TEST(A_keyword_id);
    TEST(A_multi_dec_dec);
    // Added Tests
    TEST(Lab_symbol);
    TEST(Lab_reserved_word_interface);
    TEST(Lab_symbol2);
    TEST(Lab_valid_mixed_case);
    TEST(Lab_valid_hex);
    TEST(Lab_valid_less_equal);
    TEST(Lab_valid_not_equal);
    TEST(Lab_comment_single);
    TEST(Lab_valid_bool_true);
    TEST(Lab_valid_bool_false);
    TEST(Lab_negative_number);
    TEST(Lab_symbol_inv);
    TEST(Lab_symbol_inv2);
    TEST(Lab_dec);
    TEST(Lab_dec2);
    TEST(Lab_id);
    TEST(Lab_id2);
    TEST(Lab_id_inv);
    TEST(Lab_id2_inv);
    TEST(Lab_reserved_word);
    TEST(Lab_reserved_word_interface);
    TEST(Lab_valid_mixed_case);
    suite_add_tcase (s, tc);
}

