import os
import sys
import argparse
try:
    import clang.cindex
    from clang.cindex import CursorKind
except ImportError:
    print("Error: python3-clang is required. Please install it.", file=sys.stderr)
    sys.exit(1)

def find_nodiscard_violations(file_path, include_paths):
    violations = []
    try:
        index = clang.cindex.Index.create()
        args = ['-std=c++20']
        for p in include_paths:
            args.append(f"-I{p}")
        
        tu = index.parse(file_path, args=args)
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return violations

    for cursor in tu.cursor.walk_preorder():
        try:
            kind = cursor.kind
        except ValueError:
            continue
            
        if kind in [CursorKind.CXX_METHOD, CursorKind.FUNCTION_DECL]:
            if not cursor.spelling :
                continue
            
            # DEBUG
            # print(f"DEBUG: Found {cursor.spelling} in {cursor.location.file}")

            if not cursor.location.file or os.path.abspath(cursor.location.file.name) != os.path.abspath(file_path):
                continue

            if cursor.result_type.spelling == 'void':
                continue

            is_const = cursor.is_const_method()
            name_lower = cursor.spelling.lower()
            is_factory = 'create' in name_lower or 'clone' in name_lower or 'make' in name_lower
            is_error_type = 'Result' in cursor.result_type.spelling or 'expected' in cursor.result_type.spelling
            
            if is_const or is_factory or is_error_type:
                has_nodiscard = False
                for c in cursor.get_children():
                    if c.kind == CursorKind.WARN_UNUSED_RESULT_ATTR:
                        has_nodiscard = True
                        break
                
                if not has_nodiscard:
                    reason = ""
                    if is_const: reason = "Pure observer (const method)"
                    elif is_factory: reason = "Factory function"
                    elif is_error_type: reason = "Error-returning function"
                    
                    violations.append({
                        "file": file_path,
                        "line": cursor.location.line,
                        "function": cursor.spelling,
                        "reason": reason
                    })
    return violations

def main():
    parser = argparse.ArgumentParser(description="AST-based nodiscard checker")
    parser.add_argument("dirs", nargs="+", help="Directories to scan")
    parser.add_argument("--exclude", nargs="+", default=[], help="Directories to exclude")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    include_paths = []
    for root, dirs, files in os.walk(project_root):
        if 'include' in dirs:
            include_paths.append(os.path.join(root, 'include'))

    abs_exclude = [os.path.abspath(d) for d in args.exclude]
    total = 0
    
    for directory in args.dirs:
        if os.path.isfile(directory):
             violations = find_nodiscard_violations(directory, include_paths)
             for v in violations:
                 print(f"[CS-0060.1] {directory}:{v['line']} - Function '{v['function']}' missing [[nodiscard]]. Reason: {v['reason']}.")
                 total += 1
             continue

        for root, dirs, files in os.walk(directory):
            dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
            for file in files:
                if file.endswith(('.hpp', '.h')):
                    path = os.path.join(root, file)
                    violations = find_nodiscard_violations(path, include_paths)
                    for v in violations:
                         print(f"[CS-0060.1] {path}:{v['line']} - Function '{v['function']}' missing [[nodiscard]]. Reason: {v['reason']}.")
                         total += 1
                            
    print(f"\nTotal AST nodiscard violations: {total}")

if __name__ == "__main__":
    main()
