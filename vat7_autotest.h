/* Copyright: (c) 2026 BrainInAVat7
 * License: AGPL-3.0
 *
 * Header for a simple automated testing framework
 *
 * This framework provides macros for defining test cases, assertions
 * that generate useful messaging for failed test, and a
 * straightforward way to register and run tests.
 *
 * It does not support a command line interface or automatic test discovery.
 *
 * It is restricted to one assert per test case. Multiple criteria
 * can be tested by combining conditions in a basic assert with
 * logical "and". Beyond this, it is best practice to limit tests
 * to a single assertion anyway, so the limitation aligns with best
 * practice in any case.
 *
 * The framework does support multiple test suites. Usage requires
 * calling the RUN_TESTS macro on each test suite in a user defined
 * main function.
 *
 * See usage examples here and below for more information.
 *
 *
 * USAGE EXAMPLE
 *
 * 	AT_TEST_SUITE_SETUP
 * 	{
 * 		<suite setup code goes here>
 * 	}
 *
 * 	AT_TEST_SUITE_TEARDOWN
 * 	{
 * 		<suite teardown code goes here>
 * 	}
 *
 * 	AT_TEST_SETUP
 * 	{
 * 		<test setup code goes here>
 * 	}
 *
 * 	AT_TEST_TEARDOWN
 * 	{
 * 		<test teardown code goes here>
 * 	}
 *
 * 	AT_TEST_CASE(test_function_name, "A string describing the test")
 * 	{
 * 		int x = 1;
 * 		int y = 2;
 * 		AT_ASSERT(x < y);
 * 	}
 *
 * 	AT_TEST_CASE(another_test_function, "Test for blah blah blah")
 * 	{
 * 		int x = 1;
 * 		int y = 2;
 * 		AT_ASSERT(x < y);
 * 	}
 *
 * 	int test_suite_function (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("My Tests", 20);
 *
 * 		AT_TEST_SUITE_SETUP_REGISTER;
 * 		AT_TEST_SUITE_TEARDOWN_REGISTER;
 * 		AT_TEST_SETUP_REGISTER;
 * 		AT_TEST_TEARDOWN_REGISTER;
 *
 *		AT_TEST_REGISTER(test_function_name);
 *		AT_TEST_REGISTER(another_test_function);
 *
 * 		AT_RUN_TESTS;
 * 	}
 */


#ifndef VAT7_AUTOTEST_H
#define VAT7_AUTOTEST_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/* TYPES
 * Do not instantiate these directly in usage code.
 * Use the provided macros.
 */

typedef struct
{
	char *file;
	int line;
	char fail_message[242];
	bool passed;
	bool snprintf_error;  // for now the only catchable internal error
} AT_TestResult;


typedef AT_TestResult (*AT_TestFunction)(void);
typedef void (*AT_TestHelper)(void);  // used for setup and teardown functions


typedef struct
{
	char *test_name;
	AT_TestFunction test_function;
} AT_TestCase;


typedef struct
{
	char *suite_name;
	AT_TestCase *registry;
	size_t test_count;
	size_t max_test_count;
	AT_TestHelper suite_setup;
	AT_TestHelper suite_teardown;
	AT_TestHelper test_setup;
	AT_TestHelper test_teardown;
} AT_TestSuite;


/* INTERNAL FUNCTIONS
 * Do not use these directly in usage code.
 * Use the provided macros.
 */
bool _ENG_autotest_register_test (char *, AT_TestFunction, AT_TestSuite *);
void _ENG_autotest_run_tests (AT_TestSuite *);


/* HELPER MACROS
 * These are to help define assertion macros.
 * Do not use these directly in usage code.
 * Use the provided macros below.
 *
 * NOTE: This should be called on the first line of assert macros
 * to ensure the line data in the test result is accurate.
 */
#define AT_INIT_TEST_RESULT int AT_line = __LINE__; \
	AT_TestResult test_result; \
	test_result.file = __FILE__; \
	test_result.line = AT_line; \


