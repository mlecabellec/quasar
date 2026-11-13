import argparse
import glob
import json
import os
import sys

try:
    import clang.cindex
    from clang.cindex import Config, CursorKind
except ImportError:
    print("Error: python3-clang is required. Please install it.", file=sys.stderr)
    sys.exit(1)


def configure_libclang():
    """Locate and register libclang shared library across common Linux paths."""
    if Config.loaded:
        return
    Config.set_compatibility_check(False)
    candidates = [
        "/usr/lib/llvm-19/lib/libclang.so",
        "/usr/lib/llvm-18/lib/libclang.so",
        "/usr/lib/x86_64-linux-gnu/libclang-19.so",
        "/usr/lib/x86_64-linux-gnu/libclang-18.so",
    ]
    candidates.extend(sorted(glob.glob("/usr/lib/llvm-*/lib/libclang*.so*"), reverse=True))
    candidates.extend(sorted(glob.glob("/usr/lib/x86_64-linux-gnu/libclang-*.so*"), reverse=True))
    candidates.extend(sorted(glob.glob("/usr/lib64/libclang*.so*"), reverse=True))
    for cand in candidates:
        if os.path.exists(cand):
            Config.set_library_file(cand)
            return


def find_nodiscard_violations(file_path, include_paths):
    """Parse C++ AST (-std=c++23) and detect missing [[nodiscard]] [CS-0060.1]."""
    violations = []
    try:
        configure_libclang()
        index = clang.cindex.Index.create()
        args = ["-std=c++23", "-x", "c++"]
        for p in include_paths:
            args.append(f"-I{p}")
        tu = index.parse(file_path, args=args)
    except Exception as exc:
        print(f"Error parsing {file_path}: {exc}", file=sys.stderr)
        return violations

    abs_target = os.path.abspath(file_path)
    for cursor in tu.cursor.walk_preorder():
        try:
            kind = cursor.kind
        except ValueError:
            continue

        if kind not in (CursorKind.CXX_METHOD, CursorKind.FUNCTION_DECL, CursorKind.FUNCTION_TEMPLATE):
            continue
        if not cursor.spelling:
            continue
        if not cursor.location.file or os.path.abspath(cursor.location.file.name) != abs_target:
            continue

        res_type = cursor.result_type.spelling
        if res_type == "void":
            continue

        is_const = (kind == CursorKind.CXX_METHOD) and cursor.is_const_method()
        name_lower = cursor.spelling.lower()
        is_factory = (
            "create" in name_lower
            or "clone" in name_lower
            or name_lower.startswith("make")
        )
        is_error_type = "Result" in res_type or "expected" in res_type

        if is_const or is_factory or is_error_type:
            has_nodiscard = any(
                c.kind == CursorKind.WARN_UNUSED_RESULT_ATTR for c in cursor.get_children()
            )
            if not has_nodiscard:
                reason = ""
                if is_const:
                    reason = "Pure observer (const method)"
                elif is_factory:
                    reason = "Factory function"
                elif is_error_type:
                    reason = "Error-returning function"
                violations.append(
                    {
                        "file": file_path,
                        "line": cursor.location.line,
                        "function": cursor.spelling,
                        "reason": reason,
                    }
                )
    return violations


def main():
    parser = argparse.ArgumentParser(description="AST-based [[nodiscard]] checker [CS-0060.1]")
    parser.add_argument("dirs", nargs="+", help="Files or directories to scan")
    parser.add_argument("--exclude", nargs="+", default=[], help="Directories to exclude")
    parser.add_argument("--format", choices=["text", "json"], default="text")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if violations found")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    include_paths = []
    for root, dirs, _ in os.walk(project_root):
        if any(skip in root for skip in ("/build", "/.cache", "/.git", "/tmp")):
            continue
        if "include" in dirs:
            include_paths.append(os.path.join(root, "include"))

    abs_exclude = [os.path.abspath(d) for d in args.exclude]
    all_violations = []

    for directory in args.dirs:
        if os.path.isfile(directory):
            all_violations.extend(find_nodiscard_violations(directory, include_paths))
            continue

        for root, dirs, files in os.walk(directory):
            dirs[:] = [
                d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude
            ]
            for file in files:
                if file.endswith((".hpp", ".h")):
                    path = os.path.join(root, file)
                    all_violations.extend(find_nodiscard_violations(path, include_paths))

    if args.format == "json":
        print(json.dumps(all_violations, indent=2))
    else:
        for v in all_violations:
            print(
                f"[CS-0060.1] {v['file']}:{v['line']} - "
                f"Function '{v['function']}' missing [[nodiscard]]. Reason: {v['reason']}."
            )
        print(f"\nTotal AST nodiscard violations: {len(all_violations)}")

    if args.strict and all_violations:
        sys.exit(1)


if __name__ == "__main__":
    main()
