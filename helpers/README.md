# 🛠️ Quasar Helper Scripts & Compliance Tools

This directory contains specialized tools for enforcing architectural constraints, verifying code integrity, and automating the compliance audit process for the Quasar project.

## 🚀 Master Orchestrator

### `verify_all.py`
The primary entry point for compliance verification.
- **Usage**: `python3 helpers/verify_all.py`
- **Features**: 
  - Orchestrates all sub-checkers.
  - Automatically excludes `third-party` directories.
  - Provides a consolidated report of all violations.

---

## 🔍 Specialized Checkers

### `compliance_suite.py`
Enforces core C++ standards and keyword prohibitions.
- **Rules Covered**: [CS-0010.10-13, 17, 34], [CS-0020.69], [CS-0040.6], [CS-0050.2], [CS-0060.6, 7].
- **Key Checks**:
  - Forbidden keywords (`new`, `delete`, `malloc`, `auto`, `goto`, `union`, `volatile`).
  - Naming conventions (Constants MUST be `UPPER_SNAKE_CASE`).
  - Implicit boolean conversion in control flow.
- **Usage**: `python3 helpers/compliance_suite.py --dirs <src_dirs> --exclude <exclude_dirs>`

### `ast_nodiscard_checker.py`
Semantically verifies the application of `[[nodiscard]]` (Rule CS-0060.1).
- **Rules Covered**: [CS-0060.1].
- **Key Checks**:
  - Uses `python3-clang` to parse the C++ AST.
  - Flags pure observer methods (const methods) lacking `[[nodiscard]]`.
  - Flags factory functions (e.g., `create`, `clone`) lacking `[[nodiscard]]`.
  - Flags error-returning functions (e.g., `Result`) lacking `[[nodiscard]]`.
- **Usage**: `python3 helpers/ast_nodiscard_checker.py <src_dirs> --exclude <exclude_dirs>`

### `check_annotations.py`
Verifies documentation and traceability metadata.
- **Rules Covered**: [CS-0010.45], [CS-0030.1, 2].
- **Key Checks**:
  - Missing Doxygen blocks for classes and methods.
  - Presence of mandatory `@feature` or `@exposed` tags on interface methods.
- **Usage**: `python3 helpers/check_annotations.py <src_dirs> --exclude <exclude_dirs>`

### `check_metrics.py`
Monitors code complexity and structural limits.
- **Rules Covered**: [CS-0010.35, 36].
- **Key Checks**:
  - Function length (Max 200 lines).
  - Class length (Max 1600 lines).
- **Usage**: `python3 helpers/check_metrics.py <src_dirs> --exclude <exclude_dirs>`

### `detect_deletions.py`
Analyzes Git history to prevent unjustified deletions.
- **Rules Covered**: [CS-0030.3, 9].
- **Key Checks**: Detects removed function signatures and checks for justification in commit messages.

---

## 🛠️ Maintenance Scripts

### `checkConstraintsCompliance.sh`
The legacy agent-driven orchestrator. It uses an LLM agent to perform deep semantic analysis of code against constraint definitions.

### `updateDocAndCode.sh`
A utility script for synchronizing documentation updates across the repository.

---

## 📈 Integration
These scripts should be executed:
1. **Locally**: As part of the development workflow before pushing.
2. **Pre-commit**: Integrated via `.git/hooks/pre-commit`.
3. **CI/CD**: Executed on every Pull Request to ensure zero regressions in architectural integrity.
