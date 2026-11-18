# -*- coding: utf-8 -*-
import io, json, sys

ENGINE = sys.argv[1]
DATAS  = sys.argv[2:]

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

rep = {}
for d in DATAS:
    with io.open(d, 'r', encoding='utf-8-sig') as f:
        j = json.loads(f.read(), strict=False)
    for k, v in j.get('rep', {}).items():
        rep[int(k)] = v

bad = []
for k in sorted(rep):
    if k < 1 or k > len(lines):
        bad.append((k, '<out of range>', rep[k]))
        continue
    orig = lines[k - 1].strip()
    if not (orig.startswith('//') or orig.startswith('/*') or orig.startswith('*')):
        bad.append((k, orig, rep[k]))

print('bad rep entries =', len(bad))
for k, o, n in bad:
    print('%d | ORIG: %s' % (k, o))
    print('   | NEW : %s' % n)