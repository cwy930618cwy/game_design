import os, re

ROOTS = [r'E:\ue5\game_design\code\Tenkaichi\Source']
ENGINE = r'D:\ue5\Epic Games\UE_5.6'

# 收集工程内所有 .h 文件名（可解析的目标）
own = set()
plugins = set()
for base in ROOTS:
    for dp, dn, fn in os.walk(base):
        for n in fn:
            if n.endswith('.h'):
                own.add(n)
# 插件目录
pd = os.path.join(os.path.dirname(ROOTS[0]), 'Plugins')
if os.path.isdir(pd):
    for dp, dn, fn in os.walk(pd):
        for n in fn:
            if n.endswith('.h'):
                plugins.add(n)

INC = re.compile(r'^\s*#include\s+"([^"]+)"')

print('='*70)
print('未注释的 #include 指向不存在文件（编译级 Bug）')
print('='*70)
for base in ROOTS:
    for dp, dn, fn in os.walk(base):
        for n in sorted(fn):
            if not n.endswith(('.h', '.cpp')):
                continue
            p = os.path.join(dp, n)
            rel = os.path.relpath(p, base)
            try:
                lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
            except Exception:
                continue
            inif = False
            for i, l in enumerate(lines, 1):
                if re.match(r'^\s*#if\s+0', l):
                    inif = True
                elif re.match(r'^\s*#endif', l):
                    inif = False
                if inif:
                    continue
                m = INC.match(l)
                if not m:
                    continue
                tgt = m.group(1)
                bn = os.path.basename(tgt)
                # 1) 自身目录 2) 工程任意处 3) 插件 4) 带路径的相对解析
                cand = os.path.join(os.path.dirname(p), bn)
                hit = (os.path.exists(cand) or bn in own or bn in plugins)
                if not hit and '/' not in tgt and not bn.endswith('.generated.h'):
                    print(f'{rel}:{i}: #include "{tgt}"')
