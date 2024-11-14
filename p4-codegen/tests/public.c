#include "testsuite.h"

#ifndef SKIP_IN_DOXYGEN

// D-level Tests

TEST_PROGRAM(D_sanity_zero, 0, "def int main() { return 0; }")
TEST_EXPRESSION(D_expr_int, 7, "7")
TEST_EXPRESSION(D_expr_add, 5, "2+3")
TEST_EXPRESSION(D_expr_long, 10, "1+2+3+4")
TEST_EXPRESSION(D_expr_sanity_zero, 0, "0")
TEST_PROGRAM(D_return_literal, 10, "def int main() { return 10; }")
TEST_PROGRAM(D_basic_block, 3, "def int main() { int x; int y; y = 3; x = y; return x; }")
TEST_PROGRAM(D_basic_addition, 5, "def int main() { return 2 + 3; }")

// C-level Tests

// Test binary integer operations (addition, subtraction, multiplication, division)
TEST_EXPRESSION(C_expr_add, 7, "3 + 4")
TEST_EXPRESSION(C_expr_subtract, 1, "4 - 3")
TEST_EXPRESSION(C_expr_multiply, 12, "3 * 4")
TEST_EXPRESSION(C_expr_divide, 2, "8 / 4")
TEST_EXPRESSION(C_expr_subtract_nested, 3, "(8 - 5) + (1 - 1)")
TEST_MAIN(C_precedence_test, 10, "int x; x = (2 + 3) * (4 - 2); return x;")
TEST_MAIN(C_nested_scope, 8, "int x; int y; y = 8; x = y; return x;")

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
TEST_BOOL_EXPRESSION(B_expr_nested_and_or, 1, "true && (false || true)")
TEST_MAIN(B_if_else_chain, 2, 
    "int x; "
    "if (false) { x = 1; } "
    "else { x = 2; } "
    "return x;")
TEST_MAIN(B_whileloop_decrement, 0, "int x; x = 10; while (x > 0) { x = x - 1; } return x;")
TEST_PROGRAM(B_simple_func_call, 100, 
    "def int hundred() { return 100; } "
    "def int main() { return hundred(); }")

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
TEST_MAIN(B_whileloop_mixed, 8,
    "int count; int result; "
    "count = 0; result = 0; "
    "while (count < 5) { "
    "  if (count % 2 == 0) { result = result + 2; } "
    "  else { result = result + 1; } "
    "  count = count + 1; "
    "} "
    "return result;")

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
TEST_MAIN(Made_B_whileloop_continue, 25,
    "int total; int i; total = 0; i = 0; "
    "while (i < 10) { "
    "  i = i + 1; "
    "  if (i % 2 == 0) { continue; } "
    "  total = total + i; "
    "} "
    "return total;")

// Test for combining conditionals and loops with breaks and continues
TEST_MAIN(Made_B_complex_loop, 19,
    "int sum; int i; sum = 0; i = 1; "
    "while (i <= 10) { "
    "  if (i % 3 == 0) { i = i + 1; continue; } "
    "  if (i == 8) { break; } "
    "  sum = sum + i; "
    "  i = i + 1; "
    "} "
    "return sum;")

TEST_PROGRAM(A_funccall_params_advanced, 50, 
    "def int multiply(int a, int b) { return a * b; } "
    "def int main() { return multiply(5,10); }")
TEST_PROGRAM(A_array_init_sum, 15, 
    "int arr[3]; def int main() { int sum; arr[0] = 5; arr[1] = 3; arr[2] = 7; "
    " sum = arr[0] + arr[1] + arr[2]; return sum; }")
TEST_MAIN(A_modulus_expression, 1, 
    "int a; a = 10; return a % 3;")
// Uncomment if print function implemented
// TEST_MAIN(A_print_test, 10, "print(10); return 10;")

//Couple arrays tests
// Test array declaration and element access
TEST_PROGRAM(A_Array_declaration_access, 5,
    "int arr[3]; def int main() { arr[0] = 5; return arr[0]; }")

