# -*- coding: utf-8 -*-
import io, json, sys

p = sys.argv[1]
with io.open(p, 'r', encoding='utf-8-sig') as f:
    d = json.loads(f.read(), strict=False)

pre = d.get('pre', {})
rep = d.get('rep', {})
removed = []
for k in list(pre.keys()):
    v = rep.get(k, '')
    vs = v.strip()
    if vs.startswith('/*') and vs.endswith('*/'):
        pre.pop(k)
        removed.append(k)

with io.open(p, 'w', encoding='utf-8', newline='') as f:
    f.write(json.dumps(d, ensure_ascii=False, indent=2))
print('removed duplicate pre =', removed)