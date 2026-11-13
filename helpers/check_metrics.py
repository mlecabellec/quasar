import argparse
import json
import os
import re
import sys


def strip_comments_and_strings(lines):
    """Return per-line code strings with block/line comments and literals blanked."""
    cleaned = []
    comment_lines = []
    in_block = False

    for line in lines:
        out = []
        has_comment = False
        i = 0
        n = len(line)
        in_str = False
        str_char = ""
        while i < n:
            ch = line[i]
            nxt = line[i + 1] if i + 1 < n else ""
            if in_block:
                has_comment = True
                if ch == "*" and nxt == "/":
                    in_block = False
                    i += 2
                    continue
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
                has_comment = True
                in_block = True
                i += 2
                continue
            if ch == "/" and nxt == "/":
                has_comment = True
                break
            if ch in ('"', "'"):
                in_str = True
                str_char = ch
                out.append(" ")
                i += 1
                continue
            out.append(ch)
            i += 1

        cleaned.append("".join(out))
        comment_lines.append(has_comment)
    return cleaned, comment_lines


def analyze_file(file_path):
    """Measure class length [CS-0010.36], function length [CS-0010.35], and in-body comment presence [CS-0090.5]."""
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        raw_lines = f.readlines()

    cleaned_lines, has_comment = strip_comments_and_strings(raw_lines)
    violations = []

    class_pattern = re.compile(r"\b(?:class|struct)\s+(\w+)\b")
    control_prefix = re.compile(r"^\s*(?:\}\s*)?(?:else|if|for|while|switch|catch|try|do|namespace|return|using)\b")
    control_names = {"if", "else", "for", "while", "switch", "catch", "try", "do", "namespace", "return", "sizeof", "alignof", "decltype"}
    func_pattern = re.compile(
        r"^\s*(?:[\w:~<>\*&,\s]+\s+)?([~A-Za-z_]\w*)\s*\([^;]*\)\s*(?:const\s*)?(?:noexcept(?:\([^)]*\))?\s*)?(?:override\s*)?(?:final\s*)?(?:->\s*[\w:<>&*\s]+)?\s*\{"
    )

    brace_depth = 0
    scope_stack = []
    pending_func = None
    pending_class = None

    for idx, clean in enumerate(cleaned_lines):
        line_no = idx + 1
        stripped = clean.strip()
        in_func = any(e["kind"] == "func" for e in scope_stack)

        c_match = class_pattern.search(clean)
        if c_match and ";" not in clean and "{" in clean:
            pending_class = (c_match.group(1), line_no)
        elif c_match and ";" not in clean and "{" not in clean:
            pending_class = (c_match.group(1), line_no)

        if "{" in clean and pending_class is None and not in_func and not control_prefix.search(clean):
            f_match = func_pattern.search(clean)
            if f_match and f_match.group(1) not in control_names:
                pending_func = (f_match.group(1), line_no)
            elif idx > 0 and stripped.startswith("{"):
                prev_clean = cleaned_lines[idx - 1]
                f_prev = re.search(
                    r"([~A-Za-z_]\w*)\s*\([^;]*\)\s*(?:const\s*)?(?:noexcept(?:\([^)]*\))?\s*)?(?:override\s*)?(?:final\s*)?(?:->\s*[\w:<>&*\s]+)?$",
                    prev_clean.strip(),
                )
                if (
                    f_prev
                    and f_prev.group(1) not in control_names
                    and not control_prefix.search(prev_clean)
                    and not re.search(r"\b(if|else|for|while|switch|catch|try|do|namespace)\b", prev_clean)
                ):
                    pending_func = (f_prev.group(1), idx)

        for _ in range(clean.count("{")):
            brace_depth += 1
            if pending_class is not None:
                scope_stack.append(
                    {"kind": "class", "name": pending_class[0], "start": pending_class[1], "depth": brace_depth, "comments": 0, "code": 0}
                )
                pending_class = None
            elif pending_func is not None:
                scope_stack.append(
                    {"kind": "func", "name": pending_func[0], "start": pending_func[1], "depth": brace_depth, "comments": 0, "code": 0}
                )
                pending_func = None

        # Accumulate code/comment counts for innermost active function
        for entry in reversed(scope_stack):
            if entry["kind"] == "func":
                if has_comment[idx]:
                    entry["comments"] += 1
                if stripped and stripped not in ("{", "}"):
                    entry["code"] += 1
                break

        for _ in range(clean.count("}")):
            if scope_stack and scope_stack[-1]["depth"] == brace_depth:
                closed = scope_stack.pop()
                length = line_no - closed["start"] + 1
                if closed["kind"] == "class" and length > 1600:
                    violations.append(
                        (closed["start"], "CS-0010.36", f"Class '{closed['name']}' too long ({length} lines > 1600) [CS-0010.36]")
                    )
                elif closed["kind"] == "func":
                    if length > 200:
                        violations.append(
                            (closed["start"], "CS-0010.35", f"Function '{closed['name']}' too long ({length} lines > 200) [CS-0010.35]")
                        )
                    if closed["code"] > 5 and closed["comments"] == 0:
                        violations.append(
                            (closed["start"], "CS-0090.5", f"Function '{closed['name']}' ({closed['code']} code lines) lacks in-body justification comments [CS-0090.5]")
                        )
            brace_depth = max(0, brace_depth - 1)

    return violations


def main():
    parser = argparse.ArgumentParser(description="Code metrics & in-body comment checker [CS-0010.35-36, CS-0090.5]")
    parser.add_argument("dirs", nargs="+")
    parser.add_argument("--exclude", nargs="+", default=[])
    parser.add_argument("--format", choices=["text", "json"], default="text")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if violations found")
    args = parser.parse_args()

    abs_exclude = [os.path.abspath(d) for d in args.exclude]
    all_items = []
    for d in args.dirs:
        if os.path.isfile(d):
            for line_no, rule_id, msg in analyze_file(d):
                all_items.append({"file": d, "line": line_no, "rule_id": rule_id, "message": msg})
            continue
        for root, dirs, files in os.walk(d):
            dirs[:] = [dr for dr in dirs if os.path.abspath(os.path.join(root, dr)) not in abs_exclude]
            for f in files:
                if f.endswith((".cpp", ".h", ".hpp")):
                    p = os.path.join(root, f)
                    for line_no, rule_id, msg in analyze_file(p):
                        all_items.append({"file": p, "line": line_no, "rule_id": rule_id, "message": msg})

    if args.format == "json":
        print(json.dumps(all_items, indent=2))
    else:
        for item in all_items:
            print(f"{item['file']}:{item['line']} - {item['message']}")

    if args.strict and all_items:
        sys.exit(1)


if __name__ == "__main__":
    main()
