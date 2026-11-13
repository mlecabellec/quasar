import argparse
import glob
import json
import os
import re
import sys

try:
    import clang.cindex
    from clang.cindex import Config, CursorKind

    HAS_LIBCLANG = True
except ImportError:
    HAS_LIBCLANG = False


def configure_libclang():
    """Locate and register libclang shared library across common Linux paths."""
    if not HAS_LIBCLANG or Config.loaded:
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


REF_PATTERN = re.compile(
    r"(?:@(?:task|requirement|constraint|rule|feature|exposed)\b|\[(?:TSK|REQ|RQ|CS|FE|UC)-[A-Za-z0-9_.-]+\])"
)
COMPLIANCE_PATTERN = re.compile(r"(?:@compliance\b|Compliance\s*:)", re.IGNORECASE)
PARAM_DIR_PATTERN = re.compile(
    r"@param\s*\[(in|out|in\s*,\s*out)\]\s+(\w+)(.*?)(?=(?:@(?:param|return|retval|throws|throw|error|containment|pre|post|invariant|task|requirement|constraint|rule|feature|exposed|compliance|note|warning|rationale|consumed)\b|\*/|\Z))",
    re.DOTALL,
)
PARAM_UNDIR_PATTERN = re.compile(r"@param(?!\s*\[(?:in|out|in\s*,\s*out)\])\s+(\w+)")
RETURN_BLOCK_PATTERN = re.compile(
    r"(?:@|\\)return\b(.*?)(?=(?:@(?:param|retval|throws|throw|error|containment|pre|post|invariant|task|requirement|constraint|rule|feature|exposed|compliance|note|warning|rationale|consumed)\b|\*/|\Z))",
    re.DOTALL,
)
CONTAINMENT_BLOCK_PATTERN = re.compile(
    r"(?:@|\\)containment\b(.*?)(?=(?:@(?:param|return|retval|throws|throw|error|pre|post|invariant|task|requirement|constraint|rule|feature|exposed|compliance|note|warning|rationale|consumed)\b|\*/|\Z))",
    re.DOTALL,
)


def extract_preceding_comment(lines, target_line_idx):
    """Collect Doxygen comment (/**...*/, ///..., or trailing ///<) attached to target_line_idx."""
    if 0 <= target_line_idx < len(lines):
        same_line = lines[target_line_idx]
        if "///<" in same_line or "/**<" in same_line:
            return same_line

    collected = []
    i = target_line_idx - 1
    in_block = False
    while i >= 0:
        s = lines[i].strip()
        if not s:
            if collected:
                break
            i -= 1
            continue
        if s.startswith("[["):
            # Skip attribute lines like [[nodiscard]] between comment and declaration
            i -= 1
            continue
        if in_block:
            collected.append(lines[i])
            if "/*" in s:
                break
            i -= 1
            continue
        if s.endswith("*/"):
            in_block = True
            collected.append(lines[i])
            if "/*" in s:
                break
            i -= 1
            continue
        if s.startswith(("///", "//!")):
            collected.append(lines[i])
            i -= 1
            continue
        break

    if not collected:
        return ""
    collected.reverse()
    return "\n".join(collected)


def validate_doxygen_block(file_path, line_no, kind_label, name, doc_text, require_trace=True):
    """Validate CS-0090.1 (@brief) and CS-0090.2 (traceability + @compliance)."""
    issues = []
    if not doc_text or ("/**" not in doc_text and "///" not in doc_text and "//!" not in doc_text and "/*!" not in doc_text):
        issues.append((line_no, "CS-0090.1", f"{kind_label} '{name}' missing Doxygen comment [CS-0010.45, CS-0090.1]."))
        return issues

    if "@brief" not in doc_text and r"\brief" not in doc_text:
        issues.append((line_no, "CS-0090.1", f"{kind_label} '{name}' Doxygen comment missing '@brief' [CS-0090.1]."))

    if require_trace:
        if not REF_PATTERN.search(doc_text):
            issues.append(
                (
                    line_no,
                    "CS-0090.2",
                    f"{kind_label} '{name}' missing reference citation (@task/@requirement/@constraint/@rule/@feature) [CS-0030.1, CS-0090.2].",
                )
            )
        if not COMPLIANCE_PATTERN.search(doc_text):
            issues.append(
                (
                    line_no,
                    "CS-0090.2",
                    f"{kind_label} '{name}' missing '@compliance' explanation [CS-0090.2].",
                )
            )
    return issues


