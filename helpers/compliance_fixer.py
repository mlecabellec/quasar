import os
import re
import argparse

def fix_file(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    new_lines = []
    changed = False
    
    for i, line in enumerate(lines):
        new_line = line
        
        # 1. auto removal (CS-0010.34)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*std::make_shared<([^>]+)>', r'std::shared_ptr<\2> \1 = std::make_shared<\2>', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*std::make_unique<([^>]+)>', r'std::unique_ptr<\2> \1 = std::make_unique<\2>', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*std::dynamic_pointer_cast<([^>]+)>', r'std::shared_ptr<\2> \1 = std::dynamic_pointer_cast<\2>', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*(\d+)\b', r'int \1 = \2', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*true\b', r'bool \1 = true', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*false\b', r'bool \1 = false', new_line)

        # 2. [[nodiscard]] injection (CS-0060.1) - HEADERS ONLY
        if file_path.endswith(('.h', '.hpp')):
            if not line.strip().startswith('[[nodiscard]]') and 'void' not in line and 'operator' not in line:
                # Better regex for templates and functions
                match = re.search(r'^\s*(?:template<.*?>\s*)?(?!return|if|for|while|switch|else|static_assert|using|typedef|class|struct)(?:[\w:<>]+\s+)+(\w+)\s*\(', line)
                if match:
                    func_name = match.group(1)
                    if func_name not in ['main', 'TEST', 'TEST_F'] and not func_name.startswith('~'):
                         if '(' in line and ')' in line:
                             # Check if it's already on previous line
                             context = line
                             if i > 0: context += lines[i-1]
                             if '[[nodiscard]]' not in context:
                                 new_line = line.replace(match.group(0), '[[nodiscard]] ' + match.group(0).strip())

        if new_line != line:
            changed = True
        new_lines.append(new_line)
    
    if changed:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+")
    args = parser.parse_args()
    for f in args.files:
        if os.path.isfile(f):
            if fix_file(f):
                print(f"Refactored {f}")

if __name__ == "__main__":
    main()
