/**
 * @file public.c
 * @brief Public test cases (and location for new tests)
 * 
 * This file provides a few basic sanity test cases and a location to add new tests.
 */

#include "testsuite.h"

#ifndef SKIP_IN_DOXYGEN

TEST_EXPRESSION(D_expr_add,  5, "2+3")
TEST_EXPRESSION(D_expr_mul,  6, "2*3")

// Arithmetic expressions
TEST_EXPRESSION(C_expr_2regs, 10, "1+2+3+4")
TEST_EXPRESSION(C_expr_3regs, 10, "(1+2)+(3+4)")
TEST_EXPRESSION(C_assign1, 62, "2+3*4*5")
TEST_EXPRESSION(C_assign_multiple1, 12, "2+3*4-4/2*3+4")



TEST_PROGRAM(C_assign, 14, 
        "def int main() { "
        "  int a; a = 2 + 3 * 4; "
        "  return a; }")

TEST_PROGRAM(C_assign_multiple, 40, 
        "def int main() { "
        "  int a;"
        "  a = 2 + 3 * 4; " // 14
        "  a = a - 4 / 2; " // 12
        "  a = a * 3 + 4; " // 40
        "  return a; }")

TEST_PROGRAM(C_conditional, 3, 
        "def int main() { "
        "  if (true) { return 2+1; } "
        "  else { return 3+1; } }")

TEST_PROGRAM(C_assign_multiple2, 80, 
        "def int main() { "
        "  int a; "
        "  a = 2 + 3 * 4; " // 14
        "  a = a - 4 / 2; " // 12
        "  a = a * 3 + 4; " // 40
        "  a = a + 40; "    // 80
        "  return a; }")

TEST_PROGRAM(C_while, 10, 
        "def int main() { "
        "  int a; a = 0; "
        "  while (a < 10) { a = a + 1; } "
        "  return a; }")

TEST_PROGRAM(C_while1, 15, 
        "def int main() { "
        "  int a; a = 0; "
        "  while (a < 10) { a = a + 1; } "
        "  a = a + 5; "
        "  return a; }")

TEST_PROGRAM(B_func_call, 5, 
        "def int add(int a, int b) { return a + b; } "
        "def int main() { return add(2,3); }")

TEST_PROGRAM(B_spilled_regs, 72, 
        "def int main() { "
        "  return (((1+2)+(3+4))+((5+6)+(7+8)))+"
        "         (((1+2)+(3+4))+((5+6)+(7+8))); }")

// function calls test
TEST_PROGRAM(B_func_call1, 5, 
        "def int add(int a, int b) { return a + b; } "
        "def int main() { return add(2,3); }")

TEST_PROGRAM(B_func_call2, 17, 
        "def int add(int a, int b) { return a + b; } "
        "def int main() { return add(2,3) + add(4,5); }")

TEST_PROGRAM(B_func_call3, 43, 
        "def int add(int a, int b) { return a + b; } "
        "def int main() { return add(2,3) + add(4,5) + add(6,7); }")

// Recursion Tests
TEST_PROGRAM(B_recursion, 120, 
        "def int fib(int n) { "
        "  if (n <= 1) { return n; } "
        "  return fib(n-1) + fib(n-2); } "
        "def int main() { return fib(15); }")

TEST_PROGRAM(B_recursion1, 140, 
        "def int fib(int n) { "
        "  if (n <= 1) { return n; } "
        "  return fib(n-1) + fib(n-2); } "
        "def int main() { return fib(16); }")

TEST_PROGRAM(B_recursion2, 160,
        "def int fib(int n) { "
        "  if (n <= 1) { return n; } "
        "  return fib(n-1) + fib(n-2); } "
        "def int main() { return fib(17); }")


#endif

/**
 * @brief Register all test cases
 * 
 * @param s Test suite to which the tests should be added
 */
void public_tests (Suite *s)
{
        TCase *tc = tcase_create ("Public");

        TEST(D_expr_add);
        TEST(D_expr_mul);

        TEST(C_assign1);
        TEST(C_expr_2regs);
        TEST(C_expr_3regs);
        TEST(C_assign);
        TEST(C_assign_multiple);
        TEST(C_conditional);
        TEST(C_while);
        TEST(C_while1);
        TEST(C_assign_multiple1);
        TEST(C_assign_multiple2);


        // TEST(B_func_call);
        TEST(B_spilled_regs);
        // TEST(B_func_call1);
        // TEST(B_func_call2);
        // TEST(B_func_call3);
        // TEST(B_recursion);
        // TEST(B_recursion1);
        // TEST(B_recursion2);

        suite_add_tcase (s, tc);
}

