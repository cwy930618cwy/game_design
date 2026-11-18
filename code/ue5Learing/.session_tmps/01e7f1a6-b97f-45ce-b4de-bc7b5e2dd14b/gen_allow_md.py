# -*- coding: utf-8 -*-
import io, json

RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\HAL\AllowTCHAR.h'
MD      = r'E:\ue5\game_design\code\ue5Learing\AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\Public\HAL\答疑文件\14_AllowTCHAR.h逐行注释版.md'
MAP     = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap_allow.json'

with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')
with io.open(MAP, 'r', encoding='utf-8') as f:
    emap = {int(k): v for k, v in json.load(f).items()}

segments = []
segments.append(('一、版权头 + 使用说明', 1, 6,
 ['`HEADER_UNIT_SKIP` —— 标记本文件不单独作为编译单元',
  '说明：必须在 `HAL/HideTCHAR.h` 之后 include'],
 [('第 1 行', '版权头'),
  ('第 3~4 行', '使用说明：必须跟在 HideTCHAR.h 后'),
  ('第 6 行', 'HEADER_UNIT_SKIP 标记')],
 '像药盒上的「请与另一半配合服用」：单独吃没用。'))

segments.append(('二、配对校验', 8, 12,
 ['`UE_HIDETCHAR_INCLUDED` —— `HideTCHAR.h` 打下的标记',
  '`#undef` —— 取消宏定义',
  '`#error "..."` —— 编译期直接报错并输出消息'],
 [('第 8 行', '检查标记是否存在'),
  ('第 9 行', '存在 → 取消标记（解封）'),
  ('第 11 行', '不存在 → 编译报错')],
 '像验票：没买 HideTCHAR 的票，就不让进 AllowTCHAR 的门。'))

segments.append(('三、首次 include：记下系统 TCHAR', 14, 24,
 ['`UE_HIDETCHAR_INCLUDED_FIRST` —— 「已首次 include」标记',
  '`using UE_SYSTEM_TCHAR = TCHAR;` —— 把当前 TCHAR 类型记成 UE_SYSTEM_TCHAR',
  '`!PLATFORM_TCHAR_IS_UTF8CHAR` —— 平台 TCHAR 不是 UTF8CHAR 的模式'],
 [('第 14~15 行', '首次 include 才执行'),
  ('第 17~22 行', '说明注释'),
  ('第 23 行', '记下系统 TCHAR')],
 '像拍照存档：在把 TCHAR 还回去之前，先拍一张「系统原生 TCHAR 长什么样」。'))

segments.append(('四、UTF8CHAR 模式下撤销宏', 26, 28,
 ['`PLATFORM_TCHAR_IS_UTF8CHAR` —— 平台把 TCHAR 定义成 UTF8CHAR',
  '`#undef TCHAR` —— 撤销 `HideTCHAR.h` 里的 `#define TCHAR UE_SYSTEM_TCHAR`'],
 [('第 26 行', '判断是否 UTF8CHAR 模式'),
  ('第 27 行', '撤销 TCHAR 宏'),
  ('第 28 行', '条件编译结束')],
 '像把借出去的名字收回来：第三方头用完了，TCHAR 这个名字还给 UE。'))

