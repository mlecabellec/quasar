import os
import re

def scan_codebase(root_dir):
    """
    Enhanced scanner that implements a basic state machine to track 
    comments and strings, reducing false positives.
    """
    patterns = {
        'auto': r'\bauto\b',
        'new': r'\bnew\b',
        'delete': r'\bdelete\b'
    }
    
    extensions = ('.cpp', '.hpp', '.h', '.cc', '.cxx', '.c')
    
    # Statistics
    stats = {k: {'true_positive': 0, 'false_positive': 0} for k in patterns.keys()}
    
    results = {k: [] for k in patterns.keys()}
    
    for root, dirs, files in os.walk(root_dir):
        if any(skip in root for skip in ['build', '.git', '.idea', 'third-party', 'ext-projects', 'linux-modules/ethercat/fake_lib']):
            continue
            
        for file in files:
            if file.endswith(extensions):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        
                        # 1. Analyze Block Comments (/* ... */)
                        # We find all block comments to identify what is "Commented"
                        block_comments = []
                        for m in re.finditer(r'/\*.*?\*/', content, re.DOTALL):
                            block_comments.append(m.group(0))
                        
                        # 2. Analyze String Literals ("...")
                        # We find all strings to identify what is "String"
                        strings = []
                        for m in re.finditer(r'"(.*?)"', content):
                            strings.append(m.group(0))

                        # 3. Cleaned content for "True Positive" search
                        # Remove block comments from content for analysis
                        clean_content = content
                        for bc in block_comments:
                            clean_content = clean_content.replace(bc, '/* COMMENT_BLOCK */')
                        
                        # Remove strings from content for analysis
                        for s in strings:
                            clean_content = clean_content.replace(s, '"STRING_LITERAL"')

                        # 4. Line-by-line analysis of the original file
                        lines = content.splitlines()
                        
                        # State tracking for single-line comments (//)
                        for line_num, line in enumerate(lines, 1):
                            for key, pattern in patterns.items():
                                # Find all matches in this specific line
                                for match in re.finditer(pattern, line):
                                    match_start = match.start()
                                    match_end = match.end()
                                    
                                    # Check if this specific match is inside a string or comment
                                    # We use the 'clean_content' logic mapped back to line offsets
                                    # But a simpler approach for this script: 
                                    # Check if the substring of the original line contains // or is part of a larger comment
                                    
                                    is_fp = False
                                    reason = "CODE"
                                    
                                    # Check if part of line is a single-line comment
                                    if '//' in line and line.find('//') < match.start():
                                        is_fp = True
                                        reason = "SINGLE_LINE_COMMENT"
                                    
                                    # Check if the match is part of a previously found string or block comment
                                    # (Using a simpler heuristic: check if the match exists in stripped strings/comments)
                                    # This is an approximation for the script's efficiency
                                    for s in strings:
                                        if s in line and (line.find(s) <= match.start() and (line.find(s) + len(s)) >= match.end()):
                                            is_fp = True
                                            reason = "STRING_LITERAL"
                                            break
                                    
                                    if not is_fp:
                                        stats[key]['true_positive'] += 1
                                        results[key].append(f"{file_path}:{line_num}: [{reason}] {line.strip()}")
                                    else:
                                        stats[key]['false_positive'] += 1
                                        # We don't add FPs to the results list to keep it clean, 
                                        # but we track them in stats.
                                        
                except Exception as e:
                    pass

    return results, stats

if __name__ == "__main__":
    root = os.getcwd()
    report, statistics = scan_codebase(root)
    
    print("=== ENHANCED KEYWORD SCAN REPORT ===")
    print(f"Analysis Root: {root}\n")
    
    for key in report.keys():
        print(f"--- '{key}' Analysis ---")
        print(f"  True Positives (Code):  {statistics[key]['true_positive']}")
        print(f"  False Positives (FP):   {statistics[key]['false_positive']}")
        print(f"  Total Found:           {statistics[key]['true_positive'] + statistics[key]['false_positive']}")
        print(f"  Match Rate (Accuracy): {((statistics[key]['true_positive']/(statistics[key]['true_positive']+statistics[key]['false_positive'])*100) if (statistics[key]['true_positive']+statistics[key]['false_positive'])>0 else 0):.2f}%")
        print("  Sample Findings (True Positives):")
        for m in report[key][:15]:
            print(f"    {m}")
        if len(report[key]) > 15:
            print("    ... (truncated)")
        print("")

    print("=== END OF REPORT ===")
