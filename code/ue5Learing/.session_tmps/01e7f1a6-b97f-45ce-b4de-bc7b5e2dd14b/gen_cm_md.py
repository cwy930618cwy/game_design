# -*- coding: utf-8 -*-
import io, json

RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\ConsoleManager.h'
MD      = r'E:\ue5\game_design\code\ue5Learing\AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\答疑文件\15_ConsoleManager.h逐行注释版.md'
MAP     = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap_cm.json'

with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')
with io.open(MAP, 'r', encoding='utf-8') as f:
    emap = {int(k): v for k, v in json.load(f).items()}

segments = []
segments.append(('一、版权头 + 头文件保护', 1, 3,
 ['`#pragma once` —— 头文件防重复包含'],
 [('第 1 行', '版权头'),
  ('第 3 行', '头文件保护')],
 '像信封上的封口：拆过一次就不会再拆第二次。'))

segments.append(('二、说明注释（为什么留这个空壳）', 5, 6,
 ['说明：本文件只是转手 include 接口头文件',
  '说明：如果所有 include 都改成 `IConsoleManager`，本文件可以删掉',
  '说明：或者把它挪进 `Private`，再把 `FConsoleManager` 类放回来'],
 [('第 5 行', '为什么留这个空壳'),
  ('第 6 行', '两种未来处理方式')],
 '像老店门口的指路牌：店搬了，牌子还留着告诉你新地址。'))

segments.append(('三、转手 include', 7, 7,
 ['`#include "HAL/IConsoleManager.h"` —— 真正的接口定义在这里',
  '`IWYU pragma: export` —— 告诉 Include-What-You-Use 工具「本头文件对外导出它」'],
 [('第 7 行', '转手 include + IWYU 导出标记')],
 '像转接插头：插上这个，等于插上 IConsoleManager。'))

md = []
md.append('# 15 — `HAL/ConsoleManager.h` 逐行注释版（超详细）')
md.append('')
md.append('> 依据：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\ConsoleManager.h`（**7 行**）')
md.append('> 你新建文件：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\ConsoleManager.h`')
md.append('> **✅ 本节可独立编译**——纯转手 include，无自身代码。')
md.append('> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。')
md.append('')
md.append('---')
md.append('')
md.append('## 一、这个文件解决什么问题（为什么）')
md.append('')
md.append('这是一个**纯转发头文件**（forwarding header）。它自己一行代码都没有，只做一件事：`#include "HAL/IConsoleManager.h"`。')
md.append('')
md.append('**为什么要有它？** 历史原因。早期 UE 把控制台管理器的实现类 `FConsoleManager` 放在 `ConsoleManager.h` 里，后来重构成接口 `IConsoleManager` 放进了 `IConsoleManager.h`。为了不破坏所有已经写了 `#include "HAL/ConsoleManager.h"` 的旧代码，就留了这个空壳转发。')
md.append('')
md.append('引擎注释自己都说了：**「如果所有 include 都改成 IConsoleManager，本文件可以删掉」**。')
md.append('')
md.append('**说个场景**：你在老代码里看到 `#include "HAL/ConsoleManager.h"`，想找 `IConsoleManager` 的定义 —— 打开这个文件发现是空的，别慌，它只是把你转手送到 `IConsoleManager.h`。')
md.append('')
md.append('> 🔑 **一句话**：`ConsoleManager.h` = **一个「转接插头」**，插上它等于插上 `IConsoleManager.h`。')
md.append('')
md.append('---')
md.append('')
md.append('## 二、逐段代码（每行上方都有中文注释）')
md.append('')

for title, s, e_, new_elems, rows, analogy in segments:
    md.append('### %s（第 %d~%d 行）' % (title, s, e_))
    md.append('')
    md.append('```cpp')
    start = emap[s - 1] + 1 if (s - 1) in emap else 1
    end = emap[e_]
    for ln in range(start, end + 1):
        md.append(ours[ln - 1])
    md.append('```')
    md.append('')
    md.append('**本段新元素**')
    md.append('')
    for ne in new_elems:
        md.append('- ' + ne)
    md.append('')
    md.append('**这段在干什么**')
    md.append('')
    md.append('| 行 | 干什么 |')
    md.append('|---|---|')
    for a, b in rows:
        md.append('| %s | %s |' % (a, b))
    md.append('')
    md.append('> 类比：' + analogy)
    md.append('')

md.append('---')
md.append('')
md.append('## 三、速查：这段代码在干什么（一句话版）')
md.append('')
md.append('| 段 | 行 | 一句话 |')
md.append('|---|---|---|')
for title, s, e_, _, _, _ in segments:
    name = title.split('、', 1)[1]
    md.append('| %s | %d~%d | %s |' % (name, s, e_, name))
md.append('')
md.append('---')
md.append('')
md.append('## 四、C++ / 预处理器语法小抄（本文件用到的）')
md.append('')
md.append('| 语法 | 含义 | 例子 |')
md.append('|---|---|---|')
md.append('| `#pragma once` | 头文件防重复包含 | `#pragma once` |')
md.append('| `#include "路径"` | 引入头文件 | `#include "HAL/IConsoleManager.h"` |')
md.append('| `// IWYU pragma: export` | 告诉 IWYU 工具「本头文件对外导出它」 | `#include "X.h" // IWYU pragma: export` |')
md.append('')
md.append('---')
md.append('')
md.append('## 五、我们这版 vs 引擎的差异')
md.append('')
md.append('| 项 | 引擎 | 我们 | 说明 |')
md.append('|---|---|---|---|')
md.append('| 代码行 | 7 行 | 11 行 | 多出的是每行上方的中文注释 |')
md.append('| 非注释代码行 | 2 行 | 2 行 | **逐字一致** |')
md.append('| 注释语言 | 英文 | 中文 | 全部译成中文 |')
md.append('| `IWYU pragma: export` | 保留原文 | 保留原文 + 中文说明 | 工具指令，不翻译 |')
md.append('')
md.append('---')
md.append('')
md.append('## 待确认')
md.append('')
md.append('1. 还原文件已生成：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\ConsoleManager.h`（11 行）')
md.append('2. 脚本验收：`codeOurs=2 / codeEngine=2 / diff=0`')
md.append('3. 确认后下一个：`HAL\\CriticalSection.h`（1149 字节）')
md.append('')
md.append('---')
md.append('')
md.append('## 源码依据')
md.append('')
md.append('- 引擎：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\ConsoleManager.h`（7 行）')
md.append('- 还原：`E:\\ue5\\game_design\\code\\ue5Learing\\ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\ConsoleManager.h`（11 行）')
md.append('- 总 md：`AI_Learning_Notes\\从零复刻ue5\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\02_ConsoleManager.h_总md.md`')

with io.open(MD, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(md))
print('md lines =', len(md))