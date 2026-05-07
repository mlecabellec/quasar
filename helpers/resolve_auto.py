import os
import re
import argparse

def resolve_auto_in_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    new_lines = []
    changed = False
    
    for line in lines:
        new_line = line
        
        # std::dynamic_pointer_cast
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*std::dynamic_pointer_cast<([^>]+)>', r'std::shared_ptr<\2> \1 = std::dynamic_pointer_cast<\2>', new_line)
        
        # weakSelf.lock()
        new_line = re.sub(r'if\s*\(\s*auto\s+([a-zA-Z_]\w*)\s*=\s*weakSelf\.lock\(\)\s*\)', r'if (std::shared_ptr<NamedService> \1 = weakSelf.lock())', new_line)
        
        # getChild
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*getChild\(', r'std::shared_ptr<NamedObject> \1 = getChild(', new_line)
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*([a-zA-Z_]\w*)->getChild\(', r'std::shared_ptr<NamedObject> \1 = \2->getChild(', new_line)
        
        # NamedObject::create
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*NamedObject::create\(', r'std::shared_ptr<NamedObject> \1 = NamedObject::create(', new_line)
        
        # const auto& for SlaveInfo
        new_line = re.sub(r'const\s+auto&\s+slaves\b', r'const std::vector<SlaveInfo>& slaves', new_line)
        new_line = re.sub(r'const\s+auto&\s+slaveInfo\b', r'const SlaveInfo& slaveInfo', new_line)
        new_line = re.sub(r'auto&\s+slaveInfo\b', r'SlaveInfo& slaveInfo', new_line)
        
        # enumerate()
        new_line = re.sub(r'\bauto\s+result\s*=\s*m_enumerator->enumerate\(\)', r'Result<size_t> result = m_enumerator->enumerate()', new_line)
        
        # NamedString::create
        new_line = re.sub(r'\bauto\s+([a-zA-Z_]\w*)\s*=\s*NamedString::create\(', r'std::shared_ptr<NamedString> \1 = NamedString::create(', new_line)

        if new_line != line:
            changed = True
        new_lines.append(new_line)
    
    if changed:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+")
    args = parser.parse_args()
    
    for p in args.files:
        if resolve_auto_in_file(p):
            print(f"Resolved auto in {p}")

if __name__ == "__main__":
    main()