md = []
md.append('# 14 — `HAL/AllowTCHAR.h` 逐行注释版（超详细）')
md.append('')
md.append('> 依据：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\AllowTCHAR.h`（**28 行**）')
md.append('> 你新建文件：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\AllowTCHAR.h`')
md.append('> **⚠️ 本节不能独立编译**——必须与 `HAL/HideTCHAR.h` 成对 include。')
md.append('> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。')
md.append('')
md.append('---')
md.append('')
md.append('## 一、这个文件解决什么问题（为什么）')
md.append('')
md.append('第三方库（比如 `<tchar.h>`、某些 SDK 头）里会用到 `TCHAR` 这个名字。UE 在 `PLATFORM_TCHAR_IS_UTF8CHAR` 模式下把 `TCHAR` 定义成 `char`（UTF-8），而 Windows 系统头把 `TCHAR` 定义成 `wchar_t` —— **两者撞名**，编译直接报重定义。')
md.append('')
md.append('UE 的解法是「**临时把 TCHAR 藏起来**」：')
md.append('')
md.append('```cpp')
md.append('#include "HAL/HideTCHAR.h"        // 把 TCHAR 改名成 UE_SYSTEM_TCHAR（藏起来）')
md.append('#include <tchar.h>                // 第三方头看到的是 UE_SYSTEM_TCHAR，不冲突')
md.append('#include "HAL/AllowTCHAR.h"       // 把 TCHAR 还回来（恢复成 UE 的定义）')
md.append('```')
md.append('')
md.append('`AllowTCHAR.h` 就是**「还回来」的那一半**：它撤销 `HideTCHAR.h` 的 hack，并把系统原生 TCHAR 记成 `UE_SYSTEM_TCHAR` 备用。')
md.append('')
md.append('**说个场景**：你接入一个第三方图像库，它的头文件里有 `TCHAR szPath[MAX_PATH];`。直接 include 会和 UE 的 `TCHAR` 冲突。解法就是上面那三行包裹。')
md.append('')
md.append('> 🔑 **一句话**：`AllowTCHAR.h` = **TCHAR 的「解封开关」**，必须和 `HideTCHAR.h` 成对使用，单独用会 `#error`。')
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
md.append('| `#ifdef X` | 如果宏 X 已定义 | `#ifdef UE_HIDETCHAR_INCLUDED` |')
md.append('| `#if !defined(X)` | 如果宏 X 未定义 | `#if !defined(UE_HIDETCHAR_INCLUDED_FIRST)` |')
md.append('| `#undef X` | 取消宏 X 的定义 | `#undef TCHAR` |')
md.append('| `#define X` | 定义宏 X（无值） | `#define UE_HIDETCHAR_INCLUDED_FIRST` |')
md.append('| `#error "msg"` | 编译期报错并输出 msg | `#error "HAL/AllowTCHAR.h included without..."` |')
md.append('| `using A = B;` | 类型别名（C++11） | `using UE_SYSTEM_TCHAR = TCHAR;` |')
md.append('')
md.append('---')
md.append('')
md.append('## 五、我们这版 vs 引擎的差异')
md.append('')
md.append('| 项 | 引擎 | 我们 | 说明 |')
md.append('|---|---|---|---|')
md.append('| 代码行 | 28 行 | 49 行 | 多出的是每行上方的中文注释 |')
md.append('| 非注释代码行 | 12 行 | 12 行 | **逐字一致** |')
md.append('| 注释语言 | 英文 | 中文 | 全部译成中文 |')
md.append('| `#error` 消息 | 英文原文 | 保留英文 | 编译期输出，属运行时字符串，不翻译 |')
md.append('')
md.append('---')
md.append('')
md.append('## 待确认')
md.append('')
md.append('1. 还原文件已生成：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\AllowTCHAR.h`（49 行）')
md.append('2. 脚本验收：`codeOurs=12 / codeEngine=12 / diff=0`')
md.append('3. 确认后下一个：`HAL\\ConsoleManager.h`（333 字节）')
md.append('')
md.append('---')
md.append('')
md.append('## 源码依据')
md.append('')
md.append('- 引擎：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\AllowTCHAR.h`（28 行）')
md.append('- 配套：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\HideTCHAR.h`（35 行）')
md.append('- 还原：`E:\\ue5\\game_design\\code\\ue5Learing\\ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\AllowTCHAR.h`（49 行）')
md.append('- 总 md：`AI_Learning_Notes\\从零复刻ue5\\Engine\\Source\\Runtime\\Core\\Public\\HAL\\01_AllowTCHAR.h_总md.md`')

with io.open(MD, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(md))
print('md lines =', len(md))