#define AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT(fail_message_format_str, ...) \
	int at_snpf_return = snprintf(test_result.fail_message, \
		sizeof(test_result.fail_message), \
		fail_message_format_str, __VA_ARGS__); \
	test_result.snprintf_error = at_snpf_return < 0 ? true : false; \
	return test_result


/* Explicitly casts float to double to prevent errors from implicit
 * cast by snprintf calls inside macros
 */
#define AT_CAST_FLOAT(x) _Generic((x), float: (double)(x), default: (x))


/*****************************************************************************/

/* USER MACROS START HERE. NOTHING ABOVE THIS SHOULD BE CALLED BY
 * USER CODE!
 */

/*****************************************************************************/



/* TEST CASE DEFINITION MACROS */

/* Define a test case.
 *
 * This macro declares a test case function and generates a name
 * spaced name for it based on the passed <function> argument.
 * It also inserts the first line of the test case function definition.
 * This macro should be used with the provided assertion macros, which
 * provide the return statement for the function definition began by
 * this macro.
 *
 * IMPORTANT NOTES
 * *** ONLY ONE ASSERTION MACRO MAY BE CALLED PER TEST CASE. ***
 * *** TESTS MUST BE REGISTERED TO RUN. (see AT_TEST_REGISTER) ***
 *
 * Parameters:
 * 	function : a plain text function name that will be used to
 * 		register the test. Must be a valid C function name.
 * 	str_name : a string name or description of what the test does.
 * 		This is used for reporting when a test fails.
 *
 * USAGE EXAMPLE:
 *
 * 	AT_TEST_CASE(test_function_name, "A string describing the test")
 * 	{
 * 		int x = 1;
 * 		int y = 2;
 * 		AT_ASSERT(x < y);
 * 	}
 */
#define AT_TEST_CASE(function, str_name) \
	static char * ENG_autotest_str_name_##function = str_name ; \
	static AT_TestResult ENG_autotest_test_##function (void); \
	static AT_TestResult ENG_autotest_test_##function (void)


/* Assert that an expression is true in a test case.
 *
 * Assert macros should only be used within brackets after using the
 * TEST_CASE macro. See AT_TEST_CASE for example usage.
 *
 * *** ASSERT MACROS REQUIRE A TRAILING SEMI-COLON. ***
 *
 * See documentation for AT_TEST_CASE for basic assertion usage.
 * See general documentation for MACROS TO REGISTER AND RUN TESTS
 * for more advanced usage.
 */
#define AT_ASSERT(test_expression) \
	AT_INIT_TEST_RESULT \
	test_result.passed = test_expression ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"Assertion '%s' failed", #test_expression)


/* MACROS TO REGISTER AND RUN TESTS */

/* Intended usage is to declare and define test suite functions
 * and then call them inside the main entrypoint for the binary
 * that runs the desired test suites.
 *
 * Each test suite function will return EXIT_SUCCESS (i.e., 0)
 * or EXIT_FAILURE (i.e., 1), so it is expected that this will
 * be captured and used to generate the return value for main.
 *
 * A test suite function must begin by calling AT_TEST_SUITE_INIT
 * and it must end by calling AT_RUN_TESTS.
 *
 * *** IMPORTANT NOTE ***
 *
 * Test suites statically store an array of test cases. These
 * cases are not huge, but a very large number of tests in
 * a single suite could lead to stack overflow. So, it's
 * best not to register too many tests in a single test suite.
 *
 * *** END IMPORTANT NOTE ***
 *
 *
 * Example:
 *
 * inside test_thing.h
 *
 * 	int test_thing (void);
 *
 *
 * inside test_thing.c
 *
 * 	<define test cases here using TEST_CASE macro>
 *
 * 	int test_thing (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("test the thing", 2);
 * 		AT_TEST_REGISTER(test_thing_property);
 * 		AT_TEST_REGISTER(test_thing_other_property);
 * 		AT_RUN_TESTS;
 * 	}
 *
 *
 * inside main
 *
 * 	int main (void)
 * 	{
 *		int ret;
 *		ret = test_thing();
 *		return ret;
 * 	}
 */

