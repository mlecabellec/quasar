import argparse
import os
import shutil
import subprocess
import sys
import tempfile


def run_doxygen_check(project_root, target_dirs):
    """Run Doxygen validation cleanly into an isolated temporary output directory [CS-0090.1]."""
    doxygen_bin = shutil.which("doxygen")
    doxyfile = os.path.join(project_root, "Doxyfile-html")
    if not doxygen_bin or not os.path.exists(doxyfile):
        print("[WARN] Doxygen or Doxyfile-html not found; skipping Doxygen check.")
        return 0

    tmp_out = tempfile.mkdtemp(prefix="quasar_doxygen_verify_")
    with open(doxyfile, "r", encoding="utf-8", errors="ignore") as f:
        cfg = f.read()

    abs_inputs = " ".join(f'"{os.path.abspath(d)}"' for d in target_dirs)
    override = (
        f"{cfg}\n"
        f"INPUT = {abs_inputs}\n"
        f"OUTPUT_DIRECTORY = \"{tmp_out}\"\n"
        f"GENERATE_HTML = YES\n"
        f"GENERATE_LATEX = NO\n"
        f"SEARCHENGINE = NO\n"
        f"QUIET = YES\n"
        f"WARN_AS_ERROR = NO\n"
    )
    proc = subprocess.run([doxygen_bin, "-"], input=override, text=True, capture_output=True, cwd=project_root)
    warnings = [ln for ln in proc.stderr.splitlines() if ln.strip()]
    shutil.rmtree(tmp_out, ignore_errors=True)
    for w in warnings:
        print(f"[DOXYGEN] {w}")
    print(f"Total Doxygen warnings: {len(warnings)}")
    return len(warnings)


def main():
    parser = argparse.ArgumentParser(description="Quasar Architectural Compliance Verifier")
    parser.add_argument("--dirs", nargs="+", default=None, help="Target directories/files relative to root")
    parser.add_argument("--exclude", nargs="+", default=None, help="Excluded directories")
    parser.add_argument("--skip-doxygen", action="store_true", help="Skip Doxygen validation pass")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if any checker fails")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    default_dir = os.path.join(project_root, "cmake-projects")
    default_excl = os.path.join(default_dir, "third-party")

    target_dirs = args.dirs if args.dirs else [default_dir]
    excl_dirs = args.exclude if args.exclude is not None else [default_excl]
    rel_targets = [os.path.relpath(os.path.abspath(d), project_root) for d in target_dirs]
    rel_excludes = [os.path.relpath(os.path.abspath(d), project_root) for d in excl_dirs]

    print("====================================================")
    print("   QUASAR ARCHITECTURAL COMPLIANCE VERIFIER")
    print("====================================================")
    print(f"Project Root: {project_root}")
    print(f"Targets:      {', '.join(rel_targets)}")
    print(f"Excluding:    {', '.join(rel_excludes)}")
    print()

    strict_flag = ["--strict"] if args.strict else []
    tools = [
        ["python3", os.path.join(script_dir, "compliance_suite.py"), "--dirs", *rel_targets, "--exclude", *rel_excludes, "--root", project_root, *strict_flag],
        ["python3", os.path.join(script_dir, "check_annotations.py"), *target_dirs, "--exclude", *excl_dirs, *strict_flag],
        ["python3", os.path.join(script_dir, "check_metrics.py"), *target_dirs, "--exclude", *excl_dirs, *strict_flag],
        ["python3", os.path.join(script_dir, "ast_nodiscard_checker.py"), *target_dirs, "--exclude", *excl_dirs, *strict_flag],
    ]

    failures = 0
    for tool in tools:
        print(f"Running: {' '.join(tool)}")
        res = subprocess.run(tool)
        if res.returncode != 0:
            failures += 1
        print()

    if not args.skip_doxygen:
        print("Running: Doxygen validation [CS-0090.1-3]")
        warn_count = run_doxygen_check(project_root, target_dirs)
        if args.strict and warn_count > 0:
            failures += 1
        print()

    print("====================================================")
    print("Verification Complete.")
    if args.strict and failures > 0:
        sys.exit(1)


if __name__ == "__main__":
    main()
