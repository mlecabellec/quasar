#!/usr/bin/env python3
import subprocess
import re
import os

def run_cmd(cmd):
    try:
        return subprocess.check_output(cmd, shell=True, text=True, errors='replace', stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        return e.output

def main():
    print("Extracting git history for C++ files...")
    # Get all commits that modified cpp/hpp files in the last 14 days
    log_output = run_cmd('git log -p -U0 --since="14 days ago" -- "*.cpp" "*.hpp"')
    
    current_commit = ""
    deleted_signatures = []
    
    # Regex to roughly match a C++ method signature (e.g. ReturnType ClassName::MethodName(...))
    sig_pattern = re.compile(r'^-[\s]*((?:[a-zA-Z_]\w*(?:<[^>]+>)?\s+)+(?:[a-zA-Z_]\w*::)?~?[a-zA-Z_]\w*\s*\([^)]*\))(?:\s*(?:const|override|noexcept)*\s*(?:{|;))')
    
    lines = log_output.split('\n')
    for line in lines:
        if line.startswith('commit '):
            current_commit = line.split()[1][:8]
        elif line.startswith('-') and not line.startswith('---'):
            match = sig_pattern.match(line)
            if match:
                sig = match.group(1).strip()
                deleted_signatures.append((current_commit, sig))
                
    print(f"Found {len(deleted_signatures)} deleted signatures.")
    
    # Check if they exist in the current tree
    missing_signatures = []
    for commit, sig in deleted_signatures:
        # Extract just the method name for a safer grep search
        method_name_match = re.search(r'(~?[a-zA-Z_]\w*)\s*\(', sig)
        if method_name_match:
            method_name = method_name_match.group(1)
            # Find if this method name still exists in current cpp/hpp files
            # Just matching the method name + '('
            grep_cmd = f'git grep -E "{method_name}\s*\\(" -- "*.cpp" "*.hpp"'
            try:
                subprocess.check_output(grep_cmd, shell=True, text=True, stderr=subprocess.DEVNULL)
                # Method exists (could be refactored)
            except subprocess.CalledProcessError:
                missing_signatures.append((commit, sig))
                
    with open('deletion_analysis_report.md', 'w') as f:
        f.write("# Git History Deletion Analysis\n\n")
        f.write("The following method signatures were deleted in recent commits and appear to be completely absent from the current codebase.\n\n")
        f.write("| Commit | Deleted Signature |\n")
        f.write("| ------ | ----------------- |\n")
        for commit, sig in missing_signatures:
            f.write(f"| `{commit}` | `{sig}` |\n")
            
    print(f"Report generated: deletion_analysis_report.md with {len(missing_signatures)} missing signatures.")

if __name__ == '__main__':
    main()
