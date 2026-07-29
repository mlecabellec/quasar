# Quasar Project Rules and Constraints

These rules are loaded by agents and must be strictly followed when writing, modifying, or testing code in this workspace.

## Language and C++ Primitives (CS-0010, CS-0020)

*   **[CS-0010.1]** Use C++ 23 or higher. If definitions from standards higher than C++ 23 are used, the code shall be compatible with C++ 23 (at least regarding interfaces).
*   **[CS-0010.2]** When objects are passed by value, use move semantics instead of copy semantics.
*   **[CS-0010.3]** When objects are passed by reference, use const references instead of pointers.
*   **[CS-0010.4]** When objects are returned, use move semantics instead of copy semantics.
*   **[CS-0010.5]** When objects are shared by multiple threads, use thread-safe containers.
*   **[CS-0010.6]** When objects are shared by multiple objects with potential different lifecycles, use shared_ptr or weak_ptr.
*   **[CS-0010.7]** For object fields not shared across multiple objects, use unique_ptr.
*   **[CS-0010.8]** For objects used in a single scope (method body or lambda body), usage of unique_ptr is optional and discouraged.
*   **[CS-0010.9]** Use zero / one / three rule for constructor parameters where it's possible.
*   **[CS-0010.10]** Use of new or delete keywords is forbidden.
*   **[CS-0010.11]** Use of malloc or free keywords is forbidden.
*   **[CS-0010.12]** Use of calloc or realloc keywords is forbidden.
*   **[CS-0010.13]** Use of strdup or strndup keywords is forbidden.
*   **[CS-0010.14]** When functions from standard libraries relying on unbounded buffers are used, the code shall check for buffer overflow.
*   **[CS-0010.15]** When pointers are used as parameters, the code shall check for null pointers.
*   **[CS-0010.16]** Use of unbounded arrays is forbidden.
*   **[CS-0010.17]** Use of goto keywords is forbidden.
*   **[CS-0010.18]** Dead code is forbidden. (Authorized if commented to explain purpose for external code).
*   **[CS-0010.19]** Use of undefined behavior is forbidden.
*   **[CS-0010.20]** Catch exceptions for throwing functions, or use `std::expected` / `outcome<T, E>` as defined in [CS-0020.48].
*   **[CS-0010.21]** Use of RAII is strongly recommended (mandatory for mutexes).
*   **[CS-0010.22]** Use of RAII is mandatory for file handles.
*   **[CS-0010.23]** Use of RAII is mandatory for socket handles.
*   **[CS-0010.24]** Use of RAII is mandatory for memory mapped files.
*   **[CS-0010.25]** Use of RAII is recommended for memory allocation.
*   **[CS-0010.26]** Avoid wait functions without timeout. Use timed mutexes where possible.
*   **[CS-0010.27]** Avoid sleep functions. Use timed wait functions.
*   **[CS-0010.28]** Avoid global variables.
*   **[CS-0010.29]** Avoid global functions.
*   **[CS-0010.30]** Avoid global constants.
*   **[CS-0010.31]** All declarations shall be explicit.
*   **[CS-0010.32]** All declarations shall be initialized.
*   **[CS-0010.33]** All declarations shall be contained in a namespace.
*   **[CS-0010.34]** Use of the "auto" type is strictly forbidden.
*   **[CS-0010.35]** No function body shall be longer than 200 lines (requires waiver/justification if bypassed).
*   **[CS-0010.36]** No class body shall be longer than 1600 lines.
*   **[CS-0010.37]** Infinite loops forbidden. All loops must have a hard limit on iteration counts and throw an exception on breach.
*   **[CS-0010.38]** Infinite recursion forbidden. Recursive calls must have hard iteration limits and throw on breach.
*   **[CS-0010.39]** Replace literals with constants where possible.
*   **[CS-0010.40]** Replace constants with enums where possible.
*   **[CS-0010.41]** Replace enums with templates where possible.
*   **[CS-0010.42]** Replace templates with concepts where possible.
*   **[CS-0010.43]** Replace concepts with traits where possible.
*   **[CS-0010.44]** Comment block required for every 5 lines of uncommented code.
*   **[CS-0010.45]** Each class, field, method, and member must be documented with Doxygen comments.
*   **[CS-0010.46]** Guarded member modification must use the same timed mutex for all modifications.

