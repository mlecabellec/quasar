# Quasar Helper Scripts & Compliance Tools

This directory contains specialized tools for enforcing architectural constraints, verifying code integrity, validating Doxygen documentation, and automating the compliance audit process for the Quasar project.

## Master Orchestrator

### `verify_all.py`
The primary entry point for compliance verification.
- **Usage**: `python3 helpers/verify_all.py [--dirs <paths>] [--exclude <paths>] [--skip-doxygen] [--strict]`
- **Features**:
  - Orchestrates all sub-checkers (`compliance_suite.py`, `check_annotations.py`, `check_metrics.py`, `ast_nodiscard_checker.py`, and Doxygen validation).
  - Automatically excludes `cmake-projects/third-party` by default.
  - Supports targeted verification on a specific sub-project or file.

---

## Specialized Checkers

### `compliance_suite.py`
Enforces core C++ standards, keyword prohibitions, branching, and comment hygiene.
- **Rules Covered**: [CS-0010.10-13, 17, 34, 37, 44], [CS-0020.69], [CS-0040.6], [CS-0050.2], [CS-0060.3, 6, 7, 8], [CS-0090.4].
- **Key Checks**:
  - Forbidden keywords (`new`, `delete`, `malloc`, `free`, `auto`, `goto`, `union`, `volatile`, `std::rand`, `std::srand`) after stripping comments and string literals.
  - Naming conventions (constants MUST be `UPPER_SNAKE_CASE`).
  - Maximum consecutive uncommented code lines (`<= 5` per `[CS-0010.44]`).
  - Unbounded loops (`while (true)` / `for (;;)` `[CS-0010.37]`), `bool` bit-fields (`[CS-0060.8]`), missing `else` on `else if` chains (`[CS-0060.3]`), and implicit boolean conversion (`[CS-0050.2]`).
  - Comment hygiene: non-ASCII characters in comments, nested `/*`, and commented-out C++ statements (`[CS-0090.4]`).
- **Usage**: `python3 helpers/compliance_suite.py --dirs <src_dirs> --exclude <exclude_dirs> [--format json] [--strict]`

### `ast_nodiscard_checker.py`
Semantically verifies the application of `[[nodiscard]]` (`[CS-0060.1]`) using C++23 AST parsing.
- **Rules Covered**: [CS-0010.1], [CS-0060.1].
- **Key Checks**:
  - Uses `python3-clang` (`-std=c++23`) with automatic `libclang.so` discovery.
  - Flags pure observer methods (`const`), factory functions (`create`, `clone`, `make*`), and error-returning functions (`Result`, `expected`) lacking `[[nodiscard]]`.
- **Usage**: `python3 helpers/ast_nodiscard_checker.py <src_dirs> --exclude <exclude_dirs> [--format json] [--strict]`

### `check_annotations.py`
Verifies formal Doxygen documentation, traceability citations, parameter/return contracts, error containment, and control-structure/in-body comments.
- **Rules Covered**: [CS-0010.45], [CS-0020.61, 77], [CS-0030.1, 2], [CS-0090.1 - CS-0090.9].
- **Key Checks**:
  - Uses `libclang` AST (`-std=c++23`) across classes, structs, enums, enumerators, fields, constants, methods, constructors, destructors, and functions for Doxygen `@brief` (`@details` encouraged/mandatory on non-trivial APIs) (`[CS-0090.1]`).
  - Enforces reference citations (`@task`, `@requirement`, `@constraint`, `@rule`, `@feature`, `@exposed`) and `@compliance` proof (`[CS-0090.2]`).
  - Enforces directional `@param[in|out|in,out]` contracts documenting `Justification`, `Usage`, `Consumed`/`Mutation`, and `Prerequisites` (`[CS-0090.3]`).
  - Enforces full `@return`/`@retval` contracts (`Justification`, `Usage`, `Consumed`/`Ownership`, `Prerequisites`, `Limitations`, `Lifecycle`, `Modification/Deletion`, `Specific Values`, `Error Behavior`) (`[CS-0090.7]`).
  - Enforces error-category behavior (`@throws`/`@error`/`@retval`) and `@containment` approach & limitations (`[CS-0090.8]`).
  - Emits non-blocking advisories encouraging formalized `@pre`/`@post` predicates and `@details` (`[CS-0090.1, CS-0090.9]`).
  - Verifies control-structure comments on `if`, `else if`, `else`, `for`, `while`, `do`, `switch`, `try`, `catch` (`[CS-0090.6]`).
- **Usage**: `python3 helpers/check_annotations.py <src_dirs> --exclude <exclude_dirs> [--format json] [--strict]`

### `check_metrics.py`
Monitors structural size limits and in-body comment coverage using scope-stack brace tracking.
- **Rules Covered**: [CS-0010.35, 36], [CS-0090.5].
- **Key Checks**:
  - Function length (`<= 200` lines) across arbitrary namespace nesting.
  - Class/struct length (`<= 1600` lines).
  - Mandatory in-body justification comments on functions exceeding 5 code lines (`[CS-0090.5]`).
- **Usage**: `python3 helpers/check_metrics.py <src_dirs> --exclude <exclude_dirs> [--format json] [--strict]`

### `detect_deletions.py`
Analyzes Git history to prevent unjustified deletions (`[CS-0030.3, 9]`, `[CS-0070.5]`).

---

## Maintenance & Documentation Scripts

- `build-docs.sh`: Compiles `doc/docs/` into `doc/site/` HTML, PDF, and man pages.
- `check-links.py`: Verifies local links across `doc/site/`.
- `checkConstraintsCompliance.sh`: Scans constraints under `doc/docs/architecture/CS*.md`.
