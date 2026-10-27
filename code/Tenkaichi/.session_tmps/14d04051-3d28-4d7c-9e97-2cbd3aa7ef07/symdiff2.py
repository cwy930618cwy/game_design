import os, re

LY = r'E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame'
P  = r'E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame'
LYF = ['Physics','Messages','Input','Camera','AbilitySystem']

def norm(s):
    return s.replace('Lyra', 'Tenkaichi')

IMPL = re.compile(r'^\s*(?:[\w:<>,\*&\s]*\s)?(\w+)::(~?\w+)\s*\(')

def impls(path):
    out = set()
    for l in open(path, encoding='utf-8', errors='ignore'):
        m = IMPL.match(l)
        if m:
            out.add(f'{m.group(1)}::{m.group(2)}')
    return out

print('='*70)
print('符号级比对（P00~P05，.cpp 函数实现）')
print('='*70)
total_miss = 0
for f in LYF:
    for dp, dn, fn in os.walk(os.path.join(LY, f)):
        for n in sorted(fn):
            if not n.endswith('.cpp'):
                continue
            lp = os.path.join(dp, n)
            rel = os.path.relpath(lp, LY)                     # 如 AbilitySystem\Abilities\LyraXxx.cpp
            prel = rel.replace('Lyra', 'Tenkaichi')            # 转 Tenkaichi 路径
            pp = os.path.join(P, prel)
            if not os.path.exists(pp):
                print(f'[缺文件] {prel}')
                continue
            a = {norm(x) for x in impls(lp)}
            b = impls(pp)
            miss = a - b
            if miss:
                total_miss += len(miss)
                print(f'\n### {prel}  —— 工程缺 {len(miss)} 个函数')
                for m in sorted(miss):
                    print(f'    - {m}')
            extra = b - a
            if extra:
                print(f'\n### {prel}  —— 工程多出 {len(extra)} 个')
                for m in sorted(extra):
                    print(f'    + {m}')

print()
print(f'>>> 总计缺失函数：{total_miss}')