/* Initialize the test suite so that test functions can be registered
 * and run. This macro must be called first in a test suite function.
 * (See MACROS TO REGISTER AND RUN TESTS.)
 *
 * *** NOTE: A TRAILING SEMI-COLON IS REQUIRED (See usage example)
 *
 * PARAMETERS:
 * 	char * test_suite_name : A string name for the test suite used
 * 		for reporting test results.
 * 	size_t suite_max_test_count : An unsigned integer indicating
 * 		how large the test registry should be. It must be
 * 		greater than or equal to the number of test cases;
 * 		otherwise, tests will fail to register and the tests
 * 		suite will print an error message when run.
 *
 * USAGE EXAMPLE
 * First, define test cases (see AT_TEST_CASE), then register and run
 * as follows:
 *
 * 	int test_suite_function (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("My Tests", 3);
 *		AT_TEST_REGISTER(test_function_name);
 *		AT_TEST_REGISTER(another_test_function);
 * 		AT_RUN_TESTS;
 * 	}
 */
#define AT_TEST_SUITE_INIT(test_suite_name, suite_max_test_count) \
	bool ENG_autotest_register_success; \
	static AT_TestSuite ENG_autotest_test_suite; \
	ENG_autotest_test_suite.suite_name = test_suite_name; \
	ENG_autotest_test_suite.test_count = 0u; \
	ENG_autotest_test_suite.max_test_count = suite_max_test_count; \
	static AT_TestCase ENG_autotest_suite_registry[suite_max_test_count]; \
	ENG_autotest_test_suite.registry = ENG_autotest_suite_registry; \
	ENG_autotest_test_suite.suite_setup = NULL; \
	ENG_autotest_test_suite.suite_teardown = NULL; \
	ENG_autotest_test_suite.test_setup = NULL; \
	ENG_autotest_test_suite.test_teardown = NULL


/* Register a TEST CASE to be run.
 *
 * *** NOTE: A TRAILING SEMI-COLON IS REQUIRED (See usage example)
 *
 * PARAMETERS:
 * 	function : a plain text function name. It must be a name
 * 		associated with a previously defined TEST CASE.
 * 		See usage example below.
 *
 * USAGE EXAMPLE:
 *
 * 	AT_TEST_CASE(test_function_name, "A string describing the test")
 * 	{
 * 		int x = 1;
 * 		int y = 2;
 * 		AT_ASSERT(x < y);
 * 	}
 *
 * 	int test_suite_function (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("My Tests", 1);
 *		AT_TEST_REGISTER(test_function_name);
 * 		AT_RUN_TESTS;
 * 	}
 */
#define AT_TEST_REGISTER(function) \
	ENG_autotest_register_success = \
		_ENG_autotest_register_test(ENG_autotest_str_name_##function, \
			ENG_autotest_test_##function, \
			&ENG_autotest_test_suite); \
	if (!ENG_autotest_register_success) return EXIT_FAILURE


/* Run the test suite.
 *
 * This must be called last inside a test suite function.
 *
 * USAGE EXAMPLE:
 * First, define test cases (see AT_TEST_CASE), then register and run
 * as follows:
 *
 * 	int test_suite_function (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("My Tests", 1);
 *		AT_TEST_REGISTER(test_function_name);
 * 		AT_RUN_TESTS;
 * 	}
 */
#define AT_RUN_TESTS \
	_ENG_autotest_run_tests(&ENG_autotest_test_suite); \
	return EXIT_SUCCESS
// TODO Remove assumption user doesn't want to do anything after
// running tests.


/* SETUP AND TEARDOWN MACROS */

