# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg_md.py'
s = io.open(p, encoding='utf-8').read()

s = s.replace('assert len(eng) == len(ours), (len(eng), len(ours))', '')

old = """    md.append('```cpp')
    for ln in range(s, e_ + 1):
        md.append(ours[ln - 1])
    md.append('```')"""
new = """    md.append('```cpp')
    start = emap[s - 1] + 1 if (s - 1) in emap else 1
    end = emap[e_]
    for ln in range(start, end + 1):
        md.append(ours[ln - 1])
    md.append('```')"""
assert old in s
s = s.replace(old, new)

old2 = """with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\\n')"""
new2 = """with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\\n')
with io.open(r'E:\\ue5\\game_design\\code\\ue5Learing\\.session_tmps\\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\\emap_cg.json', 'r', encoding='utf-8') as f:
    emap = {int(k): v for k, v in json.load(f).items()}"""
assert old2 in s
s = s.replace(old2, new2)

io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')