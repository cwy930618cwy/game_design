# -*- coding: utf-8 -*-
import io, os, json, sys

ENGINE  = sys.argv[1]
RESTORE = sys.argv[2]
MAPOUT  = sys.argv[3]
DATAS   = sys.argv[4:]

pre = {}
rep = {}
for d in DATAS:
    with io.open(d, 'r', encoding='utf-8-sig') as f:
        j = json.loads(f.read(), strict=False)
    for k, v in j.get('pre', {}).items():
        pre[int(k)] = v
    for k, v in j.get('rep', {}).items():
        rep[int(k)] = v

print('pre entries =', len(pre), 'rep entries =', len(rep))

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

WS = ' \t'


def leading_ws(s):
    i = 0
    while i < len(s) and s[i] in WS:
        i += 1
    return s[:i]


out = []
emap = {}
for i, line in enumerate(lines, start=1):
    if i in pre:
        out.append(pre[i])
    emap[i] = len(out) + 1
    if i in rep:
        newline = rep[i]
        old_ws = leading_ws(line)
        new_ws = leading_ws(newline)
        if old_ws != new_ws:
            newline = old_ws + newline[len(new_ws):]
        out.append(newline)
    else:
        out.append(line)

os.makedirs(os.path.dirname(RESTORE), exist_ok=True)
with io.open(RESTORE, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(out))
print('restore lines =', len(out), 'engine lines =', len(lines))


def get_code(path, strip_tail):
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
            if strip_tail and '//' in t:
                t = t.split('//')[0].strip()
            res.append(t)
    return res


for strip_tail in (False, True):
    o = get_code(RESTORE, strip_tail)
    e = get_code(ENGINE, strip_tail)
    diff = 0
    for i in range(max(len(o), len(e))):
        a = o[i] if i < len(o) else '<none>'
        b = e[i] if i < len(e) else '<none>'
        if a != b:
            diff += 1
            if diff <= 8:
                print('DIFF[%d] OURS: %s' % (i, a))
                print('DIFF[%d] ENG : %s' % (i, b))
    print('strip_tail=%s codeOurs=%d codeEngine=%d diff=%d' % (strip_tail, len(o), len(e), diff))

with io.open(MAPOUT, 'w', encoding='utf-8') as f:
    json.dump(emap, f)