// Test array initialization and element access with mixed values
TEST_PROGRAM(A_Array_mixed_values, 8,
    "int arr[3]; def int main() { int sum; arr[0] = 1; arr[1] = 2; arr[2] = 5; "
    " sum = arr[0] + arr[1] + arr[2]; "
    "return sum;}")
// Test array assignment within a loop
// This test works just 
TEST_PROGRAM(A_Array_loop_initialization, 15,
    "int arr[5]; def int main() { int i; int sum; sum = 0; "
    "while (i < 5) {" "arr[i] = i + 1;" "i = i + 1; }"
    "while (i > 0) {" "i = i - 1;" "sum = sum + arr[i]; }"
    "return sum; }")

// Test calculating sum of an array
TEST_PROGRAM(A_Array_sum, 6,
    "int arr[3]; def int main() { int sum; arr[0] = 1; arr[1] = 2; arr[2] = 3; "
    "sum = arr[0] + arr[1] + arr[2]; "
    "return sum;}")

// Test array out-of-bounds access (expecting error or some indication of failure)
// Uncomment this line if the compiler has a way to handle or report errors for out-of-bounds access
TEST_PROGRAM(Array_out_of_bounds, 0,
    "int arr[3]; def int main() { arr[3] = 5; return arr[4]; }")


// modulus tests
TEST_MAIN(A_Modulus_operator, 1,
    "int a; a = 5 % 2; return a;")

TEST_MAIN(A_Modulus_operator2, 0,
    "int a; a = 4 % 2; return a;")

TEST_MAIN(A_Modulus_operator3, 0,
    "int a; a = 0 % 2; return a;")
// Alot of modulus's
TEST_MAIN(A_Modulus_operator4, 0,
    "int a; a = 100 % 2; return a;")
// Alot of modulus's
TEST_MAIN(A_Modulus_operator5, 1,
    "int a; int b; a = 101 % 2; b = a % 2; return b;")

TEST_MAIN(A_Modulus_operator6, 1,
    "int a; int b; a = 100 % 3; return a;")

// Function with parameters
TEST_PROGRAM(A_funccall_params1, 5,
    "def int add(int a, int b) { return a + b; } "
    "def int main() { return add(2,3); }")
TEST_PROGRAM(A_funccall_params2, 5,
    "def int add(int a, int b) { return a + b; } "
    "def int main() { int a; int b; a = 2; b = 3; return add(a,b); }")

// Print functions TEsts
TEST_MAIN(A_Print_int_function, 5, 
    "print_int(5); return 5;")

TEST_MAIN(A_Print_str_function, 5,
    "int a; a = 5; print_str(\"HELLO WORLD\\n\"); return a;")

TEST_MAIN(A_Print_bool_function, 1,
    "print_bool(true); return 1;")

TEST_MAIN(A_whileloop_even_sum, 30,
    "int sum; int i; sum = 0; i = 1; "
    "while (i <= 10) { "
    "  if (i % 2 == 0) { sum = sum + i; } "
    "  i = i + 1; "
    "} "
    "return sum;")

TEST_PROGRAM(Made_B_function_class_example, 4,
    "int a[10]; int g; def int set(int x) { g = x; return x; } def int main() { a[set(3)] = set(4); return g; }")

TEST_PROGRAM(Made_B_complex_array_access, 5,
    "int arr[10]; int barr[10]; int carr[10]; def int main() { arr[0] = 1; arr[1] = 2; arr[2] = 3; barr[0] = 1; barr[1] = 2; barr[2] = 3; carr[0] = 0; carr[1] = 1; carr[2] = 2; return arr[barr[carr[0]]] + arr[barr[carr[1]]] + arr[barr[carr[2]]]; }")

