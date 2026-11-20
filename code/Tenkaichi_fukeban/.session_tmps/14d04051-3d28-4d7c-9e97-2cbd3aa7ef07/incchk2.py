import os, re

SRC = r'E:\ue5\game_design\code\Tenkaichi\Source'
PROJ = r'E:\ue5\game_design\code\Tenkaichi'

# 引擎 / 插件 头缓存（只建一次文件名集合）
cache = set()
for base in [r'D:\ue5\Epic Games\UE_5.6']:
    if os.path.isdir(base):
        for dp, dn, fn in os.walk(base):
            for n in fn:
                if n.endswith('.h'):
                    cache.add(n)
for base in [os.path.join(PROJ, 'Plugins')]:
    if os.path.isdir(base):
        for dp, dn, fn in os.walk(base):
            for n in fn:
                if n.endswith('.h'):
                    cache.add(n)

own = set()
for dp, dn, fn in os.walk(SRC):
    for n in fn:
        if n.endswith('.h'):
            own.add(n)

INC = re.compile(r'^\s*#include\s+"([^"]+)"')

print('='*70)
print('未注释 include → 目标既不在工程、也不在引擎/插件（真 Bug）')
print('='*70)
bad = []
for dp, dn, fn in os.walk(SRC):
    for n in sorted(fn):
        if not n.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dp, n)
        rel = os.path.relpath(p, SRC)
        lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
        depth = 0
        skip = False
        for i, l in enumerate(lines, 1):
            if re.match(r'^\s*#if\s+0', l):
                skip = True
            elif re.match(r'^\s*#endif', l):
                skip = False
            if skip:
                continue
            m = INC.match(l)
            if not m:
                continue
            tgt = m.group(1)
            bn = os.path.basename(tgt)
            if bn.endswith('.generated.h'):
                continue
            if bn in own or bn in cache:
                continue
            if os.path.exists(os.path.join(os.path.dirname(p), bn)):
                continue
            bad.append((rel, i, tgt))

for r, i, t in bad:
    print(f'{r}:{i}: #include "{t}"')
print(f'\n>>> 共 {len(bad)} 处')
