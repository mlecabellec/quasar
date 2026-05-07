import os
import re
import argparse
import sys

def check_file(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    violations = []
    
    # Improved function detection
    # Must look like: type name(args)
    func_pattern = re.compile(r'^\s*(?!return|if|for|while|switch|else|static_assert)(?:[\w:<>]+\s+)+(\w+)\s*\(')
    
    for i, line in enumerate(lines):
        # Only check public headers or implementation files
        # Heuristic: skip if it's clearly a private member or local var
        if re.search(r'^\s*([a-zA-Z_]\w*)\s+\w+\s*;', line): continue # variable
        
        match = func_pattern.search(line)
        if match:
            func_name = match.group(1)
            # Skip common false positives
            if func_name in ['TEST', 'TEST_F', 'SECTION', 'REQUIRE']: continue
            if func_name.startswith('~'): continue # Destructor
            
            # Check previous 3 lines for Doxygen start
            has_doxygen = False
            for j in range(max(0, i-3), i):
                if '/**' in lines[j] or '///' in lines[j]:
                    has_doxygen = True
                    break
            
            if not has_doxygen:
                # Only report if it's a declaration or definition start
                if ';' in line or '{' in line:
                    violations.append((i+1, f"Function '{func_name}' missing Doxygen documentation [CS-0010.45]"))

    return violations

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dirs", nargs="+")
    parser.add_argument("--exclude", nargs="+", default=[])
    args = parser.parse_args()
    
    abs_exclude = [os.path.abspath(d) for d in args.exclude]
    total_violations = 0
    for directory in args.dirs:
        for root, dirs, files in os.walk(directory):
            dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
            for file in files:
                if file.endswith(('.h', '.hpp', '.cpp')):
                    file_path = os.path.join(root, file)
                    v = check_file(file_path)
                    for line, msg in v:
                        print(f"{file_path}:{line} - {msg}")
                        total_violations += 1
    
    print(f"\nTotal annotation violations: {total_violations}")

if __name__ == "__main__":
    main()
