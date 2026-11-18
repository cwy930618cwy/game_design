# -*- coding: utf-8 -*-
import io, os, json

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h'

pre = {}
rep = {}


def add(line, text):
    pre[line] = '// ' + text


add(8,  '如果 UE_HIDETCHAR_INCLUDED 已定义，说明前面确实 include 过 HideTCHAR.h。')
add(9,  '取消该标记，表示 TCHAR 的隐藏期结束。')
add(10, '否则，没 include 过 HideTCHAR.h 就来了。')
add(11, '直接编译报错：AllowTCHAR.h 必须与 HideTCHAR.h 成对使用。')
add(12, '条件编译结束。')
add(14, '如果还没定义过 UE_HIDETCHAR_INCLUDED_FIRST，即这是第一次成对 include。')
add(15, '打上已首次 include 标记，保证下面这段只执行一次。')
add(23, '把当前的 TCHAR 类型记成 UE_SYSTEM_TCHAR，即系统原生 TCHAR。')
add(24, '条件编译结束。')
add(26, '如果平台把 TCHAR 定义成 UTF8CHAR，即 TCHAR 不是宽字符。')
add(27, '取消 TCHAR 的宏定义，恢复成 Platform.h 里的定义。')
add(28, '条件编译结束。')

rep[1]  = '// 版权所有 Epic Games, Inc. 保留所有权利。'
rep[3]  = '// 本头文件预期在 HAL/HideTCHAR.h 之后被 include，详见那个头文件里的注释。'
rep[4]  = '// 原文第二行：for more details.'
rep[6]  = '// HEADER_UNIT_SKIP 标记：不直接单独 include，只能成对使用。'
rep[17] = '\t// 我们希望 UE_SYSTEM_TCHAR 在 TCHAR 第一次被定义之后就存在，即使在'
rep[18] = '\t// !PLATFORM_TCHAR_IS_UTF8CHAR 模式下也要有。'
rep[19] = '\t//'
rep[20] = '\t// TCHAR 预期会在 HideTCHAR/AllowTCHAR 这对包裹的第一批 include 里被定义，'
rep[21] = '\t// 所以我们在撤销 hack 之前，先用它来定义我们自己的类型，如果它存在的话。'
rep[22] = '\t//'

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

with io.open(r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap_allow.json', 'w', encoding='utf-8') as f:
    json.dump(emap, f)