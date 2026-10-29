import os, re

root = r"E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame"
cn = re.compile(r'[\u4e00-\u9fff]')
hdecl = re.compile(r'^\t(?:virtual\s+|static\s+|UE_API\s+)?[A-Za-z_][\w:<>\*&\s]*\([^;]*\)\s*(?:const\s*)?(?:override\s*)?;')
cppdef = re.compile(r'^[A-Za-z_][\w:<>\*&\s]*\b(\w+)::(~?\w+)\s*\(')

skip = {'//', '/*', '*', '#', 'UFUNCTION', 'UPROPERTY', 'DECLARE_', 'GENERATED_', 'UE_INLINE'}
res = {}
for dirpath, _, files in os.walk(root):
    for f in sorted(files):
        if not f.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dirpath, f)
        lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
        inblock = 0
        for i, ln in enumerate(lines):
            is_h = f.endswith('.h')
            m = hdecl.match(ln) if is_h else cppdef.match(ln)
            if not m:
                continue
            s = ln.strip()
            if any(s.startswith(k) for k in skip):
                continue
            if is_h and '::' in s:
                continue
            prev = [lines[j].strip() for j in range(max(0, i-4), i)]
            ok = any(c.startswith('//') and cn.search(c) for c in prev)
            if not ok:
                res.setdefault(os.path.relpath(dirpath, root), []).append(f"{f}:{i+1}")
for d in sorted(res):
    print(d, len(res[d]))
    for x in res[d]:
        print('   ', x)
