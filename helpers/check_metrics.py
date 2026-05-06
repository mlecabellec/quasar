import os
import re
import argparse

def analyze_file(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    violations = []
    current_class = None
    class_start_line = 0
    current_function = None
    func_start_line = 0
    brace_level = 0
    
    for i, line in enumerate(lines):
        class_match = re.search(r'\bclass\s+(\w+)', line)
        if class_match and brace_level == 0:
            current_class = class_match.group(1)
            class_start_line = i + 1
            
        func_match = re.search(r'^\s*(?:[\w:<>]+\s+)+(\w+)\s*\(.*?\)\s*\{', line)
        if func_match:
            current_function = func_match.group(1)
            func_start_line = i + 1

        if '{' in line: brace_level += line.count('{')
        if '}' in line: 
            brace_level -= line.count('}')
            if brace_level == 0 and current_class:
                length = (i + 1) - class_start_line
                if length > 1600:
                    violations.append((class_start_line, f"Class '{current_class}' too long ({length} lines > 1600) [CS-0010.36]"))
                current_class = None
            
            if brace_level <= 1 and current_function:
                length = (i + 1) - func_start_line
                if length > 200:
                    violations.append((func_start_line, f"Function '{current_function}' too long ({length} lines > 200) [CS-0010.35]"))
                current_function = None

    return violations

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dirs", nargs="+")
    parser.add_argument("--exclude", nargs="+", default=[])
    args = parser.parse_args()
    
    abs_exclude = [os.path.abspath(d) for d in args.exclude]
    for d in args.dirs:
        for root, dirs, files in os.walk(d):
            dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
            for f in files:
                if f.endswith(('.cpp', '.h', '.hpp')):
                    p = os.path.join(root, f)
                    v = analyze_file(p)
                    for line, msg in v:
                        print(f"{p}:{line} - {msg}")

if __name__ == "__main__":
    main()
