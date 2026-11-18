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

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

# 找出所有块注释区间（1-based，含首尾）
blocks = []
i = 0
while i < len(lines):
    t = lines[i].strip()
    if t.startswith('/*') and '*/' not in t:
        start = i + 1
        j = i
        while j < len(lines):
            if '*/' in lines[j]:
                break
            j += 1
        blocks.append((start, j + 1))
        i = j + 1
    else:
        i += 1

# 对每个块：若块内有 rep 条目，则整块压成一行 // 中文注释
block_replace = {}
for (s, e) in blocks:
    texts = []
    for ln in range(s, e + 1):
        if ln in rep:
            v = rep[ln].strip()
            if v.startswith('*'):
                v = v.lstrip('*').strip()
            if v.startswith('/**'):
                v = v[3:].strip()
            if v.endswith('*/'):
                v = v[:-2].strip()
            if v:
                texts.append(v)
    if texts:
        merged = ' '.join(texts)
        merged = merged.replace('  ', ' ').strip()
        block_replace[s] = '// ' + merged
        for ln in range(s, e + 1):
            rep.pop(ln, None)
            pre.pop(ln, None)

print('blocks =', len(blocks), 'block_replace =', len(block_replace))

WS = ' \t'


def leading_ws(s):
    i = 0
    while i < len(s) and s[i] in WS:
        i += 1
    return s[:i]


out = []
emap = {}
i = 0
while i < len(lines):
    ln = i + 1
    if ln in block_replace:
        # 找到该块结束行
        end = ln
        for (s, e) in blocks:
            if s == ln:
                end = e
                break
        if ln in pre:
            out.append(pre[ln])
        emap[ln] = len(out) + 1
        out.append(block_replace[ln])
        for k in range(ln + 1, end + 1):
            emap[k] = len(out)
        i = end
        continue
    if ln in pre:
        out.append(pre[ln])
    emap[ln] = len(out) + 1
    if ln in rep:
        newline = rep[ln]
        old_ws = leading_ws(lines[i])
        new_ws = leading_ws(newline)
        if old_ws != new_ws:
            newline = old_ws + newline[len(new_ws):]
        out.append(newline)
    else:
        out.append(lines[i])
    i += 1

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