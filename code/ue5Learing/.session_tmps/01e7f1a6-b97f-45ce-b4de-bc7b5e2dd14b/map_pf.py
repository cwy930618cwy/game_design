# -*- coding: utf-8 -*-
import io, json

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h'
OUT     = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap.json'

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    eng = f.read().split('\n')
with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')


def key(line):
    t = line.strip()
    if t == '':
        return ('blank',)
    if t.startswith('//'):
        return ('comment',)
    if t.startswith('/*') or t.startswith('*'):
        return ('block',)
    return ('code', t.split('//')[0].strip())


emap = {}
idx = 0
for i, eline in enumerate(eng, start=1):
    k = key(eline)
    while idx < len(ours):
        if key(ours[idx]) == k:
            emap[i] = idx + 1
            idx += 1
            break
        idx += 1

print('mapped', len(emap), 'of', len(eng))
missing = [i for i in range(1, len(eng) + 1) if i not in emap]
print('missing:', missing[:20])
with io.open(OUT, 'w', encoding='utf-8') as f:
    json.dump(emap, f)