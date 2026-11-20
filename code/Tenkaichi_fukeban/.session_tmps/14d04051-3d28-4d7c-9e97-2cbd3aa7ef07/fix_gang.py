# -*- coding: utf-8 -*-
p = r'E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra_从零底层代码搭建重构2\00_学习路径总纲_按文件夹自下而上.md'
with open(p, encoding='utf-8') as f:
    lines = f.read().split('\n')

# 定位 P06 段起始
start = None
for i, l in enumerate(lines):
    if l.startswith('### P06 Character'):
        start = i
        break
assert start is not None, 'P06 段未找到'

# 找段尾（下一个 ### 或文件尾，保留空行）
end = start + 1
while end < len(lines) and not lines[end].startswith('### '):
    end += 1

new = [
    '### P06 Character（✅ 16/16 全落盘，教学全收官，2026-10-27 更新）',
    '',
    '| 子项 | 类 / 工程文件 | 状态 |',
    '|------|--------------|------|',
    '| P06.0 | 阶段总 md | ✅ 已建（`P06_Character\\P06_Character_总md.md`） |',
    '| P06.1 | `TenkaichiPawnData` | ✅ 三件套已建 + 已落盘，零挂起 |',
    '| P06.2 | `TenkaichiPawnExtensionComponent` | ✅ 三件套已建 + 已落盘，⑥ **已解开** |',
    '| P06.3 | `TenkaichiHeroComponent` | ✅ 三件套已建 + 已落盘，⏸ ⑦ 待 P15 GameFeatures |',
    '| P06.4 | `TenkaichiPawn` | ✅ 三件套已建 + 已落盘，零挂起 |',
    '| P06.5 | `TenkaichiCharacter` | ✅ 三件套已建 + 已落盘，⑧ **已解开**，仅剩 ㉙ 待 P14 |',
    '| P06.6 | `TenkaichiCharacterWithAbilities` | ✅ 三件套已建 + 已落盘，⑨ **已解开** |',
    '| P06.7 | `TenkaichiCharacterMovementComponent` | ✅ 三件套已建 + 已落盘，零挂起 |',
    '| P06.8 | `TenkaichiHealthComponent` | ✅ 三件套已建 + 已落盘，⏸ ㉘ 待 P14 AssetManager |',
    '',
    '> 📌 **2026-10-27 命名规范化**：P06 目录下 23 个 md 已从旧编号（`4.3.x`/`4.6.x`/`4.7.x`/`4.8.x`）统一重命名为 `P06.x_Tenkaichi类名_总md/_h/_cpp`（铁律 37）。`04_角色体系_总md.md` 归档为 `04_角色体系_总md(重构版1归档).md`。',
    '> 📌 **挂起状态**：㉖ ㉗ ⑥ ⑧ ⑨ ㉒ ㉔ **已全部解开**；仍挂起 ⑦（P15）、㉘（P14）、㉙（P14）。',
    '> ➡️ **下一步进 P07 Teams**。',
    '',
]

lines[start:end] = new
with open(p, 'w', encoding='utf-8', newline='\n') as f:
    f.write('\n'.join(lines))

print('done, replaced lines %d..%d' % (start, end))
for l in lines[start:start + len(new)]:
    print(l)