TEST_MAIN(B_Nested_if_statements_galore, 1,
    "int a; int b; int c; int d; int e; a = 1; b = 2; c = 3; d = 4; e = 5; if (a == 1) { if (b == 2) { if (c == 3) { if (d == 4) { if (e == 5) { return 1; } } } } } else { return 0; }")
// int a[10];
// int g;

// def int set(int x) {
//     g = x;
//     return x;
// }

// def int foo(int x, int y) {
//     return x + y;
// }

// def int main() 
// {
//     if (set(3) == 3) {
//         g = 99;
//         if (set(98) == 4) {
//             return 1;
//         } else {
//             if (foo(set(4), set(4)) == 8) {
//                 return 10000;
//             }
//         }
//     }
// }

TEST_PROGRAM(A_jumbled_together_stuff, 10000,
    "int a[10]; int g; def int set(int x) { g = x; return x; } def int foo(int x, int y) { return x + y; } def int main() { if (set(3) == 3) { g = 99; if (set(98) == 4) { return 1; } else { if (foo(set(4), set(4)) == 8) { return 10000; } } } }")

// int data[5];

// def int compute(int x, int y) {
//     return (x*y) + (x-y);
// }

// def int main() {
//     int i;
//     int result;
//     i = 0;
//     result = 0;

//     while (i < 5) {
//         data[i] = compute(i, i+1);
//         result = result + data[i];
//         i = i + 1;
//     }

//     return result;
// }

TEST_PROGRAM(A_Array_sum_and_stuff, 35,
    "int data[5]; def int compute(int x, int y) { return (x*y) + (x-y); } def int main() { int i; int result; i = 0; result = 0; while (i < 5) { data[i] = compute(i, i+1); result = result + data[i]; i = i + 1; } return result; }")
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
    TEST(D_return_literal);
    TEST(D_basic_block);
    TEST(D_basic_addition);

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
    TEST(C_precedence_test);
    TEST(C_nested_scope);
    TEST(C_expr_subtract_nested);

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
    TEST(B_whileloop_mixed);
    TEST(B_whileloop_break_early);
    TEST(B_whileloop_continue_even_sum);
    TEST(B_function_noparams);
    TEST(B_funccall);
    TEST(B_function_conditional);
    TEST(B_function_in_expression);
    TEST(B_multiple_function_calls);
    TEST(B_if_else_chain);
    TEST(B_expr_nested_and_or);
    TEST(B_if_else_chain);
    TEST(B_whileloop_decrement);
    TEST(B_simple_func_call);

    // Register A-level test
    TEST(A_funccall_params);
    TEST(A_Array_declaration_access);
    TEST(A_Array_mixed_values);
    TEST(A_Array_loop_initialization);
    TEST(A_Array_sum);
    // Uncomment the following line if out-of-bounds handling is implemented
    TEST(Array_out_of_bounds);
    TEST(A_Modulus_operator);
    TEST(A_Modulus_operator2);
    TEST(A_Modulus_operator3);
    TEST(A_Modulus_operator4);
    TEST(A_Modulus_operator5);
    TEST(A_Modulus_operator6);

    // function calls
    TEST(A_funccall_params1);
    TEST(A_funccall_params2);
    TEST(A_funccall_params_advanced);
    TEST(A_array_init_sum);
    TEST(A_modulus_expression);
    TEST(A_whileloop_even_sum);

    TEST(A_Print_int_function);
    TEST(A_Print_str_function);
    // TEST(A_Print_bool_function);

    TEST(Made_B_conditional_if_else);
    TEST(Made_B_conditional_nested);
    TEST(Made_B_whileloop_sum);
    TEST(Made_B_whileloop_break);
    TEST(Made_B_whileloop_continue);
    TEST(Made_B_complex_loop);
    TEST(Made_B_function_class_example);
    TEST(Made_B_complex_array_access);
    TEST(B_Nested_if_statements_galore);

    TEST(A_jumbled_together_stuff);
    TEST(A_Array_sum_and_stuff);

    suite_add_tcase (s, tc);
}
