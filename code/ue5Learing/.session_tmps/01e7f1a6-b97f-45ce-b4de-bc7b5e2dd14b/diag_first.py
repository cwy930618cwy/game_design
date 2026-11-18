# -*- coding: utf-8 -*-
import io, sys

ENGINE  = sys.argv[1]
RESTORE = sys.argv[2]


def get_code(path, strip_tail=True):
    res = []
    in_block = False
    with io.open(path, 'r', encoding='utf-8', newline='') as f:
        for idx, l in enumerate(f.read().split('\n'), start=1):
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
            if strip_tail and '//' in t:
                t = t.split('//')[0].strip()
            res.append((idx, t))
    return res


o = get_code(RESTORE)
e = get_code(ENGINE)
print('ours=%d engine=%d' % (len(o), len(e)))

for i in range(max(len(o), len(e))):
    a = o[i] if i < len(o) else (0, '<none>')
    b = e[i] if i < len(e) else (0, '<none>')
    if a[1] != b[1]:
        print('FIRST DIFF at code index %d' % i)
        for k in range(max(0, i - 4), min(len(o), i + 4)):
            print('  ours[%d] L%d: %s' % (k, o[k][0], o[k][1]))
        print('  ---')
        for k in range(max(0, i - 4), min(len(e), i + 4)):
            print('  eng [%d] L%d: %s' % (k, e[k][0], e[k][1]))
        break