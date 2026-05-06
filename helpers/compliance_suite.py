import os
import re
import sys
import argparse
import json
from typing import List, Dict, Any

class ComplianceViolation:
    def __init__(self, rule_id: str, file_path: str, line_number: int, message: str, severity: str = "error"):
        self.rule_id = rule_id
        self.file_path = file_path
        self.line_number = line_number
        self.message = message
        self.severity = severity

    def to_dict(self):
        return {
            "rule_id": self.rule_id,
            "file": self.file_path,
            "line": self.line_number,
            "message": self.message,
            "severity": self.severity
        }

    def __str__(self):
        return f"[{self.rule_id}] {self.file_path}:{self.line_number} - {self.message}"

class ComplianceChecker:
    def __init__(self, root_dir: str):
        self.root_dir = root_dir
        self.violations: List[ComplianceViolation] = []

    def report(self, violation: ComplianceViolation):
        self.violations.append(violation)

    def scan_files(self, include_dirs: List[str], exclude_dirs: List[str] = None):
        if exclude_dirs is None:
            exclude_dirs = []
        
        abs_exclude = [os.path.abspath(os.path.join(self.root_dir, d)) for d in exclude_dirs]

        for directory in include_dirs:
            full_path = os.path.join(self.root_dir, directory)
            if not os.path.exists(full_path):
                continue
            for root, dirs, files in os.walk(full_path):
                dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
                for file in files:
                    if file.endswith(('.cpp', '.h', '.hpp', '.c')):
                        self.check_file(os.path.join(root, file))

    def check_file(self, file_path: str):
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return

        rel_path = os.path.relpath(file_path, self.root_dir)
        
        # Strip multiline comments for keyword search
        content_no_ml = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        lines_no_ml = content_no_ml.splitlines()
        
        full_lines = content.splitlines()

        self.check_forbidden_keywords(rel_path, full_lines)
        self.check_naming_conventions(rel_path, full_lines)
        self.check_metrics(rel_path, full_lines)
        self.check_nodiscard(rel_path, full_lines)
        self.check_explicit_bool(rel_path, full_lines)

    def _is_in_comment(self, line: str, pos: int) -> bool:
        # Very simple check for // comments
        comment_idx = line.find('//')
        if comment_idx != -1 and comment_idx < pos:
            return True
        return False

    def check_forbidden_keywords(self, file_path: str, lines: List[str]):
        forbidden = {
            r'\bnew\b': ('CS-0010.10', 'Use of "new" is forbidden.'),
            r'\bdelete\b': ('CS-0010.10', 'Use of "delete" is forbidden.'),
            r'\bmalloc\b': ('CS-0010.11', 'Use of "malloc" is forbidden.'),
            r'\bfree\b': ('CS-0010.11', 'Use of "free" is forbidden.'),
            r'\bcalloc\b': ('CS-0010.12', 'Use of "calloc" is forbidden.'),
            r'\brealloc\b': ('CS-0010.12', 'Use of "realloc" is forbidden.'),
            r'\bstrdup\b': ('CS-0010.13', 'Use of "strdup" is forbidden.'),
            r'\bstrndup\b': ('CS-0010.13', 'Use of "strndup" is forbidden.'),
            r'\bgoto\b': ('CS-0010.17', 'Use of "goto" is forbidden.'),
            r'\bauto\b': ('CS-0010.34', 'Use of "auto" is strictly forbidden.'),
            r'\bunion\b': ('CS-0060.6', 'Use of C-style "union" is forbidden.'),
            r'\bstd::rand\b': ('CS-0060.7', 'Use of "std::rand" is forbidden.'),
            r'\bstd::srand\b': ('CS-0060.7', 'Use of "std::srand" is forbidden.'),
            r'\bvolatile\b': ('CS-0020.69', 'Use of "volatile" is forbidden.')
        }

        in_ml_comment = False
        for i, line in enumerate(lines):
            # ML comment tracking
            clean_line = line
            if '/*' in line and '*/' in line:
                clean_line = re.sub(r'/\*.*?\*/', '', line)
            elif '/*' in line:
                in_ml_comment = True
                clean_line = line.split('/*')[0]
            elif '*/' in line:
                in_ml_comment = False
                clean_line = line.split('*/')[-1]
            elif in_ml_comment:
                continue

            # SL comment stripping
            clean_line = clean_line.split('//')[0]
            
            if clean_line.strip().startswith('#include'): continue

            for pattern, (rule_id, msg) in forbidden.items():
                if re.search(pattern, clean_line):
                    self.report(ComplianceViolation(rule_id, file_path, i + 1, msg))

    def check_naming_conventions(self, file_path: str, lines: List[str]):
        const_pattern = re.compile(r'\b(?:static\s+)?(?:constexpr|const)\s+[\w:<>]+\s+([a-zA-Z]\w+)\s*=')
        for i, line in enumerate(lines):
            clean_line = line.split('//')[0]
            match = const_pattern.search(clean_line)
            if match:
                const_name = match.group(1)
                if not (const_name.isupper() and all(c.isalnum() or c == '_' for c in const_name)):
                    if const_name not in ['it', 'i', 'j', 'k', 'value', 'data']: 
                         self.report(ComplianceViolation('CS-0040.6', file_path, i + 1, f"Constant '{const_name}' should be UPPER_SNAKE_CASE."))

    def check_metrics(self, file_path: str, lines: List[str]):
        code_lines_without_comment = 0
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped == "" or stripped == "{" or stripped == "}" or stripped.startswith(('/', '*')):
                code_lines_without_comment = 0
            else:
                code_lines_without_comment += 1
                if code_lines_without_comment > 10: # Increased threshold for less noise
                    self.report(ComplianceViolation('CS-0010.44', file_path, i + 1, "Large code block without comments (potential maintenance risk).", severity="warning"))
                    code_lines_without_comment = 0

    def check_nodiscard(self, file_path: str, lines: List[str]):
        # Refined regex for function declarations/definitions
        func_pattern = re.compile(r'^\s*(?!return|if|for|while|switch|else|static_assert)(?:[\w:<>]+\s+)+(\w+)\s*\(')
        for i, line in enumerate(lines):
            if 'void' in line or '[[nodiscard]]' in line: continue
            if 'main' in line or 'operator' in line: continue
            
            match = func_pattern.search(line)
            if match:
                func_name = match.group(1)
                # Check previous 2 lines for [[nodiscard]]
                context = line
                if i > 0: context += lines[i-1]
                if i > 1: context += lines[i-2]
                
                if '[[nodiscard]]' not in context:
                    # Filter out constructors/destructors heuristic
                    if func_name.startswith('~'): continue
                    # If line has no semicolon and no brace, it might be a multi-line decl, skip for now to reduce noise
                    if ';' not in line and '{' not in line: continue
                    
                    self.report(ComplianceViolation('CS-0060.1', file_path, i + 1, f"Function '{func_name}' might be missing [[nodiscard]].", severity="warning"))

    def check_explicit_bool(self, file_path: str, lines: List[str]):
        if_pattern = re.compile(r'\b(?:if|while)\s*\(([^;=!<>]+)\)')
        for i, line in enumerate(lines):
            clean_line = line.split('//')[0]
            match = if_pattern.search(clean_line)
            if match:
                expr = match.group(1).strip()
                # Very conservative check
                if re.match(r'^[a-zA-Z_]\w*$', expr) or re.match(r'^![a-zA-Z_]\w*$', expr):
                    if expr not in ['true', 'false']:
                        self.report(ComplianceViolation('CS-0050.2', file_path, i + 1, f"Implicit boolean conversion in '({expr})'.", severity="warning"))

def main():
    parser = argparse.ArgumentParser(description="Quasar Compliance Suite")
    parser.add_argument("--root", default=".", help="Project root directory")
    parser.add_argument("--dirs", nargs="+", default=["cmake-projects"], help="Directories to scan")
    parser.add_argument("--exclude", nargs="+", default=[], help="Directories to exclude")
    parser.add_argument("--format", choices=["text", "json"], default="text", help="Output format")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    
    checker = ComplianceChecker(project_root)
    checker.scan_files(args.dirs, args.exclude)

    if args.format == "json":
        print(json.dumps([v.to_dict() for v in checker.violations], indent=2))
    else:
        for v in checker.violations:
            print(v)
        print(f"\nTotal violations: {len(checker.violations)}")

if __name__ == "__main__":
    main()