/* Define code that will be run before the test suite is run.
 * TEST_SUITE_SETUP_REGISTER must be called inside a test
 * suie function for the code to actually run.
 *
 * See TEST_SUITE_SETUP_REGISTER.
 *
 * NOTE: Curly braces should be used as with a function definition.
 *
 * USAGE EXAMPLE:
 * 	AT_TEST_SUITE_SETUP
 * 	{
 * 		<suite setup code goes here>
 * 	}
 */
#define AT_TEST_SUITE_SETUP \
	static void ENG_autotest_test_suite_setup (void); \
	static void ENG_autotest_test_suite_setup (void)


/* Define code that will be run after the test suite is run.
 * TEST_SUITE_SETUP_REGISTER must be called inside a test
 * suie function for the code to actually run.
 *
 * See TEST_SUITE_SETUP_REGISTER.
 *
 * NOTE: Curly braces should be used as with a function definition.
 *
 * USAGE EXAMPLE:
 * 	AT_TEST_SUITE_TEARDOWN
 * 	{
 * 		<suite teardown code goes here>
 * 	}
 */
#define AT_TEST_SUITE_TEARDOWN \
	static void ENG_autotest_test_suite_teardown (void); \
	static void ENG_autotest_test_suite_teardown (void)


/* Define code that will be run before each test is run.
 * TEST_SETUP_REGISTER must be called inside a test suite
 * function for the code to actually run.
 *
 * See TEST_SETUP_REGISTER.
 *
 * NOTE: Curly braces should be used as with a function definition.
 *
 * USAGE EXAMPLE:
 * 	AT_TEST_SETUP
 * 	{
 * 		<test setup code goes here>
 * 	}
 */
#define AT_TEST_SETUP \
	static void ENG_autotest_test_setup (void); \
	static void ENG_autotest_test_setup (void)


/* Define code that will be run after each test is run.
 * TEST_TEARDOWN_REGISTER must be called inside a test
 * suite function for the code to actually run.
 *
 * See TEST_TEARDOWN_REGISTER.
 *
 * NOTE: Curly braces should be used as with a function definition.
 *
 * USAGE EXAMPLE:
 * 	AT_TEST_TEARDOWN
 * 	{
 * 		<test teardown code goes here>
 * 	}
 */
#define AT_TEST_TEARDOWN \
	static void ENG_autotest_test_teardown (void); \
	static void ENG_autotest_test_teardown (void)


/* Register setup and teardown code.
 * The corresponding setup or teardown code must be defined above
 * the relevant test suite function and the relevant REGISTER macro
 * invoked inside that test suite function.
 *
 * NOTE: A TRAILING SEMI-COLON MUST BE INCLUDED. (See usage example)
 *
 * USAGE EXAMPLE
 *
 * 	AT_TEST_SUITE_SETUP
 * 	{
 * 		<suite setup code goes here>
 * 	}
 *
 * 	AT_TEST_SUITE_TEARDOWN
 * 	{
 * 		<suite teardown code goes here>
 * 	}
 *
 * 	AT_TEST_SETUP
 * 	{
 * 		<test setup code goes here>
 * 	}
 *
 * 	AT_TEST_TEARDOWN
 * 	{
 * 		<test teardown code goes here>
 * 	}
 *
 * 	AT_TEST_CASE(test_function_name, "A string describing the test")
 * 	{
 * 		int x = 1;
 * 		int y = 2;
 * 		AT_ASSERT(x < y);
 * 	}
 *
 * 	int test_suite_function (void)
 * 	{
 * 		AT_TEST_SUITE_INIT("My Tests", 1);
 * 		AT_TEST_SUITE_SETUP_REGISTER;
 * 		AT_TEST_SUITE_TEARDOWN_REGISTER;
 * 		AT_TEST_SETUP_REGISTER;
 * 		AT_TEST_TEARDOWN_REGISTER;
 *		AT_TEST_REGISTER(test_function_name);
 * 		AT_RUN_TESTS;
 * 	}
 */
