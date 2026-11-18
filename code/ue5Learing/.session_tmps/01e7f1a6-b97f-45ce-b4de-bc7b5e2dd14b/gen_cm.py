# -*- coding: utf-8 -*-
import io, os, json

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\ConsoleManager.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ConsoleManager.h'

pre = {}
rep = {}


def add(line, text):
    pre[line] = '// ' + text


add(3, '头文件防重复包含：同一编译单元里多次 include 只展开一次。')
add(6, '直接转手 include 接口头文件：如果所有 include 都改成 IConsoleManager，本文件可以删掉；')
add(7, '或者把它挪进 Private，再把 FConsoleManager 类放回来。')

rep[1] = '// 版权所有 Epic Games, Inc. 保留所有权利。'
rep[5] = '// 只是转手 include 接口头文件：如果所有 include 都改成 IConsoleManager，本文件可以删掉；'
rep[6] = '// 或者把它挪进 Private，再把 FConsoleManager 类放回来。'
rep[7] = '#include "HAL/IConsoleManager.h" // IWYU pragma: export（IWYU 指令：本头文件对外导出它）'

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

out = []
emap = {}
for i, line in enumerate(lines, start=1):
    if i in pre:
        out.append(pre[i])
    emap[i] = len(out) + 1
    out.append(rep.get(i, line))

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
            if diff <= 6:
                print('DIFF[%d] OURS: %s' % (i, a))
                print('DIFF[%d] ENG : %s' % (i, b))
    print('strip_tail=%s codeOurs=%d codeEngine=%d diff=%d' % (strip_tail, len(o), len(e), diff))

with io.open(r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap_cm.json', 'w', encoding='utf-8') as f:
    json.dump(emap, f)