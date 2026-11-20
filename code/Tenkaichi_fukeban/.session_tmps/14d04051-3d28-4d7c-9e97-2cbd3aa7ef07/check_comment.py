import os, re, sys

root = r"E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame"
decl = re.compile(r'^\s*(virtual\s+)?(static\s+)?([\w:<>\*&\s]*?)\b(\w+)\s*\([^;{]*\)(\s*const)?\s*(override|final)?\s*;')
cn = re.compile(r'[\u4e00-\u9fff]')

bad = []
for dirpath, _, files in os.walk(root):
    for f in files:
        if not f.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dirpath, f)
        lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
        for i, ln in enumerate(lines):
            s = ln.strip()
            if s.startswith('//') or s.startswith('/*') or s.startswith('*') or s.startswith('#'):
                continue
            if not decl.match(ln):
                continue
            # 只看函数声明/定义行
            if '(' not in ln:
                continue
            prev = lines[i-1].strip() if i > 0 else ''
            prev2 = lines[i-2].strip() if i > 2 else ''
            ok = (prev.startswith('//') and cn.search(prev)) or (prev2.startswith('//') and cn.search(prev2))
            if not ok:
                bad.append((os.path.relpath(p, root), i+1, s[:90], prev[:60]))
for b in bad:
    print('|'.join(str(x) for x in b))
print('TOTAL', len(bad))