#define AT_TEST_SUITE_SETUP_REGISTER \
	ENG_autotest_test_suite.suite_setup = ENG_autotest_test_suite_setup


#define AT_TEST_SUITE_TEARDOWN_REGISTER \
	ENG_autotest_test_suite.suite_teardown = \
		ENG_autotest_test_suite_teardown


#define AT_TEST_SETUP_REGISTER \
	ENG_autotest_test_suite.test_setup = ENG_autotest_test_setup


#define AT_TEST_TEARDOWN_REGISTER \
	ENG_autotest_test_suite.test_teardown = ENG_autotest_test_teardown


/* ALTERNATIVE ASSERTION MACROS
 *
 * These may be used in place of the basic AT_ASSERT macro
 * to generate more detailed, type-specific, test failure messages.
 *
 * Assert macros should only be used within brackets after using the
 * TEST_CASE macro. See TEST_CASE for example usage.
 *
 * *** ASSERT MACROS REQUIRE A TRAILING SEMI-COLON. ***
 *
 * See documentation for AT_ASSERT for more detail.
 *
 * Many of these take string format specifiers as arguments.
 * While it makes usage code verbose, explictly passing format specifiers
 * allows flexibility and control in failure messagine, for example
 * choosing whether integers will be displayed in decimal, octal, or hex
 * without proliferation of assertion macros.
 */

/* NULL POINTER ASSERTIONS
 *
 * These take a single argument, a variable of any pointer type.
 * See general documentation above on ALTERNATIVE ASSERTION MACROS
 */
#define AT_ASSERT_NULL(ptr) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (ptr) == NULL ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"Assertion '" #ptr \ " is NULL' failed: '%p' is not NULL", ptr)


#define AT_ASSERT_NOT_NULL(ptr) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (ptr) != NULL ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"Assertion '" #ptr " not NULL' failed: '%p' is NULL", ptr)


/* BOOLEAN ASSERTIONS
 *
 * While the generic AT_ASSERT tests the truth value of an expression,
 * AT_ASSERT_TRUE and AT_ASSERT_FALSE test the value of an individual
 * variable.
 *
 * Parameters:
 * 	arg : The variable asserted to evaluate to true or false
 * 	fmt : A format specifier string for failure messaging to report
 * 		the value of the variable (e.g., "%d" for decimal integers)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */
#define AT_ASSERT_TRUE(arg, fmt) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_TRUE (" #arg ") failed: " \
		fmt " evaluates to false not true", AT_CAST_FLOAT(arg))


#define AT_ASSERT_FALSE(arg, fmt) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg) ? false : true; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_FALSE (" #arg ") failed: " \
		fmt " evaluates to true not false", AT_CAST_FLOAT(arg))


/* COMPARISON ASSERTIONS
 *
 * Can be used to compare any type that can be used with the built in
 * comparison operators (<, >, ==, !=, <=, >=).
 *
 * Parameters:
 * 	arg_1 : The left value for comparison
 * 	arg_2 : The right value for comparison
 * 	fmt_1 : A format specifier string for reporting the value of arg_1
 * 		in failure messaging. (e.g., "%d" for decimal integers)
 * 	fmt_2 : A format specifier string for reporting the value of arg_2
 * 		in failure messaging. (e.g., "%d" for decimal integers)
 *
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */
#define AT_ASSERT_EQUAL(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) == (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_EQUAL (" #arg_1 ", " #arg_2 ") failed: "\
		fmt_1 " not equal to " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


#define AT_ASSERT_LESS(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) < (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_LESS (" #arg_1 ", " #arg_2 ") failed: " \
		fmt_1 " not less than " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


#define AT_ASSERT_GREATER(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) > (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_GREATER (" #arg_1 ", " #arg_2 ") failed: " \
		fmt_1 " not greater than " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


#define AT_ASSERT_NOT_EQUAL(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) != (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_EQUAL (" #arg_1 ", " #arg_2 ") failed: " \
		fmt_1 " equals " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


