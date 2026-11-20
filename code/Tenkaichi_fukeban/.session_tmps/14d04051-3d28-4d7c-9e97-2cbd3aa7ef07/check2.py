import os, re

root = r"E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\Teams"
cn = re.compile(r'[\u4e00-\u9fff]')
# .h 里的函数声明：以 类型 开头，含 ( ，以 ; 结尾，且不是 UFUNCTION/UPROPERTY
hdecl = re.compile(r'^\t(?:virtual\s+|static\s+|UE_API\s+)?[A-Za-z_][\w:<>\*&\s]*\([^;]*\)\s*(?:const\s*)?(?:override\s*)?;')
# .cpp 里的顶层函数定义：行首 返回类型 类名::函数名(
cppdef = re.compile(r'^[A-Za-z_][\w:<>\*&\s]*\b(\w+)::(~?\w+)\s*\(')

for dirpath, _, files in os.walk(root):
    for f in sorted(files):
        if not f.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dirpath, f)
        lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
        for i, ln in enumerate(lines):
            is_h = f.endswith('.h')
            m = hdecl.match(ln) if is_h else cppdef.match(ln)
            if not m:
                continue
            s = ln.strip()
            if s.startswith(('UFUNCTION', 'UPROPERTY', 'DECLARE_', 'return', '//')):
                continue
            if '::' in s and s.endswith(';'):
                continue
            prev = lines[i-1].strip() if i > 0 else ''
            prev2 = lines[i-2].strip() if i > 1 else ''
            prev3 = lines[i-3].strip() if i > 2 else ''
            cands = [prev, prev2, prev3]
            ok = any(c.startswith('//') and cn.search(c) for c in cands)
            if not ok:
                print(f"{f}:{i+1}: {s[:95]}   <-- prev=[{prev[:50]}]")
