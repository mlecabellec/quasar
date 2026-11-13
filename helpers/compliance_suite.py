import argparse
import json
import os
import re
import sys
from typing import Dict, List, Tuple


class ComplianceViolation:
    def __init__(self, rule_id: str, file_path: str, line_number: int, message: str, severity: str = "error"):
        self.rule_id = rule_id
        self.file_path = file_path
        self.line_number = line_number
        self.message = message
        self.severity = severity

    def to_dict(self) -> Dict[str, object]:
        return {
            "rule_id": self.rule_id,
            "file": self.file_path,
            "line": self.line_number,
            "message": self.message,
            "severity": self.severity,
        }

    def __str__(self) -> str:
        return f"[{self.rule_id}] {self.file_path}:{self.line_number} - {self.message}"


class ComplianceChecker:
    def __init__(self, root_dir: str, max_uncommented: int = 5):
        self.root_dir = root_dir
        self.max_uncommented = max_uncommented
        self.violations: List[ComplianceViolation] = []

    def report(self, violation: ComplianceViolation):
        self.violations.append(violation)

    def scan_files(self, include_dirs: List[str], exclude_dirs: List[str] = None):
        if exclude_dirs is None:
            exclude_dirs = []
        abs_exclude = [os.path.abspath(os.path.join(self.root_dir, d)) for d in exclude_dirs]

        for directory in include_dirs:
            full_path = os.path.join(self.root_dir, directory)
            if os.path.isfile(full_path):
                self.check_file(full_path)
                continue
            if not os.path.exists(full_path):
                continue
            for root, dirs, files in os.walk(full_path):
                dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
                for file in files:
                    if file.endswith((".cpp", ".h", ".hpp", ".c")):
                        self.check_file(os.path.join(root, file))

    @staticmethod
    def strip_comments_and_strings(lines: List[str]) -> Tuple[List[str], List[str], List[Tuple[int, str]]]:
        """Return (code_only_lines, raw_comment_lines, nested_block_comment_hits)."""
        code_lines: List[str] = []
        comment_lines: List[str] = []
        nested_hits: List[Tuple[int, str]] = []
        in_block = False

        for idx, line in enumerate(lines):
            code_out: List[str] = []
            comm_out: List[str] = []
            i = 0
            n = len(line)
            in_str = False
            str_char = ""

            # Preserve #include headers verbatim for line structure while skipping literal scanning
            stripped_l = line.lstrip()
            if not in_block and stripped_l.startswith("#include"):
                code_lines.append(line)
                comment_lines.append("")
                continue

            while i < n:
                ch = line[i]
                nxt = line[i + 1] if i + 1 < n else ""
                if in_block:
                    if ch == "/" and nxt == "*":
                        nested_hits.append((idx + 1, "Nested '/*' detected inside block comment [CS-0090.4]."))
                        comm_out.append("/*")
                        i += 2
                        continue
                    if ch == "*" and nxt == "/":
                        in_block = False
                        i += 2
                        continue
                    comm_out.append(ch)
                    i += 1
                    continue

                if in_str:
                    if ch == "\\":
                        i += 2
                        continue
                    if ch == str_char:
                        in_str = False
                    i += 1
                    continue

                if ch == "/" and nxt == "*":
                    in_block = True
                    i += 2
                    continue
                if ch == "/" and nxt == "/":
                    comm_out.append(line[i:])
                    break
                if ch in ('"', "'"):
                    in_str = True
                    str_char = ch
                    code_out.append(" ")
                    i += 1
                    continue

                code_out.append(ch)
                i += 1

            code_lines.append("".join(code_out))
            comment_lines.append("".join(comm_out))

        return code_lines, comment_lines, nested_hits

    def check_file(self, file_path: str):
        try:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {file_path}: {e}", file=sys.stderr)
            return

        rel_path = os.path.relpath(file_path, self.root_dir)
        lines = content.splitlines()
        code_lines, comment_lines, nested_hits = self.strip_comments_and_strings(lines)

        for ln, msg in nested_hits:
            self.report(ComplianceViolation("CS-0090.4", rel_path, ln, msg))

        self.check_forbidden_keywords(rel_path, code_lines)
        self.check_naming_conventions(rel_path, code_lines)
        self.check_metrics(rel_path, lines)
        self.check_explicit_bool(rel_path, code_lines)
        self.check_safety_constructs(rel_path, code_lines)
        self.check_total_branching(rel_path, code_lines)
        self.check_comment_hygiene(rel_path, lines, comment_lines)

    def check_forbidden_keywords(self, file_path: str, code_lines: List[str]):
        forbidden = {
            r"\bnew\b": ("CS-0010.10", 'Use of "new" is forbidden.'),
            r"\bdelete\b": ("CS-0010.10", 'Use of "delete" is forbidden.'),
            r"\bmalloc\b": ("CS-0010.11", 'Use of "malloc" is forbidden.'),
            r"\bfree\b": ("CS-0010.11", 'Use of "free" is forbidden.'),
            r"\bcalloc\b": ("CS-0010.12", 'Use of "calloc" is forbidden.'),
            r"\brealloc\b": ("CS-0010.12", 'Use of "realloc" is forbidden.'),
            r"\bstrdup\b": ("CS-0010.13", 'Use of "strdup" is forbidden.'),
            r"\bstrndup\b": ("CS-0010.13", 'Use of "strndup" is forbidden.'),
            r"\bgoto\b": ("CS-0010.17", 'Use of "goto" is forbidden.'),
            r"\bauto\b": ("CS-0010.34", 'Use of "auto" is strictly forbidden.'),
            r"\bunion\b": ("CS-0060.6", 'Use of C-style "union" is forbidden.'),
            r"\bstd::rand\b": ("CS-0060.7", 'Use of "std::rand" is forbidden.'),
            r"\bstd::srand\b": ("CS-0060.7", 'Use of "std::srand" is forbidden.'),
            r"\bvolatile\b": ("CS-0020.69", 'Use of "volatile" is forbidden.'),
        }

        for i, clean_line in enumerate(code_lines):
            if clean_line.strip().startswith("#include"):
                continue
            for pattern, (rule_id, msg) in forbidden.items():
                if re.search(pattern, clean_line):
                    self.report(ComplianceViolation(rule_id, file_path, i + 1, msg))

    def check_naming_conventions(self, file_path: str, code_lines: List[str]):
        const_pattern = re.compile(r"\b(?:static\s+)?(?:constexpr|const)\s+[\w:<>]+\s+([a-zA-Z]\w+)\s*=")
        for i, clean_line in enumerate(code_lines):
            # Ignore references and function parameters
            if "&" in clean_line.split("=")[0] or "(" in clean_line.split("=")[0]:
                continue
            match = const_pattern.search(clean_line)
            if match:
                const_name = match.group(1)
                if not (const_name.isupper() and all(c.isalnum() or c == "_" for c in const_name)):
                    if const_name not in ("it", "i", "j", "k", "value", "data"):
                        self.report(
                            ComplianceViolation(
                                "CS-0040.6",
                                file_path,
                                i + 1,
                                f"Constant '{const_name}' should be UPPER_SNAKE_CASE.",
                            )
                        )

    def check_metrics(self, file_path: str, lines: List[str]):
        code_lines_without_comment = 0
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped == "" or stripped in ("{", "}", "};") or stripped.startswith(("/", "*")):
                code_lines_without_comment = 0
            else:
                code_lines_without_comment += 1
                if code_lines_without_comment > self.max_uncommented:
                    self.report(
                        ComplianceViolation(
                            "CS-0010.44",
                            file_path,
                            i + 1,
                            f"Large code block without comments (>{self.max_uncommented} lines).",
                            severity="warning",
                        )
                    )
                    code_lines_without_comment = 0

    def check_explicit_bool(self, file_path: str, code_lines: List[str]):
        if_pattern = re.compile(r"\b(?:if|while)\s*\(([^;=!<>]+)\)")
        for i, clean_line in enumerate(code_lines):
            match = if_pattern.search(clean_line)
            if match:
                expr = match.group(1).strip()
                if re.match(r"^[a-zA-Z_]\w*$", expr) or re.match(r"^![a-zA-Z_]\w*$", expr):
                    if expr not in ("true", "false"):
                        self.report(
                            ComplianceViolation(
                                "CS-0050.2",
                                file_path,
                                i + 1,
                                f"Implicit boolean conversion in '({expr})'.",
                                severity="warning",
                            )
                        )

    def check_safety_constructs(self, file_path: str, code_lines: List[str]):
        bool_bitfield = re.compile(r"\bbool\s+\w+\s*:\s*\d+\s*;")
        inf_while = re.compile(r"\bwhile\s*\(\s*(?:true|1)\s*\)")
        inf_for = re.compile(r"\bfor\s*\(\s*;\s*;\s*\)")
        for i, clean_line in enumerate(code_lines):
            if bool_bitfield.search(clean_line):
                self.report(
                    ComplianceViolation(
                        "CS-0060.8",
                        file_path,
                        i + 1,
                        "Bit-field of type 'bool' is forbidden; use signed/unsigned int or std::byte.",
                    )
                )
            if inf_while.search(clean_line) or inf_for.search(clean_line):
                self.report(
                    ComplianceViolation(
                        "CS-0010.37",
                        file_path,
                        i + 1,
                        "Unbounded infinite loop forbidden without hard iteration guard.",
                    )
                )

    def check_total_branching(self, file_path: str, code_lines: List[str]):
        """Verify every if - else if chain closes with an unconditional else [CS-0060.3]."""
        # Track pending 'else if' at current brace depth
        depth = 0
        pending_else_if: Dict[int, int] = {}
        for i, clean_line in enumerate(code_lines):
            s = clean_line.strip()
            if not s or s.startswith("#"):
                continue

            if re.search(r"\belse\s+if\s*\(", s):
                pending_else_if[depth] = i + 1

            for ch in s:
                if ch == "{":
                    depth += 1
                elif ch == "}":
                    depth = max(0, depth - 1)
                    if depth in pending_else_if:
                        # Look ahead on same line for chained else / else if
                        after_brace = s.split("}", 1)[-1].strip()
                        if after_brace.startswith("else if"):
                            pending_else_if[depth] = i + 1
                        elif after_brace.startswith("else"):
                            pending_else_if.pop(depth, None)
                        else:
                            # Wait for next non-empty line to check if 'else' follows
                            j = i + 1
                            next_token = ""
                            while j < len(code_lines):
                                nxt = code_lines[j].strip()
                                if nxt:
                                    next_token = nxt
                                    break
                                j += 1
                            if re.match(r"^else\b", next_token) and not re.match(r"^else\s+if\b", next_token):
                                pending_else_if.pop(depth, None)
                            elif re.match(r"^else\s+if\b", next_token):
                                pass
                            else:
                                ln = pending_else_if.pop(depth)
                                self.report(
                                    ComplianceViolation(
                                        "CS-0060.3",
                                        file_path,
                                        ln,
                                        "'if-else if' chain missing terminating unconditional 'else' branch.",
                                        severity="warning",
                                    )
                                )

    def check_comment_hygiene(self, file_path: str, raw_lines: List[str], comment_lines: List[str]):
        """Enforce ASCII-only comments and detect commented-out C++ code [CS-0090.4]."""
        code_in_comment = re.compile(
            r"^(?:return\b|if\s*\(|for\s*\(|while\s*\(|std::\w+.*?;|\w+\s*(?:=|\+=|-=)\s*[^/]+;)\s*$"
        )
        for i, comm in enumerate(comment_lines):
            if not comm:
                continue
            # Strictly printable ASCII + whitespace [CS-0090.4 / JSF AV Rule 9]
            if any(ord(ch) > 127 for ch in comm):
                self.report(
                    ComplianceViolation(
                        "CS-0090.4",
                        file_path,
                        i + 1,
                        "Non-ASCII character detected in comment (strict 7-bit ASCII required).",
                        severity="warning",
                    )
                )
            stripped = raw_lines[i].strip()
            if stripped.startswith("//") and not stripped.startswith(("///", "//!", "// @", "// [")):
                body = stripped[2:].strip()
                if code_in_comment.match(body):
                    self.report(
                        ComplianceViolation(
                            "CS-0090.4",
                            file_path,
                            i + 1,
                            "Suspected commented-out C++ statement (MISRA Dir 4.4 / CS-0090.4).",
                            severity="warning",
                        )
                    )


def main():
    parser = argparse.ArgumentParser(description="Quasar Compliance Suite [CS-0010..CS-0090]")
    parser.add_argument("--root", default=".", help="Project root directory")
    parser.add_argument("--dirs", nargs="+", default=["cmake-projects"], help="Directories to scan")
    parser.add_argument("--exclude", nargs="+", default=[], help="Directories to exclude")
    parser.add_argument("--format", choices=["text", "json"], default="text", help="Output format")
    parser.add_argument("--max-uncommented", type=int, default=5, help="Max consecutive uncommented lines [CS-0010.44]")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if any violation is reported")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, "..")) if args.root == "." else os.path.abspath(args.root)

    checker = ComplianceChecker(project_root, max_uncommented=args.max_uncommented)
    checker.scan_files(args.dirs, args.exclude)

    if args.format == "json":
        print(json.dumps([v.to_dict() for v in checker.violations], indent=2))
    else:
        for v in checker.violations:
            print(v)
        print(f"\nTotal violations: {len(checker.violations)}")

    if args.strict and checker.violations:
        sys.exit(1)


if __name__ == "__main__":
    main()
