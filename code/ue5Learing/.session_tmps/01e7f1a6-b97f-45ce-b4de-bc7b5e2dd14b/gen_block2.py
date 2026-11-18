# -*- coding: utf-8 -*-
import io, os, json, sys

ENGINE  = sys.argv[1]
RESTORE = sys.argv[2]
MAPOUT  = sys.argv[3]
DATAS   = sys.argv[4:]

STAR = chr(42)
SLASH = chr(47)
BLOCK_START = SLASH + STAR
BLOCK_END = STAR + SLASH
DOC_START = SLASH + STAR + STAR

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

blocks = []
i = 0
while i < len(lines):
    t = lines[i].strip()
    if t.startswith(BLOCK_START) and BLOCK_END not in t:
        start = i + 1
        j = i
        while j < len(lines):
            if BLOCK_END in lines[j]:
                break
            j += 1
        blocks.append((start, j + 1))
        i = j + 1
    else:
        i += 1

block_replace = {}
for (s, e) in blocks:
    texts = []
    for ln in range(s, e + 1):
        if ln in rep:
            v = rep[ln].strip()
            if v in (BLOCK_START, BLOCK_END, DOC_START):
                continue
            if v.startswith(DOC_START):
                v = v[3:].strip()
            elif v.startswith(BLOCK_START):
                v = v[2:].strip()
            if v.endswith(BLOCK_END):
                v = v[:-2].strip()
            if v.startswith(STAR):
                v = v.lstrip(STAR).strip()
            if v:
                texts.append(v)
    if texts:
        merged = ' '.join(texts)
        while '  ' in merged:
            merged = merged.replace('  ', ' ')
        block_replace[s] = '// ' + merged.strip()
        for ln in range(s, e + 1):
            rep.pop(ln, None)
            pre.pop(ln, None)

print('blocks =', len(blocks), 'block_replace =', len(block_replace))

for k in [k for k in pre if k in rep]:
    pre.pop(k)

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


def get_code(path):
    res = []
    in_block = False
    with io.open(path, 'r', encoding='utf-8', newline='') as f:
        for l in f.read().split('\n'):
            t = l.strip()
            if in_block:
                if BLOCK_END in t:
                    in_block = False
                continue
            if t == '':
                continue
            if t.startswith('//'):
                continue
            if t.startswith(BLOCK_START):
                if BLOCK_END not in t:
                    in_block = True
                continue
            if '//' in t:
                t = t.split('//')[0].strip()
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

with io.open(MAPOUT, 'w', encoding='utf-8') as f:
    json.dump(emap, f)