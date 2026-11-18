# -*- coding: utf-8 -*-
import io, sys

ENGINE = sys.argv[1]
OUT    = sys.argv[2]

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

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

out = []
out.append('total blocks = %d' % len(blocks))
for idx, (s, e) in enumerate(blocks, start=1):
    out.append('')
    out.append('=== BLOCK %d  lines %d-%d ===' % (idx, s, e))
    for ln in range(s, e + 1):
        out.append('%d|%s' % (ln, lines[ln - 1]))

with io.open(OUT, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(out))
print('dumped', len(blocks), 'blocks to', OUT)