def validate_parameters_and_contract(line_no, name, param_names, return_type, is_ctor_dtor, is_noexcept, doc_text):
    """Validate CS-0090.3 (@param[in|out|in,out] 4-part contract) and return/throws docs."""
    issues = []
    if not doc_text:
        return issues

    undir = PARAM_UNDIR_PATTERN.findall(doc_text)
    for p in undir:
        issues.append(
            (
                line_no,
                "CS-0090.3",
                f"Function '{name}' parameter '{p}' uses '@param' without direction '[in]', '[out]', or '[in,out]' [CS-0090.3].",
            )
        )

    found_params = {}
    for direction, pname, desc in PARAM_DIR_PATTERN.findall(doc_text):
        found_params[pname] = (direction, desc)

    for pname in param_names:
        if pname not in found_params:
            issues.append(
                (
                    line_no,
                    "CS-0090.3",
                    f"Function '{name}' missing '@param[in|out|in,out] {pname}' documentation [CS-0020.77, CS-0090.3].",
                )
            )
            continue

        _, desc = found_params[pname]
        desc_lower = desc.lower()
        missing_facets = []
        if "justification" not in desc_lower and "@rationale" not in desc_lower:
            missing_facets.append("Justification")
        if "usage" not in desc_lower:
            missing_facets.append("Usage")
        if "consumed" not in desc_lower and "mutat" not in desc_lower and "altered" not in desc_lower:
            missing_facets.append("Consumed/Mutation status")
        if "prerequisit" not in desc_lower and "@pre" not in desc_lower and "null" not in desc_lower:
            missing_facets.append("Prerequisites")

        if missing_facets:
            issues.append(
                (
                    line_no,
                    "CS-0090.3",
                    f"Function '{name}' parameter '{pname}' missing required contract field(s): {', '.join(missing_facets)} [CS-0090.3].",
                )
            )

    if not is_ctor_dtor and return_type and return_type != "void":
        ret_match = RETURN_BLOCK_PATTERN.search(doc_text)
        has_retval = "@retval" in doc_text or r"\retval" in doc_text
        if not ret_match and not has_retval:
            issues.append(
                (line_no, "CS-0020.77", f"Non-void function '{name}' missing '@return'/'@retval' [CS-0020.77, CS-0090.7].")
            )
        else:
            ret_desc = (ret_match.group(1) if ret_match else doc_text).lower()
            ret_missing = []
            if "justification" not in ret_desc and "@rationale" not in doc_text.lower():
                ret_missing.append("Justification")
            if "usage" not in ret_desc:
                ret_missing.append("Usage")
            if "consumed" not in ret_desc and "ownership" not in ret_desc and "transfer" not in ret_desc and "borrow" not in ret_desc:
                ret_missing.append("Consumed/Ownership")
            if "prerequisit" not in ret_desc and "@pre" not in doc_text.lower() and "@post" not in doc_text.lower():
                ret_missing.append("Prerequisites")
            if "limitation" not in ret_desc:
                ret_missing.append("Limitations")
            if "lifecycle" not in ret_desc and "lifetime" not in ret_desc:
                ret_missing.append("Lifecycle")
            if "modif" not in ret_desc and "delet" not in ret_desc and "mutab" not in ret_desc:
                ret_missing.append("Modification/Deletion")
            if "specific" not in ret_desc and not has_retval:
                ret_missing.append("Specific Values")
            if "error" not in ret_desc:
                ret_missing.append("Error Behavior")
            if ret_missing:
                issues.append(
                    (
                        line_no,
                        "CS-0090.7",
                        f"Function '{name}' '@return' contract missing field(s): {', '.join(ret_missing)} [CS-0090.7].",
                    )
                )

    can_fail = ("Result" in return_type or "expected" in return_type or not is_noexcept) and not is_ctor_dtor
    if can_fail:
        has_err_doc = "@throw" in doc_text or r"\throw" in doc_text or "@error" in doc_text
        if not has_err_doc:
            issues.append(
                (line_no, "CS-0020.61", f"Function '{name}' missing error-category docs ('@throws'/'@error') [CS-0020.61, CS-0090.8].")
            )
        cont_match = CONTAINMENT_BLOCK_PATTERN.search(doc_text)
        if not cont_match:
            issues.append(
                (line_no, "CS-0090.8", f"Failing function '{name}' missing '@containment' strategy and limitations [CS-0090.8].")
            )
        elif "limitation" not in cont_match.group(1).lower():
            issues.append(
                (line_no, "CS-0090.8", f"Function '{name}' '@containment' missing explicit limitations [CS-0090.8].")
            )

    # Encouraged formal Design-by-Contract & @details checks [CS-0090.1, CS-0090.9]
    is_nontrivial = len(param_names) > 0 or (return_type and return_type != "void")
    if is_nontrivial and "@details" not in doc_text and r"\details" not in doc_text:
        issues.append(
            (line_no, "ADVISORY-CS-0090.1", f"Non-trivial function '{name}' encouraged to include '@details' alongside '@brief' [CS-0090.1].")
        )
    if len(param_names) > 0 and "@pre" not in doc_text and r"\pre" not in doc_text and "[[expects" not in doc_text:
        issues.append(
            (line_no, "ADVISORY-CS-0090.9", f"Function '{name}' encouraged to formalize preconditions via '@pre' or C++26 contracts [CS-0090.9].")
        )
    if not is_ctor_dtor and return_type and return_type != "void" and "@post" not in doc_text and r"\post" not in doc_text and "[[ensures" not in doc_text:
        issues.append(
            (line_no, "ADVISORY-CS-0090.9", f"Function '{name}' encouraged to formalize postconditions via '@post' or C++26 contracts [CS-0090.9].")
        )

    return issues


