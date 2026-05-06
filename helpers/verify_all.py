import os
import subprocess
import sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    cmake_projects = os.path.join(project_root, "cmake-projects")
    third_party = os.path.join(cmake_projects, "third-party")

    print("====================================================")
    print("   QUASAR ARCHITECTURAL COMPLIANCE VERIFIER")
    print("====================================================")
    print(f"Project Root: {project_root}")
    print(f"Excluding: {third_party}")
    print()

    tools = [
        ["python3", os.path.join(script_dir, "compliance_suite.py"), "--dirs", "cmake-projects", "--exclude", "cmake-projects/third-party", "--root", project_root],
        ["python3", os.path.join(script_dir, "check_annotations.py"), cmake_projects, "--exclude", third_party],
        ["python3", os.path.join(script_dir, "check_metrics.py"), cmake_projects, "--exclude", third_party]
    ]

    for tool in tools:
        print(f"Running: {' '.join(tool)}")
        subprocess.run(tool)
        print()

    print("====================================================")
    print("Verification Complete.")

if __name__ == "__main__":
    main()
