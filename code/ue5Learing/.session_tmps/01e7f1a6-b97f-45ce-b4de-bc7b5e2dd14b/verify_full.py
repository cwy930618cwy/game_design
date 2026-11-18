# -*- coding: utf-8 -*-
import io, re, sys

ENGINE  = sys.argv[1]
RESTORE = sys.argv[2]


def get_code(path):
    res = []
    in_block = False
    with io.open(path, 'r', encoding='utf-8', newline='') as f:
        for l in f.read().split('\n'):
            t = l.strip()
            if in_block:
                if '*/' in t:
                    in_block = False
                continue
            if t == '':
                continue
            if t.startswith('//'):
                continue
            if t.startswith('/*'):
                if '*/' not in t:
                    in_block = True
                continue
            # 剔除行尾 // 注释
            if '//' in t:
                t = t.split('//')[0].strip()
            # 剔除行内 /* */ 注释
            t = re.sub(r'/\*.*?\*/', '', t).strip()
            if t == '':
                continue
            res.append(t)
    return res


o = get_code(RESTORE)
e = get_code(ENGINE)
diff = 0
for i in range(max(len(o), len(e))):
    a = o[i] if i < len(o) else '<none>'
    b = e[i] if i < len(e) else '<none>'
    if a != b:
        diff += 1
        if diff <= 8:
            print('DIFF[%d] OURS: %s' % (i, a))
            print('DIFF[%d] ENG : %s' % (i, b))
print('codeOurs=%d codeEngine=%d diff=%d' % (len(o), len(e), diff))