def check_control_structures_and_bodies(file_path, lines):
    """Check CS-0090.5 (in-body justification comments) and CS-0090.6 (control-structure comments)."""
    issues = []
    ctrl_pattern = re.compile(
        r"^\s*(?:}\s*)?(?:else\s+if\b|else\b|if\s*\(|for\s*\(|while\s*\(|do\b\s*\{?|switch\s*\(|try\b|catch\s*\()"
    )
    in_block_comment = False

    for idx, line in enumerate(lines):
        stripped = line.strip()
        if in_block_comment:
            if "*/" in stripped:
                in_block_comment = False
            continue
        if stripped.startswith("/*"):
            if "*/" not in stripped:
                in_block_comment = True
            continue
        if not stripped or stripped.startswith("//") or stripped.startswith("#"):
            continue

        # Control structure check [CS-0090.6]
        if ctrl_pattern.match(line):
            has_comment = "//" in line or "/*" in line
            # Check immediately preceding lines (skip braces/blank lines up to 2 steps)
            if not has_comment:
                look = idx - 1
                steps = 0
                while look >= 0 and steps < 3:
                    prev = lines[look].strip()
                    if prev == "" or prev in ("{", "}"):
                        look -= 1
                        steps += 1
                        continue
                    if prev.startswith("//") or prev.endswith("*/"):
                        has_comment = True
                    break
            # Also accept an opening comment on the very next line inside '{'
            if not has_comment and idx + 1 < len(lines):
                nxt = lines[idx + 1].strip()
                if nxt.startswith("//") or nxt.startswith("/*"):
                    has_comment = True

            if not has_comment:
                token = stripped.split("(")[0].split("{")[0].strip()
                issues.append(
                    (
                        idx + 1,
                        "CS-0090.6",
                        f"Control structure '{token}' missing explanatory comment describing operation [CS-0090.6].",
                    )
                )

    return issues


