# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_md2.py'
s = io.open(p, encoding='utf-8').read()

old = """    md.append('```cpp')
    for el in range(s, e_ + 1):
        if el in emap:
            md.append(ours[emap[el] - 1])
    md.append('```')"""

new = """    md.append('```cpp')
    start = emap[s - 1] + 1 if (s - 1) in emap else 1
    end = emap[e_]
    for ln in range(start, end + 1):
        md.append(ours[ln - 1])
    md.append('```')"""

assert old in s
s = s.replace(old, new)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')