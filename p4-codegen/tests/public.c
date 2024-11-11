#include "testsuite.h"

#ifndef SKIP_IN_DOXYGEN

// D-level Tests

TEST_PROGRAM(D_sanity_zero, 0, "def int main() { return 0; }")
TEST_EXPRESSION(D_expr_int, 7, "7")
TEST_EXPRESSION(D_expr_add, 5, "2+3")
TEST_EXPRESSION(D_expr_long, 10, "1+2+3+4")
TEST_EXPRESSION(D_expr_sanity_zero, 0, "0")

// C-level Tests

// Test binary integer operations (addition, subtraction, multiplication, division)
TEST_EXPRESSION(C_expr_add, 7, "3 + 4")
TEST_EXPRESSION(C_expr_subtract, 1, "4 - 3")
TEST_EXPRESSION(C_expr_multiply, 12, "3 * 4")
TEST_EXPRESSION(C_expr_divide, 2, "8 / 4")

// Test unary integer operation (negation)
TEST_EXPRESSION(C_expr_negation, -5, "-5")
TEST_EXPRESSION(C_expr_combined, -3, "-(5 - 2)")

// Test assignment without arrays
TEST_MAIN(C_assignment_simple, 42, "int x; x = 42; return x;")
TEST_PROGRAM(C_global_assignment_mult, 14, "int a; def int main() { a = 2 + 3 * 4; return a; }")
TEST_PROGRAM(C_global_assignment, 14, "int a; def int main() { a = 2 + 3 * 4; return a; }")

// Test variable locations
TEST_MAIN(C_location_access, 15, "int y; y = 15; return y;")

// Additional C-level Tests

TEST_EXPRESSION(C_expr_complex, 11, "3 + 2 * 4")
TEST_EXPRESSION(C_expr_parentheses, 20, "(3 + 2) * 4")
TEST_MAIN(C_expr_precedence, 14, "int x; x = 2 + 3 * 4; return x;")
TEST_MAIN(C_multiple_vars, 25, "int a; int b; a = 10; b = 15; return a + b;")
TEST_MAIN(C_var_reuse, 30, "int a; a = 10; a = a + 20; return a;")

// B-level Tests

// Test boolean literals and expressions
TEST_BOOL_EXPRESSION(B_expr_true, 1, "true")
TEST_BOOL_EXPRESSION(B_expr_false, 0, "false")
TEST_BOOL_EXPRESSION(B_expr_not, 1, "!false")
TEST_BOOL_EXPRESSION(B_expr_not_t, 0, "!true")

// Test conditional (if-else) with boolean literals
TEST_MAIN(B_if_true, 5, "int result; if (true) { result = 5; } else { result = 10; } return result;")
TEST_MAIN(B_if_false, 10, "int result; if (false) { result = 5; } else { result = 10; } return result;")
TEST_MAIN(B_conditional, 2, "int r; if (true) { r = 2; } else { r = 3; } return r;")

// Test while loops
TEST_MAIN(B_whileloop_simple, 3, "int count; count = 0; while (count < 3) { count = count + 1; } return count;")
TEST_MAIN(B_whileloop, 10, "int a; a = 0; while (a < 10) { a = a + 1; } return a;")

// Test continue and break in while loops
TEST_MAIN(B_whileloop_break, 5, 
        "int count; count = 0; "
        "while (count < 10) { "
        "  if (count == 5) { break; } "
        "  count = count + 1; "
        "} "
        "return count;")

TEST_MAIN(B_whileloop_continue, 4, 
        "int count;  int result; count = 0; result = 0; "
        "while (count < 5) { "
        "  count = count + 1; "
        "  if (count == 3) { continue; } "
        "  result = result + 1; "
        "} "
        "return result;")

// Additional B-level Tests

// Nested conditionals
TEST_MAIN(B_nested_conditionals, 1, 
    "int result; "
    "if (true) { "
    "  if (false) { result = 0; } "
    "  else { result = 1; } "
    "} "
    "else { result = 2; } "
    "return result;")

