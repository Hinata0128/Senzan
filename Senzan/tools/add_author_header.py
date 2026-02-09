#!/usr/bin/env python3
import os
from datetime import date

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
TARGET_DIRS = [
    os.path.join(ROOT, 'SourceCode', 'Game', '01_GameObject', '00_MeshObject', '00_Character', '01_Player'),
    os.path.join(ROOT, 'SourceCode', 'Game', '02_Camera'),
]
EXTENSIONS = ['.cpp', '.h', '.inl', '.hpp']

AUTHOR = '淵脇 未来.'
TODAY = date.today().isoformat()

HEADER_TEMPLATE = '''/**********************************************************************************
* @author    : {author}
* @date      : {date}.
* @brief     : 
**********************************************************************************/\n\n'''

header_text = HEADER_TEMPLATE.format(author=AUTHOR, date=TODAY)

def should_process(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
            if '@author' in content:
                return False
    except Exception:
        return False
    return True

def prepend_header(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f'Failed to read {path}: {e}')
        return False

    new_content = header_text + content
    try:
        with open(path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f'Prepended header to {path}')
        return True
    except Exception as e:
        print(f'Failed to write {path}: {e}')
        return False

if __name__ == '__main__':
    for target in TARGET_DIRS:
        if not os.path.isdir(target):
            print(f'Skipping missing directory: {target}')
            continue
        for dirpath, dirs, files in os.walk(target):
            for name in files:
                if any(name.endswith(ext) for ext in EXTENSIONS):
                    path = os.path.join(dirpath, name)
                    if should_process(path):
                        prepend_header(path)
                    else:
                        print(f'Skipped (already has author): {path}')
    print('Done.')
