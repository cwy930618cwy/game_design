import os, re

LY = r'E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame'
P  = r'E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame'
LYF = ['Physics','Messages','Input','Camera','AbilitySystem']

def norm(s):
    return s.replace('Lyra', 'Tenkaichi')

# 提取 类::函数 实现符号
IMPL = re.compile(r'^\s*(?:[\w:<>,*&\s]*\s)?(\w+)::(~?\w+)\s*\(')
# 提取 .h 里的函数声明（近似）
DECL = re.compile(r'^\s*(?:virtual\s+|static\s+|UE_API\s+|inline\s+)*[\w:<>,\s\*&]*?\b(~?\w+)\s*\([^;]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?[;{]')

def impls(path):
    out = set()
    try:
        for l in open(path, encoding='utf-8', errors='ignore'):
            m = IMPL.match(l)
            if m:
                out.add(f'{m.group(1)}::{m.group(2)}')
    except Exception:
        pass
    return out

print('='*70)
print('符号级比对：Lyra .cpp 实现的函数 vs 工程同名 .cpp')
print('='*70)
total_miss = 0
for f in LYF:
    for dp, dn, fn in os.walk(os.path.join(LY, f)):
        for n in sorted(fn):
            if not n.endswith('.cpp'):
                continue
            lp = os.path.join(dp, n)
            rel = os.path.relpath(lp, os.path.join(LY, f))
            pp = os.path.join(P, f, rel).replace('Lyra', 'Tenkaichi')
            pp = os.path.join(P, f, os.path.basename(rel).replace('Lyra', 'Tenkaichi'))
            if not os.path.exists(pp):
                print(f'[缺文件] {f}/{os.path.basename(rel)}')
                continue
            a = {norm(x) for x in impls(lp)}
            b = impls(pp)
            miss = a - b
            if miss:
                total_miss += len(miss)
                print(f'\n### {f}/{os.path.basename(rel)}  工程缺 {len(miss)} 个函数：')
                for m in sorted(miss):
                    print(f'    - {m}')
            extra = b - a
            if extra:
                print(f'\n### {f}/{os.path.basename(rel)}  工程多出 {len(extra)} 个：')
                for m in sorted(extra):
                    print(f'    + {m}')

print()
print(f'总计缺失函数：{total_miss}')