// Complex while loop with mixed conditions and increments
// TEST_MAIN(B_whileloop_mixed, 6,
//     "int count; int result; "
//     "count = 0; result = 0; "
//     "while (count < 5) { "
//     "  if (count % 2 == 0) { result = result + 2; } "
//     "  else { result = result + 1; } "
//     "  count = count + 1; "
//     "} "
//     "return result;")

// While loop with early exit (break)
TEST_MAIN(B_whileloop_break_early, 3,
    "int count; "
    "count = 0; "
    "while (true) { "
    "  count = count + 1; "
    "  if (count == 3) { break; } "
    "} "
    "return count;")

// Continue to skip iteration in a loop
TEST_MAIN(B_whileloop_continue_even_sum, 6,
    "int count; int sum; "
    "count = 0; sum = 0; "
    "while (count < 5) { "
    "  count = count + 1; "
    "  if (count % 2 != 0) { continue; } "
    "  sum = sum + count; "
    "} "
    "return sum;")

// Function call tests
TEST_PROGRAM(B_function_noparams, 42,
        "def int meaningOfLife() { return 42; } "
        "def int main() { return meaningOfLife(); }")
TEST_PROGRAM(B_funccall, 20, 
        "def int ten() { return 10; } "
        "def int main() { return ten() + ten(); }")
TEST_PROGRAM(B_function_conditional, 15,
        "def int fifteen() { return 15; } "
        "def int main() { if (true) { return fifteen(); } else { return 0; } }")
TEST_PROGRAM(B_function_in_expression, 25,
        "def int five() { return 5; } "
        "def int main() { return five() * 5; }")
TEST_PROGRAM(B_multiple_function_calls, 30,
        "def int ten() { return 10; } "
        "def int main() { return ten() + ten() + ten(); }")

// A-level Test (Function with parameters)
TEST_PROGRAM(A_funccall_params, 5,
        "def int add(int a, int b) { return a + b; } "
        "def int main() { return add(2,3); }")

TEST_MAIN(Made_B_conditional_if_else, 1,
    "int result; "
    "if (5 > 3) { result = 1; } "
    "else { result = 0; } "
    "return result;")

TEST_MAIN(Made_B_conditional_nested, 42,
    "int result; "
    "if (true) { "
    "  if (false) { result = 0; } "
    "  else { result = 42; } "
    "} else { result = 99; } "
    "return result;")

// Test for generating code for while loops
TEST_MAIN(Made_B_whileloop_sum, 55,
    "int sum; int i; sum = 0; i = 1; "
    "while (i <= 10) { sum = sum + i; i = i + 1; } "
    "return sum;")

TEST_MAIN(Made_B_whileloop_break, 5,
    "int count; int i; count = 0; i = 0; "
    "while (true) { "
    "  i = i + 1; "
    "  if (i > 5) { break; } "
    "  count = count + 1; "
    "} "
    "return count;")

// Test for generating code for continue statement
// TEST_MAIN(Made_B_whileloop_continue, 20,
//     "int total; int i; total = 0; i = 0; "
//     "while (i < 10) { "
//     "  i = i + 1; "
//     "  if (i % 2 == 0) { continue; } "
//     "  total = total + i; "
//     "} "
//     "return total;")

// Test for combining conditionals and loops with breaks and continues
// TEST_MAIN(Made_B_complex_loop, 15,
//     "int sum; int i; sum = 0; i = 1; "
//     "while (i <= 10) { "
//     "  if (i % 3 == 0) { i = i + 1; continue; } "
//     "  if (i == 8) { break; } "
//     "  sum = sum + i; "
//     "  i = i + 1; "
//     "} "
//     "return sum;")

//Couple arrays tests
// Test array declaration and element access
TEST_MAIN(A_Array_declaration_access, 5,
    "int arr[3]; arr[0] = 1; arr[1] = 2; arr[2] = 5; "
    "return arr[2];")

// Test array initialization and element access with mixed values
TEST_PROGRAM(A_Array_mixed_values, 10,
    "int arr[3]; def int main() { arr[0] = 1; arr[1] = 2; arr[2] = 5; "
    "int sum; sum = arr[0] + arr[1] + arr[2]; "
    "return sum;}")
