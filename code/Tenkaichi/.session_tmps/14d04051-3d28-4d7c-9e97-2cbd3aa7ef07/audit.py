import os, re, sys

BASE = r'E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra_从零底层代码搭建重构2'
SRC  = r'E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame'
LY   = r'E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame'

FOLDERS = ['P00_Physics','P01_Messages','P02_System叶子','P03_Input','P04_Camera','P05_AbilitySystem']
LYF     = ['Physics','Messages','Input','Camera','AbilitySystem']

print('#'*70)
print('# 1. md 代码块内的 Lyra 残留（铁律 41 违规）')
print('#'*70)
for f in FOLDERS:
    d = os.path.join(BASE, f)
    for dp, dn, fn in os.walk(d):
        for n in sorted(fn):
            if not n.endswith('.md'):
                continue
            p = os.path.join(dp, n)
            lines = open(p, encoding='utf-8', errors='ignore').read().split('\n')
            inb = False
            for i, l in enumerate(lines, 1):
                s = l.strip()
                if s.startswith('```'):
                    inb = not inb
                    continue
                if inb and re.search(r'\bLyra', l):
                    print(f'{f}/{n}:{i}: {l.rstrip()[:130]}')

print()
print('#'*70)
print('# 2. md 文件名不规范（Lyra 前缀 / 旧编号）')
print('#'*70)
for f in FOLDERS:
    d = os.path.join(BASE, f)
    for dp, dn, fn in os.walk(d):
        for n in sorted(fn):
            if not n.endswith('.md'):
                continue
            if re.search(r'Lyra', n) or re.match(r'^\d', n):
                print(f'{f}/{n}')

print()
print('#'*70)
print('# 3. 工程代码中的英文行注释（铁律 39）')
print('#'*70)
EN_OK = ('Copyright', 'http', 'GENERATED', '#pragma', '#include', '#define', '#if', '#endif', '//~')
for dp, dn, fn in os.walk(SRC):
    for n in sorted(fn):
        if not n.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dp, n)
        rel = os.path.relpath(p, SRC)
        for i, l in enumerate(open(p, encoding='utf-8', errors='ignore'), 1):
            m = re.search(r'(//|/\*)\s*(.*)$', l)
            if not m:
                continue
            txt = m.group(2).strip()
            if not txt:
                continue
            if any(txt.startswith(x) for x in EN_OK):
                continue
            # 纯 ASCII 且含英文单词 = 疑似未中文化
            letters = re.findall(r'[A-Za-z]{3,}', txt)
            if letters and not re.search(r'[\u4e00-\u9fff]', txt):
                print(f'{rel}:{i}: {l.rstrip()[:120]}')
