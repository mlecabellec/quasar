/**
 * @file main.cpp
 * @brief GoogleTest test-runner entry point for the Quasar calibration test suite.
 * @details Initializes the GoogleTest framework with CLI arguments and executes all registered
 * calibration unit tests in the linked binary.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents the test-runner entry point under CS-0090.1 through CS-0090.9.
 */
// Include GoogleTest framework main declarations.
#include <gtest/gtest.h>

/**
 * @brief Process entry point initializing and executing the calibration GoogleTest suite.
 * @details Forwards command-line arguments to GoogleTest initialization and returns the
 * aggregated test execution exit status to the operating system.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents argc/argv contracts, return code semantics, and exception containment
 * for the calibration unit-test binary (CS-0090.1..CS-0090.9).
 * @param[in] argc
 *   Justification: Argument count passed from the OS/test harness.
 *   Usage: Forwarded by address to testing::InitGoogleTest().
 *   Consumed: Read and potentially adjusted by GoogleTest flag stripping.
 *   Prerequisites: Non-negative integer >= 0 matching argv length.
 * @param[in,out] argv
 *   Justification: Argument vector carrying executable name and GoogleTest flags.
 *   Usage: Passed to testing::InitGoogleTest() for flag parsing and filtering.
 *   Consumed: GoogleTest-recognized flags may be compacted out of argv.
 *   Prerequisites: Valid array of argc+1 pointers terminated by nullptr when argc > 0.
 * @return Exit status code from RUN_ALL_TESTS() (0 on full pass, non-zero on failure).
 *   Justification: Communicates test suite pass/fail status to CTest and shell callers.
 *   Usage: Returned directly from main to the C runtime.
 *   Consumed: Value returned to OS process exit handler; no ownership transfer.
 *   Prerequisites: testing::InitGoogleTest() completed prior to RUN_ALL_TESTS().
 *   Limitations: Aggregates all test cases into a single integer exit code.
 *   Lifecycle: Scalar value destroyed upon process termination.
 *   Modification / Deletion: Caller does not modify; used solely as process exit status.
 *   Specific Values: 0 indicates all enabled tests passed; 1 indicates at least one failure.
 *   Error Behavior: Uncaught exceptions in setup or tests may terminate or yield non-zero code.
 * @pre GoogleTest runtime environment is link-initialized.
 * @post All registered TEST cases not excluded by --gtest_filter have executed.
 * @throws std::exception if test harness initialization or an unhandled test body throws.
 * @containment
 *   Approach: GoogleTest catches standard exceptions inside individual TEST scopes.
 *   Limitations: Exceptions thrown during global initialization outside TEST bodies propagate.
 */
int main(int argc, char **argv) {
    // Parse GoogleTest command-line options before running test suites.
    ::testing::InitGoogleTest(&argc, argv);
    // Execute all registered unit tests and propagate aggregate status code.
    return RUN_ALL_TESTS();
}
