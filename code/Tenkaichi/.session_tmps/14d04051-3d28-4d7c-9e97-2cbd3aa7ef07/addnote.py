# -*- coding: utf-8 -*-
p = r'E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra_从零底层代码搭建重构2\P06_Character\04_角色体系_总md(重构版1归档).md'
with open(p, encoding='utf-8') as f:
    c = f.read()

if '【归档说明】' in c:
    print('already')
else:
    note = (
        '> ⚠️ **【归档说明】2026-10-27**：本文件是**重构版1（L1~L7 分层）**的遗留总 md，'
        '`阶段四：角色与玩家` 编号体系已废止。\n'
        '> 重构版2 中本层内容已拆分到 **`P06_Character\\`**，编号统一为 `P06.1`~`P06.8`'
        '（见 `P06_Character_总md.md`）。\n'
        '> **本文件仅作历史参照，不再作为推进依据**（铁律 02：以重构版2 目录为准）。\n\n'
    )
    c = note + c
    with open(p, 'w', encoding='utf-8', newline='\n') as f:
        f.write(c)
    print('added')
print('\n'.join(c.split('\n')[:6]))
