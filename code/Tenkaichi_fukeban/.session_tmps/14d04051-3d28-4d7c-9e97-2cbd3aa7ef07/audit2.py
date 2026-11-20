import os, re

P = r'E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame'
LY = r'E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame'

# 判定：疑似「Lyra 讲解性英文注释」= 以 // 开头、纯英文、含 >=2 个英文单词、
# 且不是：类名分节 / 被注释掉的死代码(含 ; 或 { 或 }) / 参数标记 /*xxx*/ / TODO / Copyright
STOP = ('Copyright', 'TODO', '@TODO', 'NOTE:', 'Note:', 'http')
CODE_CHARS = (';', '{', '}', '=', '->', '(', ')')

def is_class_banner(t):
    # 形如 // UTenkaichiXxx 或 // FTenkaichiXxx 或 // ATenkaichiXxx 单独一行
    return bool(re.fullmatch(r'(U|F|A|S|E|I)?(Tenkaichi|GameplayTagStack)\w*', t))

print('=== 疑似 Lyra 讲解性英文注释（需中文化，铁律 39）===')
for dp, dn, fn in os.walk(P):
    for n in sorted(fn):
        if not n.endswith(('.h', '.cpp')):
            continue
        p = os.path.join(dp, n)
        rel = os.path.relpath(p, P)
        for i, l in enumerate(open(p, encoding='utf-8', errors='ignore'), 1):
            s = l.strip()
            if not s.startswith('//'):
                continue
            t = s[2:].strip()
            if not t:
                continue
            if any(t.startswith(x) for x in STOP):
                continue
            if is_class_banner(t):
                continue
            if any(c in t for c in CODE_CHARS):      # 死代码
                continue
            if re.search(r'[\u4e00-\u9fff]', t):      # 已中文
                continue
            words = re.findall(r'[A-Za-z]{2,}', t)
            if len(words) < 2:                        # 单个词（多为字段名/宏）
                continue
            print(f'{rel}:{i}: {s[:130]}')