#define AT_ASSERT_NOT_LESS(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) >= (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_LESS (" #arg_1 ", " #arg_2 ") failed: " \
		fmt_1 " less than " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


#define AT_ASSERT_NOT_GREATER(arg_1, arg_2, fmt_1, fmt_2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) <= (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_GREATER (" #arg_1 ", " #arg_2 ") failed: " \
		fmt_1 " greater than " fmt_2, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2))


/* NEAR ASSERTIONS
 *
 * Used to test whether two values are within a given distance from
 * on another. This is primarily intended for floating point comparison
 * but can be used with any type for which arithmetic and comparison
 * operators can be used (e.g., +, >)
 *
 * Parameters:
 * 	arg_1 : (short for argument 1) The left value for comparison
 * 	arg_2 : (short for argument 2) The right value for comparison
 * 	ep    : (short for epsilon) The tolerance threshold for nearness
 * 		The test is whether arg_2 is in the range of
 * 		arg_1 + or - ep.
 * 		The test is inclusive of boundary points.
 * 	fmt_1 : A format specifier string for reporting the value of arg_1
 * 		in failure messaging. (e.g., "%f" for floats in decimal)
 * 	fmt_2 : A format specifier string for reporting the value of arg_2
 * 		in failure messaging. (e.g., "%f" for floats in decimal)
 * 	fmt_e : A format specifier string for reporting the value of ep
 * 		in failure messaging. (e.g., "%f" for floats in decimal)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */
#define AT_ASSERT_NEAR(arg_1, arg_2, ep, fmt_1, fmt_2, fmt_e) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) + (ep) >= (arg_2) \
		&& (arg_1) - (ep) <= (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NEAR (" #arg_1 ", " #arg_2 ", " #ep ") failed: " \
		"distance between "fmt_1 " and " fmt_2 " more than " fmt_e, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2), AT_CAST_FLOAT(ep))


#define AT_ASSERT_NOT_NEAR(arg_1, arg_2, ep, fmt_1, fmt_2, fmt_e) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (arg_1) + (ep) < (arg_2) \
		|| (arg_1) - (ep) > (arg_2) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_NEAR (" #arg_1 ", " #arg_2 ", " #ep ") failed: "\
		"distance between "fmt_1 " and " fmt_2 " less than " fmt_e, \
		AT_CAST_FLOAT(arg_1), AT_CAST_FLOAT(arg_2), AT_CAST_FLOAT(ep))


/* IN RANGE ASSERTIONS
 *
 * Used to test whether a value is within a range defined by a given
 * minimum and maximum value. The range is inclusive of boundary values.
 * Can be used for any type compatible with C builtin comparison
 * operators (e.g., <, >, ==)
 *
 * Parameters:
 * 	v   : The value to be tested for being within the specified range
 * 	mn  : The minimum value of the range.
 * 	mx  : The maximu value of the range.
 * 	fv  : A format specifier for reporting the value of arg
 * 		in failure messaging. (e.g., "%d" for decimal integers)
 * 	fmn : A format specifier string for reporting the value of min
 * 		in failure messaging. (e.g., "%d" for decimal integers)
 * 	fmx : A format specifier string for reporting the value of max
 * 		in failure messaging. (e.g., "%d" for decimal integers)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */
#define AT_ASSERT_IN_RANGE(v, mn, mx, fv, fmn, fmx) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (mn) <= (v) && (mx) >= (v) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_IN_RANGE (" #v ", " #mn ", " #mx ") failed: "\
		fv " not in range (" fmn ", " fmx ")", \
		AT_CAST_FLOAT(v), AT_CAST_FLOAT(mn), AT_CAST_FLOAT(mx))


#define AT_ASSERT_NOT_IN_RANGE(v, mn, mx, fv, fmn, fmx) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (mn) >= (v) || (mx) <= (v) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_IN_RANGE (" #v ", " #mn ", " #mx ") failed: "\
		fv " in range (" fmn ", " fmx ")", \
		AT_CAST_FLOAT(v), AT_CAST_FLOAT(mn), AT_CAST_FLOAT(mx))


