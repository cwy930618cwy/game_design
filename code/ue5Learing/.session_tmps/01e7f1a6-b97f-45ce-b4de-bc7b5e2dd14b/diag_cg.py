# -*- coding: utf-8 -*-
import io

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'


def spans(path):
    res = []
    with io.open(path, 'r', encoding='utf-8', newline='') as f:
        lines = f.read().split('\n')
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
            res.append((start, j + 1, j + 1 - start))
            i = j + 1
        else:
            i += 1
    return res


e = spans(ENGINE)
o = spans(RESTORE)
print('engine blocks:', len(e))
print('restore blocks:', len(o))
for a, b in zip(e, o):
    flag = '' if a[2] == b[2] else '  <<< MISMATCH'
    print('eng %d-%d (%d)  |  ours %d-%d (%d)%s' % (a[0], a[1], a[2], b[0], b[1], b[2], flag))