*   **[CS-0020.47]** Prefer `std::span` (or `gsl::span`) for non-owning array-like parameters.
*   **[CS-0020.48]** Prefer `std::expected<T, E>` (or `outcome<T, E>`) for error-propagation in logic functions.
*   **[CS-0020.49]** Use `std::move_only_function` for callback interfaces without copy semantics.
*   **[CS-0020.50]** Mark logically `noexcept` functions as `noexcept` and verify in tests.
*   **[CS-0020.51]** Prefer `std::format` (or `fmt::format`) over `printf` or `std::ostringstream`.
*   **[CS-0020.52]** Prefer `std::ranges` algorithms over classic STL algorithms.
*   **[CS-0020.53]** Use named C++ modules if available.
*   **[CS-0020.54]** Prefer `co_await`/coroutines for async IO or background tasks.
*   **[CS-0020.55]** Annotate branch probability using `[[likely]]`/`[[unlikely]]`.
*   **[CS-0020.56]** Annotate resource allocator functions with `[[nodiscard]]`.
*   **[CS-0020.57]** Prefer `constexpr` for compile-time evaluable functions.
*   **[CS-0020.58]** Return containers by value (NRVO/move semantics), never return raw pointers to internal storage.
*   **[CS-0020.59]** Avoid implicit conversions that lose information. Use `explicit` or `static_cast` with comment.
*   **[CS-0020.60]** Annotate interfaces returning resources, errors, or validation results with `[[nodiscard]]`.
*   **[CS-0020.61]** Document failure modes in Doxygen (`@throws`, `@error`).
*   **[CS-0020.62]** Wrap raw pointers immediately in `gsl::not_null` or `std::not_null`.
*   **[CS-0020.63]** Prefer `std::unique_ptr<T[]>` or `std::span<T>` over manual `new[]`/`delete[]`. Use `std::vector` only when size mutability is needed.
*   **[CS-0020.64]** Provide bitwise operator overloads (`|`, `&`, `^`, `~`) for bit-flag `enum class` types.
*   **[CS-0020.65]** Do not expose raw `char*` buffers. Use `std::string_view` for read-only and `std::span<std::byte>` for mutable raw data.
*   **[CS-0020.66]** Enable ASan, UBSan, and TSan in all CI builds.
*   **[CS-0020.67]** All `constexpr` functions must be `noexcept` unless exceptions are part of the intended interface.
*   **[CS-0020.68]** Expose lock-guarded access using `std::unique_lock<std::mutex>` from private helper, never expose raw `std::mutex`.
*   **[CS-0020.69]** Do not use `volatile` for thread synchronization; use `std::atomic` or mutexes.
*   **[CS-0020.70]** Run `clang-tidy` with standard check groups on CI builds.
*   **[CS-0020.71]** Formatter rules: LLVM style overrides (`AllowShortIfStatementsOnASingleLine: false`, `DerivePointerAlignment: false`, `AlignTrailingComments: true`).
*   **[CS-0020.72]** Enforce `cppcheck` warnings as build errors in CI.
*   **[CS-0020.73]** Use pre-commit hooks running `clang-tidy`.
*   **[CS-0020.74]** Include `static_assert` checking struct sizes for binary compatibility.
*   **[CS-0020.75]** Use strict compiler flag warnings (`-Wshadow`, `-Wconversion`, etc.) as errors.
*   **[CS-0020.76]** Enforce `std::span<std::byte>` over raw `char*` templates via `static_assert`.
*   **[CS-0020.77]** Doxygen `@brief`, `@param`, `@return`, and `@throws` on every public API.
*   **[CS-0020.78]** Senior architect approval required for new rules.
*   **[CS-0020.79]** Minimal usage example required for new library feature rules.
*   **[CS-0020.80]** Maintain changelog at `doc/architecture/CS-0020_changelog.md`.

## Code Modification, Deletion, and Review Standards (CS-0030)

*   **[CS-0030.1]** **Feature Contribution Annotation**: All methods/fields must have annotating comments detailing feature contributions.
*   **[CS-0030.2]** **Exposed Interfaces Annotation**: APIs, bindings, network boundaries must use `@exposed` or equivalent.
*   **[CS-0030.3]** **Deletion Justification**: Justify any method/field deletion in commits/descriptions.
*   **[CS-0030.4]** **Preliminary Analysis**: Document impact analysis before method deletions or signature changes.
*   **[CS-0030.5]** **Caller/Callee Analysis**: Document tree analysis to check downstream impact on dependents.
*   **[CS-0030.6]** **Test Coverage Analysis**: Pre-modification coverage review. Add tests if coverage is poor.
*   **[CS-0030.7]** **Test Types**: Functional, worst-case (extreme loads/drops), and stress tests (concurrency/iterations) required.
*   **[CS-0030.8]** **Test Modification Justification**: Changing existing tests requires written explanation/justification.
*   **[CS-0030.9]** **Deletion Detector**: Run `helpers/detect_deletions.py` to identify deleted signatures.
*   **[CS-0030.10]** **Coverage Gatekeepers**: Reject MRs if coverage drops or lacks branch coverage.
*   **[CS-0030.15]** **Two-Phase Review**: Phase 1: Architecture, Impact & Tests. Phase 2: Syntax, Style & Comments.
*   **[CS-0030.16]** **No Unjustified Deletions**: Block MRs with deleted methods/fields lacking justification.
*   **[CS-0030.17]** **Execution Proof**: Verify stress and worst-case test logs.
*   **[CS-0030.18 - CS-0030.23]** **Standardized Review Template**: Feature addressed, impact summary, deletions, testing performed, coverage delta.

## Strategic Constant Management (CS-0040)