// Test array assignment within a loop
TEST_MAIN(A_Array_loop_initialization, 15,
    "int arr[5]; int i; int sum; sum = 0; "
    "for (i = 0; i < 5; i = i + 1) { arr[i] = i + 1; } "
    "for (i = 0; i < 5; i = i + 1) { sum = sum + arr[i]; } "
    "return sum;")

// Test calculating sum of an array
TEST_MAIN(A_Array_sum, 6,
    "int arr[3]; arr[0] = 1; arr[1] = 2; arr[2] = 3; "
    "int sum; sum = arr[0] + arr[1] + arr[2]; "
    "return sum;")

// Test array out-of-bounds access (expecting error or some indication of failure)
// Uncomment this line if the compiler has a way to handle or report errors for out-of-bounds access
// TEST_MAIN(Array_out_of_bounds, 0,
//     "int arr[2]; arr[3] = 10; "
//     "return 0;")

// Test using an array as a return value in a function (requires array function support)
TEST_PROGRAM(A_Array_return_function, 12,
    "def int get_third_element(int arr[3]) { return arr[2]; } "
    "def int main() { int arr[3]; arr[0] = 4; arr[1] = 8; arr[2] = 12; "
    "return get_third_element(arr); }")

// Test modifying array elements within a function
TEST_PROGRAM(A_Array_modify_function, 20,
    "def void set_elements(int arr[3]) { arr[0] = 5; arr[1] = 10; arr[2] = 20; } "
    "def int main() { int arr[3]; set_elements(arr); "
    "return arr[2]; }")

// Test multi-dimensional array access
TEST_MAIN(A_Multi_dimensional_array_access, 8,
    "int arr[2][2]; arr[0][0] = 3; arr[0][1] = 5; arr[1][0] = 2; arr[1][1] = 8; "
    "return arr[1][1];")

#endif

/**
 * @brief Register all test cases
 * 
 * @param s Test suite to which the tests should be added
 */
void public_tests (Suite *s)
{
    TCase *tc = tcase_create ("Public");

    // Register D-level tests
    TEST(D_sanity_zero);
    TEST(D_expr_int);
    TEST(D_expr_add);
    TEST(D_expr_long);
    TEST(D_expr_sanity_zero);

    // Register C-level tests
    TEST(C_expr_add);
    TEST(C_expr_subtract);
    TEST(C_expr_multiply);
    TEST(C_expr_divide);
    TEST(C_expr_negation);
    TEST(C_expr_combined);
    TEST(C_assignment_simple);
    TEST(C_global_assignment_mult);
    TEST(C_global_assignment);
    TEST(C_location_access);
    TEST(C_expr_complex);
    TEST(C_expr_parentheses);
    TEST(C_expr_precedence);
    TEST(C_multiple_vars);
    TEST(C_var_reuse);

    // Register B-level tests
    TEST(B_expr_true);
    TEST(B_expr_false);
    TEST(B_expr_not);
    TEST(B_expr_not_t);
    TEST(B_if_true);
    TEST(B_if_false);
    TEST(B_conditional);
    TEST(B_whileloop_simple);
    TEST(B_whileloop);
    TEST(B_whileloop_break);
    TEST(B_whileloop_continue);
    TEST(B_nested_conditionals);
//     TEST(B_whileloop_mixed);
    TEST(B_whileloop_break_early);
//     TEST(B_whileloop_continue_even_sum);
    TEST(B_function_noparams);
    TEST(B_funccall);
    TEST(B_function_conditional);
    TEST(B_function_in_expression);
    TEST(B_multiple_function_calls);

    // Register A-level test
    TEST(A_funccall_params);
    TEST(A_Array_declaration_access);
    TEST(A_Array_mixed_values);
    TEST(A_Array_loop_initialization);
    TEST(A_Array_sum);
    // Uncomment the following line if out-of-bounds handling is implemented
    // TEST(Array_out_of_bounds);
    TEST(A_Array_return_function);
    TEST(A_Array_modify_function);
    TEST(A_Multi_dimensional_array_access);

    TEST(Made_B_conditional_if_else);
    TEST(Made_B_conditional_nested);
    TEST(Made_B_whileloop_sum);
    TEST(Made_B_whileloop_break);
    //TEST(Made_B_whileloop_continue);
//     TEST(Made_B_complex_loop);

    suite_add_tcase (s, tc);
}
