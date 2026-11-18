# -*- coding: utf-8 -*-
import io, json, sys

ENGINE = sys.argv[1]
DATAS  = sys.argv[2:]

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

# 找出所有「宏续行」区间：从以 \ 结尾的行开始，直到不以 \ 结尾的行
in_macro = False
macro_lines = set()
for i, l in enumerate(lines, start=1):
    stripped = l.rstrip()
    if in_macro:
        macro_lines.add(i)
        if not stripped.endswith('\\'):
            in_macro = False
    else:
        if stripped.endswith('\\'):
            macro_lines.add(i)
            in_macro = True

print('macro continuation lines =', len(macro_lines))

removed = 0
for d in DATAS:
    with io.open(d, 'r', encoding='utf-8-sig') as f:
        j = json.loads(f.read(), strict=False)
    pre = j.get('pre', {})
    for k in list(pre.keys()):
        if int(k) in macro_lines:
            pre.pop(k)
            removed += 1
    with io.open(d, 'w', encoding='utf-8', newline='') as f:
        f.write(json.dumps(j, ensure_ascii=False, indent=2))

print('removed pre entries inside macros =', removed)