*   **[CS-0040.1]** **Prohibition of Magic Numbers**: Direct use of raw literal numbers/strings in logic is forbidden. Use named symbolic constants.
*   **[CS-0040.2]** **Mandatory Symbolic Limitations**: Loop bounds, timeouts, buffer sizes, retries, thresholds must be named constants.
*   **[CS-0040.3]** **Exposure in Headers**: Declare public/shared constants in headers (using `constexpr` or `enum class`). Local constants in anonymous namespaces or as `static constexpr`.
*   **[CS-0040.4]** **Authorized Trivial Literals**: `0`, `1`, and `-1` authorized without waiver for loop init, increments/decrements, null-check, or standard return codes.
*   **[CS-0040.5]** **Explicit Waivers**: Any other literal usage requires a comment explaining why a symbolic constant is detrimental.
*   **[CS-0040.6]** **Naming**: Constants must be `UPPER_SNAKE_CASE`.

## Verifiable Integrity and C++26 Safety (CS-0050)

*   **[CS-0050.1]** **Language-Level Contracts**: Replace manual precondition/postcondition checks with C++26 Contracts (`[[expects: condition]]` and `[[ensures: condition]]`).
*   **[CS-0050.2]** **Explicit Boolean Logic**: Avoid implicit boolean conversions; prefer explicit expressions (e.g. `if (ptr != nullptr)`).
*   **[CS-0050.3]** **Rule of Zero**: Prefer Rule of Zero for business logic; delegate resource management to RAII wrappers.
*   **[CS-0050.4]** **Structured Concurrency**: Prefer `std::execution` (Senders/Receivers) over raw coroutines/threads.
*   **[CS-0050.5]** **Reflective Metadata Automation**: Use C++26 Static Reflection to automate hierarchical registration. Manual mapping is forbidden if reflection applies.
*   **[CS-0050.6]** **Deterministic Stack Guarantees**: Real-time paths must have statically verifiable maximum stack depths. No unbounded recursion/large allocations.
*   **[CS-0050.7]** **Safety-Profile Enforcement**: Life-time and Bounds safety profiles required. Memory/bounds-unsafe code must be annotated/justified.
*   **[CS-0050.8]** **Explicit Bounds Checking**: Use checked accessors (e.g. `at()`) or checked `std::span` in non-performance-critical paths.

## High-Integrity and Deterministic Logic (CS-0060)

*   **[CS-0060.1]** **Semantic [[nodiscard]]**: Obs, factories, error returns must use `[[nodiscard]]`. Cast to `(void)` with comment to ignore.
*   **[CS-0060.2]** **Shadowing Prevention**: Outer variables must not be shadowed by inner ones (locals, params, fields).
*   **[CS-0060.3]** **Total Branching**: All `if-else if` constructs must end with an unconditional `else` block (comment if empty).
*   **[CS-0060.4]** **Literal Pointer Comparison**: Pointer arithmetic (`ptr+n`, `ptr++`) forbidden. Use `std::span` or `std::array`.
*   **[CS-0060.5]** **Exception-Safe Destructors**: Exceptions must not escape destructors; all destructors must be `noexcept`.
*   **[CS-0060.6]** **Union Substitution**: C-style `union` is forbidden. Use `std::variant` or `std::bit_cast`.
*   **[CS-0060.7]** **Modern Randomness**: `std::rand` and `std::srand` are forbidden. Use `<random>` and `std::random_device`.
*   **[CS-0060.8]** **Bit-Field Integrity**: Use `signed int`, `unsigned int`, or `std::byte` for bit-fields. `bool` or other types forbidden.

## Agent Operational Standards and Submission Protocols (CS-0070)

*   **[CS-0070.1]** **Mandatory Clearance**: Strictly forbidden from doing `git commit` or `git push` without explicit user clearance.
*   **[CS-0070.2]** **No Assumed Approval**: Clearance must be requested and granted for every distinct task.
*   **[CS-0070.3]** **Local Build**: Successful local build of affected components is mandatory before requesting clearance.
*   **[CS-0070.4]** **Full Test Pass**: Verify all relevant tests pass.
*   **[CS-0070.5]** **Deletion Analysis**: Run `helpers/detect_deletions.py` to confirm no unintended feature/interface loss. Justify any deletions.
*   **[CS-0070.6]** **Impact Summary**: Summarize changes, why/how, and impact on dependencies.
*   **[CS-0070.7]** **Review Recommendation**: Recommend manual code reviews highlighting complex/high-risk areas.

## Machine Resource Constraints & Thread Limits (CS-0080)

*   **[CS-0080.1]** On this specific machine, system resources are severely limited. Limit running threads to at most 2, including Antigravity threads.
*   **[CS-0080.2]** Prefer single-threaded processes and executions wherever possible.
*   **[CS-0080.3]** All C++ builds must be single-threaded. Do NOT use parallel compilation flags (e.g., do not use `-j` or `-jN` with `make` or `ninja`, unless explicitly specifying `-j1`). Use `make -j1` or simply `make`.
*   **[CS-0080.4]** All Java builds must be single-threaded. Do NOT run Maven or Gradle builds in parallel mode. Do NOT use `-T` or `--threads` with Maven.

