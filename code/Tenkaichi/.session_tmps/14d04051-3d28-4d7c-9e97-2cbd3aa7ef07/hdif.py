import os, re

LY = r'E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame'
P  = r'E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame'
LYF = ['Physics','Messages','Input','Camera','AbilitySystem']

def norm(s):
    return s.replace('Lyra', 'Tenkaichi')

# UPROPERTY / UFUNCTION 属性名
PROP = re.compile(r'UPROPERTY|UFUNCTION')
FIELD = re.compile(r'^\s*(?:UPROPERTY\([^)]*\)\s*)?(?:UE_API\s+)?(?:mutable\s+|static\s+|virtual\s+|inline\s+)?'
                   r'(?:const\s+)?[\w:<>,\s\*&]+?\b(\w+)\s*(?:=\s*[^;]+)?;')

def fields(path):
    out = set()
    txt = open(path, encoding='utf-8', errors='ignore').read()
    # 只看行首缩进的成员变量/函数声明（粗粒度）
    for l in txt.split('\n'):
        s = l.strip()
        if not s or s.startswith('//') or s.startswith('#') or s.startswith('/*'):
            continue
        m = re.match(r'^\s*(?:UPROPERTY\([^)]*\)\s*)?[\w:<>,\s\*&]+?\b(\w+)\s*\([^;]*\)\s*[^;]*;', l)
        if m and len(m.group(1)) > 2:
            out.add(m.group(1))
    return out

print('='*70)
print('.h 成员函数声明比对（P00~P05）')
print('='*70)
total = 0
for f in LYF:
    for dp, dn, fn in os.walk(os.path.join(LY, f)):
        for n in sorted(fn):
            if not n.endswith('.h'):
                continue
            lp = os.path.join(dp, n)
            rel = os.path.relpath(lp, LY).replace('Lyra', 'Tenkaichi')
            pp = os.path.join(P, rel)
            if not os.path.exists(pp):
                print(f'[缺文件] {rel}')
                continue
            a = fields(lp)
            b = fields(pp)
            miss = a - b
            # 忽略模板/宏噪声
            miss = {x for x in miss if not x.startswith(('GENERATED', 'UE_', 'DECLARE', 'DEFINE'))}
            if miss:
                total += len(miss)
                print(f'\n### {rel} —— 工程缺 {len(miss)} 个')
                for m in sorted(miss):
                    print(f'    - {m}')
print()
print(f'>>> 总计缺失声明：{total}')
