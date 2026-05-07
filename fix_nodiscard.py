import re
import os

def fix_file(path):
    with open(path, 'r') as f:
        content = f.read()

    original = content

    # 1. Misplaced template attribute
    content = re.sub(r'\[\[nodiscard\]\]\s+template\s*<([^>]+)>', r'template <\1> [[nodiscard]]', content)

    # 2. Invalid attribute on constructors or initializer lists
    content = re.sub(r'\[\[nodiscard\]\]\s+:', ':', content)
    content = re.sub(r'\[\[nodiscard\]\]\s+explicit', 'explicit', content)
    
    # 3. Invalid attribute on variable declarations
    bad_var_types = [
        r'std::unique_lock<[^>]+>',
        r'std::lock_guard<[^>]+>',
        r'std::scoped_lock<[^>]+>',
        r'std::chrono::microseconds',
        r'std::chrono::system_clock::time_point'
    ]
    
    for vt in bad_var_types:
        content = re.sub(r'\[\[nodiscard\]\]\s+(' + vt + r')', r'\1', content)

    # Targeted fix for std::string variable declarations
    content = re.sub(r'\[\[nodiscard\]\]\s+std::string\s+(\w+)\(static_cast', r'std::string \1(static_cast', content)

    if content != original:
        with open(path, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    count = 0
    for root, dirs, files in os.walk('cmake-projects'):
        for file in files:
            if file.endswith('.h') or file.endswith('.hpp'):
                path = os.path.join(root, file)
                if fix_file(path):
                    print(f"Fixed: {path}")
                    count += 1
    print(f"Total files fixed: {count}")

if __name__ == "__main__":
    main()