/* ARRAY ASSERTIONS */

/* VALUES IN ARRAYS
 *
 * Parameters:
 * 	v   : The value asserted to be in or not in the array
 * 	a   : The array
 * 	len : The length of the array, or simply the highest index
 * 		to be checked if not all values are initialized
 *	fv  : A format specifier string for v used in reporting
 *		if the test fails (e.g., "%f" for floats in decimal)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */

#define AT_ASSERT_IN_ARRAY(v, a, len, fv) \
	AT_INIT_TEST_RESULT \
	test_result.passed = false; \
	for (size_t AT_index = 0; AT_index < (len); i++) \
	{ \
		if ((a)[AT_index] == (v)) \
		{ \
			test_result.passed = true; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_IN_ARRAY (" #v ", " #a ") failed: " fv " not found", \
		AT_CAST_FLOAT(v))


/* NOTE: found_value_at_index defaults to 0, but this is fine because
 * the message will not print if the test passes.
 */
#define AT_ASSERT_NOT_IN_ARRAY(v, a, len, fv) \
	AT_INIT_TEST_RESULT \
	test_result.passed = true; \
	size_t at_found_value_at_index = 0; \
	for (size_t AT_index = 0; AT_index < (len); AT_index++) \
	{ \
		if ((a)[AT_index] == (v)) \
		{ \
			test_result.passed = false; \
			at_found_value_at_index = AT_index; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_NOT_IN_ARRAY (" #v ", " #a ") failed: " \
		fv " found at index %zu", \
		AT_CAST_FLOAT(v), at_found_value_at_index)


/* COMPONENT-WISE ARRAY EQUALITY
 * Works with any element type that works with built in comparison
 * operators
 *
 * Parameters: (do not use fa1 or fa2 for NOT_EQUAL)
 * 	a1  : The array whose components will be the left value in
 * 	      the comparison
 * 	a2  : The array whose components will be the right value in
 * 	      the comparison
 * 	len : The length of the a1 and a2, or simply the highest index
 * 		to be checked if not all values are initialized or
 * 		the desire is to check only part 2 arrays of different
 * 		length.
 *	fa1 : A format specifier string for components of a1
 *		used in reporting if the test fails
 * 		(e.g., "%f" if a1 is an array of floats)
 *	fa2 : A format specifier string for components of a2
 *		used in reporting if the test fails
 *		(e.g., "%f" if a1 is an array of floats)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */

/* NOTE: mismatch_index defaults to 0, but this is fine because
 * the message will only print if the test fails.
 */
#define AT_ASSERT_ARRAY_EQUAL(a1, a2, len, fa1, fa2) \
	AT_INIT_TEST_RESULT \
	test_result.passed = true; \
	size_t mismatch_index = 0; \
	for (size_t AT_index = 0; AT_index < (len); AT_index++) \
	{ \
		if ((a1)[AT_index] != (a2)[AT_index]) \
		{ \
			test_result.passed = false; \
			mismatch_index = AT_index; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_ARRAY_EQUAL (" #a1 ", " #a2 ") failed: " \
		#a1 "[%zu] is " fa1 " but " #a2 "[%zu] is " fa2, \
		mismatch_index, AT_CAST_FLOAT(a1[mismatch_index]), \
		mismatch_index, AT_CAST_FLOAT(a2[mismatch_index]))


#define AT_ASSERT_ARRAY_NOT_EQUAL(a1, a2, len) \
	AT_INIT_TEST_RESULT \
	test_result.passed = false; \
	for (size_t AT_index = 0; AT_index < (len); AT_index++) \
	{ \
		if ((a1)[AT_index] != (a2)[AT_index]) \
		{ \
			test_result.passed = true; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"%s", "ASSERT_ARRAY_NOT_EQUAL (" #a1 ", " #a2 ") failed: " \
		"no mismatch found")


/* COMPONENT-WISE ARRAY NEARNESS
 * Intended for float comparison, but compatible with any type that
 * works with arithmetic and comparison operators
 *
 * Parameters: (do not use f1, f2, or fe for NOT_NEAR)
 * 	a1 : The array whose components will be the left value in
 * 	       the comparison
 * 	a2 : The array whose components will be the right value in
 * 		the comparison
 * 	ln : The length of the a1 and a2, or simply the highest index
 * 		to be checked if not all values are initialized or
 * 		the desire is to check only part 2 arrays of different
 * 		length.
 * 	ep : (short for epsilon) The tolerance threshold for nearness
 * 		The test is whether a2[i] is in the range of
 * 		a1[i] + or - ep.
 * 		The test is inclusive of boundary points.
 *	f1 : A format specifier string for components of a1
 *		used in reporting if the test fails
 *		(e.g., "%f" if a1 is an array of floats)
 *	f2 : A format specifier string for components of a1
 *		used in reporting if the test fails
 *		(e.g., "%f" if a1 is an array of floats)
 * 	fe : A format specifier string for reporting the value of ep
 * 		in failure messaging. (e.g., "%f" for floats in decimal)
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */

/* NOTE: mismatch_index defaults to 0, but this is fine because
 * the message will only print if the test fails.
 */
#define AT_ASSERT_ARRAY_NEAR(a1, a2, ln, ep, f1, f2, fe) \
	AT_INIT_TEST_RESULT \
	test_result.passed = true; \
	size_t mismatch_index = 0; \
	for (size_t AT_index = 0; AT_index < (ln); AT_index++) \
	{ \
		if ((a1)[AT_index] + (ep) < (a2)[AT_index] \
			|| (a1)[AT_index] - (ep) > (a2)[AT_index]) \
		{ \
			test_result.passed = false; \
			mismatch_index = AT_index; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"ASSERT_ARRAY_NEAR (" #a1 ", " #a2 ") failed: " \
		#a1 "[%zu] (" f1 ") more than " fe \
		" from " #a2 "[%zu] (" f2 ")", \
		mismatch_index, AT_CAST_FLOAT(a1[mismatch_index]), \
		AT_CAST_FLOAT(ep), \
		mismatch_index, AT_CAST_FLOAT(a2[mismatch_index]))


#define AT_ASSERT_ARRAY_NOT_NEAR(a1, a2, ln, ep) \
	AT_INIT_TEST_RESULT \
	test_result.passed = false; \
	for (size_t AT_index = 0; AT_index < (ln); AT_index++) \
	{ \
		if ((a1)[AT_index] + (ep) < (a2)[AT_index] \
			|| (a1)[AT_index] - (ep) > (a2)[AT_index]) \
		{ \
			test_result.passed = true; \
			break; \
		} \
	} \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"%s", "ASSERT_ARRAY_NOT_NEAR (" #a1 ", " #a2 ") failed: " \
		"no mismatch found")


/* BIT FLAG SET ASSERTIONS
 *
 * Parameters:
 * 	mask     : The value to be tested
 * 	position : The bit position to be checked
 *
 * For example, if the value is int 4, bit position 2 is set, and all
 * others are not set.
 *
 * See general documentation for ALTERNATIVE ASSERTION MACROS for more.
 */
#define AT_ASSERT_BIT_SET(mask, position) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (mask) & (1 << (position)) ? true : false; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"%s", "ASSERT_BIT_SET (" #mask ", " #position ") failed")


#define AT_ASSERT_BIT_NOT_SET(mask, position) \
	AT_INIT_TEST_RESULT \
	test_result.passed = (mask) & (1 << (position)) ? false : true; \
	AT_DEF_FAIL_MSG_AND_RETURN_TEST_RESULT( \
		"%s", "ASSERT_BIT_NOT_SET (" #mask ", " #position ") failed")


#endif  // Header Guard
