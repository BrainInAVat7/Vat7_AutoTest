/* Copyright: (c) 2026 BrainInAVat7
 * License: AGPL-3.0
 *
 * Source code for a simple automated testing framework
 *
 * This framework provides macros for defining test cases, assertions that
 * generate useful messaging for failed test, and straightforward test
 * registration and running.
 *
 * It does not support a command line interface or automatic test discovery.
 *
 * It does support multiple test suites, test setup and teardown,
 * test suite setup and teardown, and many assertion macros.
 *
 * There can only be one assert per test case.
 */


#include <autotest.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>


/* Register a test in the test suite registry
 *
 * Returns true on success and false on failure to register.
 */
bool _ENG_autotest_register_test
	(char *test_name, AT_TestFunction test_func, AT_TestSuite *suite)
{
	if (suite->test_count >= suite->max_test_count)
	{
		char *name = suite->suite_name;
		printf("Not enough memory in test suite registry %s\n", name);
		printf("Try increasing max_test_count.\n");
		return false;
	}
	AT_TestCase test_case = {test_name, test_func};
	suite->registry[suite->test_count] = test_case;
	suite->test_count++;
	return true;
}


/* Run tests in a given test suite.
 */
void _ENG_autotest_run_tests (AT_TestSuite *suite)
{
	size_t pass_count = 0;
	size_t fail_count = 0;
	size_t test_count = suite->test_count;
	AT_TestCase *tests = suite->registry;

	printf("\nRunning tests from test suite '%s'\n", suite->suite_name);

	if (suite->suite_setup != NULL) suite->suite_setup();

	for (size_t i = 0; i < test_count; i++)
	{
		if (suite->test_setup != NULL) suite->test_setup();
		AT_TestResult result = tests[i].test_function();
		if (suite->test_teardown != NULL) suite->test_teardown();
		bool passed = result.passed;
		if (passed)
		{
			pass_count++;
		}
		else
		{
			printf("\nFAIL: %s\n", tests[i].test_name);
			printf("Line %d of file ", result.line);
			printf("%s\n", result.file);
			if (!result.snprintf_error)
			{
				printf("%s\n", result.fail_message);
			}
			else
			{
				printf("snprintf error in assert macro\n");
				printf("Could not generate failure message\n");
			}
			fail_count++;
		}
	}

	if (suite->suite_teardown != NULL) suite->suite_teardown();

	printf("\n\t%zu of %zu tests passed\n", pass_count, test_count);
	if (fail_count)
	{
		printf("\t%zu of %zu tests failed\n", fail_count, test_count);
	}
	printf("\n");
}