def check_file_ast(file_path, include_paths):
    """Inspect C++ declarations and definitions using libclang (-std=c++23) + control check."""
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    issues = []
    abs_target = os.path.abspath(file_path)

    if HAS_LIBCLANG:
        configure_libclang()
        index = clang.cindex.Index.create()
        args = ["-std=c++23", "-x", "c++"]
        for p in include_paths:
            args.append(f"-I{p}")
        tu = index.parse(file_path, args=args)

        seen_lines = set()
        target_kinds = {
            CursorKind.CLASS_DECL: "Class",
            CursorKind.STRUCT_DECL: "Struct",
            CursorKind.ENUM_DECL: "Enum",
            CursorKind.ENUM_CONSTANT_DECL: "Enumerator",
            CursorKind.FIELD_DECL: "Field",
            CursorKind.VAR_DECL: "Constant/Variable",
            CursorKind.TYPE_ALIAS_DECL: "TypeAlias",
            CursorKind.CXX_METHOD: "Method",
            CursorKind.CONSTRUCTOR: "Constructor",
            CursorKind.DESTRUCTOR: "Destructor",
            CursorKind.FUNCTION_DECL: "Function",
            CursorKind.FUNCTION_TEMPLATE: "TemplateFunction",
        }

        for cursor in tu.cursor.walk_preorder():
            try:
                kind = cursor.kind
            except ValueError:
                continue
            if kind not in target_kinds:
                continue
            if not cursor.location.file or os.path.abspath(cursor.location.file.name) != abs_target:
                continue
            if not cursor.spelling:
                continue

            line_no = cursor.location.line
            if (kind, line_no) in seen_lines:
                continue
            seen_lines.add((kind, line_no))

            line_idx = line_no - 1
            raw_line = lines[line_idx] if 0 <= line_idx < len(lines) else ""

            # Collapse macro-synthesized GTest / Catch2 members into one check per test macro line
            stripped_raw = raw_line.lstrip()
            if stripped_raw.startswith(("TEST(", "TEST_F(", "TEST_P(", "TYPED_TEST(", "TEST_CASE(", "SCENARIO(")):
                if kind not in (CursorKind.CLASS_DECL, CursorKind.FUNCTION_DECL):
                    continue

            # Skip local non-constant variables inside function bodies
            if kind == CursorKind.VAR_DECL:
                parent_kind = cursor.lexical_parent.kind if cursor.lexical_parent else None
                if parent_kind not in (CursorKind.TRANSLATION_UNIT, CursorKind.NAMESPACE, CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
                    # Check if namespace/file-level vs local const; still check static constexpr or file/class constants
                    if "constexpr" not in raw_line and "static const" not in raw_line:
                        continue
                if "const" not in cursor.type.spelling and "constexpr" not in raw_line:
                    continue

            # For out-of-line C++ definitions in .cpp matching header decl, skip repeating API contract
            if kind in (CursorKind.CXX_METHOD, CursorKind.CONSTRUCTOR, CursorKind.DESTRUCTOR) and "::" in raw_line:
                if cursor.semantic_parent != cursor.lexical_parent:
                    continue

            # Forward declarations (except pure decl headers)
            if kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL) and not cursor.is_definition():
                continue

            doc_text = cursor.raw_comment or extract_preceding_comment(lines, line_idx)
            label = target_kinds[kind]
            issues.extend(validate_doxygen_block(file_path, line_no, label, cursor.spelling, doc_text))

            if kind in (CursorKind.CXX_METHOD, CursorKind.CONSTRUCTOR, CursorKind.DESTRUCTOR, CursorKind.FUNCTION_DECL, CursorKind.FUNCTION_TEMPLATE):
                is_ctor_dtor = kind in (CursorKind.CONSTRUCTOR, CursorKind.DESTRUCTOR)
                params = [c.spelling for c in cursor.get_children() if c.kind == CursorKind.PARM_DECL and c.spelling]
                ret_type = cursor.result_type.spelling if not is_ctor_dtor else ""
                is_noexcept = "noexcept" in raw_line
                issues.extend(
                    validate_parameters_and_contract(
                        line_no, cursor.spelling, params, ret_type, is_ctor_dtor, is_noexcept, doc_text
                    )
                )

    issues.extend(check_control_structures_and_bodies(file_path, lines))
    return issues


def main():
    parser = argparse.ArgumentParser(description="Quasar Doxygen, Traceability & Control Comment Checker [CS-0090]")
    parser.add_argument("dirs", nargs="+")
    parser.add_argument("--exclude", nargs="+", default=[])
    parser.add_argument("--format", choices=["text", "json"], default="text")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if any violation is reported")
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
            for line_no, rule_id, msg in check_file_ast(directory, include_paths):
                all_violations.append({"file": directory, "line": line_no, "rule_id": rule_id, "message": msg})
            continue
        for root, dirs, files in os.walk(directory):
            dirs[:] = [d for d in dirs if os.path.abspath(os.path.join(root, d)) not in abs_exclude]
            for file in sorted(files):
                if file.endswith((".h", ".hpp", ".cpp")):
                    file_path = os.path.join(root, file)
                    for line_no, rule_id, msg in check_file_ast(file_path, include_paths):
                        all_violations.append({"file": file_path, "line": line_no, "rule_id": rule_id, "message": msg})

    violations = [v for v in all_violations if not v["rule_id"].startswith("ADVISORY-")]
    advisories = [v for v in all_violations if v["rule_id"].startswith("ADVISORY-")]

    if args.format == "json":
        print(json.dumps(all_violations, indent=2))
    else:
        for v in violations:
            print(f"[{v['rule_id']}] {v['file']}:{v['line']} - {v['message']}")
        for a in advisories:
            print(f"[{a['rule_id']}] {a['file']}:{a['line']} - {a['message']}")
        print(f"\nTotal annotation violations: {len(violations)} (advisories: {len(advisories)})")

    if args.strict and violations:
        sys.exit(1)


if __name__ == "__main__":
    main()
