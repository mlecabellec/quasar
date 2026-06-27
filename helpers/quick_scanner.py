import os
import re

def robust_scan(root_dir, keywords):
    """
    Extremely robust scanner.
    Identifies True Positives (code) and False Pos 
    by analyzing exact character ranges.
    """
    extensions = ('.cpp', '.hpp', '.h', '.cc', '.cxx', '.c')
    
    # Initialize stats with correct key mapping
    stats = {}
    for kw in keywords:
        stats[kw] = {
            'true_positive': 0,
            'block_comment': 0,
            'line_comment': 0,
            'string_literal': 0
        }
    
    print(f"=== Starting Robust Scan in: {root_dir} ===")
    
    for root, dirs, files in os.walk(root_dir):
        # Skip heavy/irrelevant directories
        # Added 'tmp' and fixed the path-based check to be more robust
        if any(skip in root for skip in ['build', '.git', '.idea', 'third-party', 'ext-projects', 'linux-modules/ethercat/fake_lib', 'tmp']):
            continue
            
        for file in files:
            if file.endswith(extensions):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    # 1. Pre-calculate all "Non-Code" ranges in this file
                    non_code_ranges = []
                    
                    # Block Comments: /* ... */
                    for m in re.finditer(r'/\*[\s\S]*?\*/', content):
                        non_code_ranges.append((m.start(), m.end(), 'block_comment'))
                    
                    # Line Comments: // ...
                    for m in re.finditer(r'//.*', content):
                        non_code_ranges.append((m.start(), m.end(), 'line_comment'))
                        
                    # Strings: "..." (handles escaped quotes)
                    for m in re.finditer(r'"(?:[^"\\]|\\.)*"', content):
                        non_code_ranges.append((m.start(), m.end(), 'string_literal'))

                    # 2. Search for each keyword
                    for kw in keywords:
                        pattern = re.compile(r'\b' + re.escape(kw) + r'\b')
                        for m in re.finditer(pattern, content):
                            match_start = m.start()
                            match_end = m.end()
                            
                            is_fp = False
                            found_reason = "CODE"
                            
                            for start, end, r_type in non_code_ranges:
                                # Check if the keyword match is inside a non-code range
                                if match_start >= start and match_end <= end:
                                    is_fp = True
                                    found_reason = r_type
                                    stats[kw][r_type] += 1
                                    break
                            
                            if not is_fp:
                                stats[kw]['true_positive'] += 1
                                # Line number calculation
                                line_num = content.count('\n', 0, match_start) + 1
                                line_start = content.rfind('\n', 0, match_start) + 1
                                line_end = content.find('\n', match_start)
                                if line_end == -1: line_end = len(content)
                                line_text = content[line_start:line_end].strip()
                                
                                # Print only a subset of TPs to avoid massive output
                                # but we'll print the first 2 per file to see progress
                                # print(f"[TP] {file_path}:{line_num} | {line_text}")

                except Exception as e:
                    # Using stderr for errors to keep stdout clean for the summary
                    import sys
                    print(f"[ERROR] Could not read {file_path}: {e}", file=sys.stderr)

    return stats

if __name__ == "__main__":
    import sys
    target_dir = os.getcwd()
    if len(sys.argv) > 1:
        target_dir = sys.argv[1]
        
    keywords_to_find = ['auto', 'new', 'delete']
    
    final_stats = robust_scan(target_dir, keywords_to_find)
    
    print("\n" + "="*40)
    print("FINAL AUDIT SUMMARY")
    print("="*40)
    for kw, s in final_stats.items():
        tp = s['true_positive']
        fp = s['block_comment'] + s['line_comment'] + s['string_literal']
        accuracy = (tp / (tp + fp) * 100) if (tp + fp) > 0 else 0
        
        print(f"\nKEYWORD: '{kw}'")
        print(f"  [+] True Positives (Code): {tp}")
        print(f"  [-] False Positives (Total): {fp}")
        print(f"    -> inside block comments: {s['block_comment']}")
        print(f"    -> inside line comments:  {s['line_comment']}")
        print(f"    -> inside string literals: {s['string_literal']}")
        print(f"  [!] Accuracy: {accuracy:.2f}%")
    print("\n=== END OF AUDIT ===")
