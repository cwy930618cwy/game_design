# -*- coding: utf-8 -*-
import io

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h'
MD      = r'E:\ue5\game_design\code\ue5Learing\AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\答疑文件\12_PixelFormat.h逐行注释版.md'

with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')

# 引擎行号 -> 还原文件行号
with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    eng = f.read().split('\n')

# 重建映射：还原文件里，每个引擎行前面可能有若干注释行
# 用「引擎行内容」在还原文件里顺序匹配
emap = {}
idx = 0
for i, eline in enumerate(eng, start=1):
    while idx < len(ours):
        if ours[idx] == eline:
            emap[i] = idx + 1
            idx += 1
            break
        idx += 1

print('mapped', len(emap), 'of', len(eng))

segments = [
    ('一、版权头 + include + PF_MAX 防重定义', 1, 11,
     ['`#pragma once` —— 头文件保护，同一编译单元只展开一次',
      '`CoreTypes.h` —— 基础类型层（`uint8` / `uint32` / `uint64` / `int32` / `TCHAR`）',
      '`Misc/EnumClassFlags.h` —— 提供 `ENUM_CLASS_FLAGS` 宏',
      '`#if defined(PF_MAX)` / `#undef PF_MAX` —— 先清掉旧定义，避免枚举重定义冲突'],
     [('`#pragma once`', '防重复包含'),
      ('`#include "CoreTypes.h"`', '拿基础类型'),
      ('`#include "Misc/EnumClassFlags.h"`', '拿位运算宏'),
      ('`#undef PF_MAX`', '清旧定义')],
     '像进门先脱外套：先把可能冲突的旧 `PF_MAX` 摘掉，再摆自己的东西。'),
    ('二、前置声明 `EPixelFormatCapabilities`', 13, 13,
     ['`enum class EPixelFormatCapabilities : uint32` —— 能力位枚举的前置声明（完整定义在第 224 行）'],
     [('第 13 行', '先声明，后面 `FPixelFormatInfo` 才能引用它')],
     '像点菜前先报菜名：菜还没上，名字先占位。'),
    ('三、`enum EPixelFormat`（94 个格式）', 15, 112,
     ['`enum EPixelFormat : uint8` —— 底层 `uint8` 的**非** `enum class`（可直接当整数用）',
      '`PF_Unknown` —— 0 号，未知格式',
      '`PF_B8G8R8A8` —— 2 号，最常见的 32 位无压缩格式',
      '`PF_DXT1` / `PF_DXT5` —— 5 / 7 号，PC 端块压缩',
      '`PF_ASTC_*` —— 50~54 号，移动端块压缩',
      '`PF_MAX` —— 94 号，**哨兵值**，不是真实格式，只用来定数组大小'],
     [('第 17~111 行', '94 个格式编号，每个值 = 一种显存排布'),
      ('第 111 行 `PF_MAX = 94`', '总数哨兵，`GPixelFormats[PF_MAX]` 用它当长度')],
     '像色卡：每格一个编号，`PF_MAX` 是「一共多少格」的标签，不是颜色。'),
    ('四、`FOREACH_ENUM_EPIXELFORMAT(op)` 遍历宏', 113, 207,
     ['`FOREACH_ENUM_EPIXELFORMAT(op)` —— 把每个枚举值套一遍 `op(...)`，供反射 / 序列化 / 调试输出统一遍历',
      '行尾反斜杠 —— 宏续行符，最后一行（第 207 行）没有'],
     [('第 113 行', '宏定义开始'),
      ('第 114~207 行', '92 个 `op(...)`，顺序与枚举一致'),
      ('第 206 行 `op(PF_R8G8B8)` 加反斜杠', '引擎原文反斜杠前无空格，照抄')],
     '像点名册：名字按顺序念一遍，谁要处理就自己举手（`op` 就是那个动作）。'),
    ('五、`EPixelFormatChannelFlags`（通道位标志）', 209, 222,
     ['`enum class EPixelFormatChannelFlags : uint8` —— 哪些通道有效',
      '`R = 1 << 0` —— 第 0 位代表 R 通道',
      '`RGBA = R | G | B | A` —— 位或组合',
      '`ENUM_CLASS_FLAGS(...)` —— 给 `enum class` 生成位运算和 `EnumHasAllFlags`'],
     [('第 212~215 行', 'R/G/B/A 各占一位'),
      ('第 216~218 行', 'RG / RGB / RGBA 组合'),
      ('第 222 行', '生成位运算工具')],
     '像四个开关：可以只开一个，也可以一起开。'),
    ('六、`EPixelFormatCapabilities`（能力位标志）', 224, 263,
     ['`Texture1D` / `Texture2D` / `Texture3D` / `TextureCube` —— 能当哪种纹理',
      '`RenderTarget` / `DepthStencil` —— 能当渲染目标 / 深度模板',
      '`UAV` / `TypedUAVLoad` / `TypedUAVStore` —— 无序访问视图相关',
      '`AnyTexture` / `AllTextureFlags` / `AllBufferFlags` / `AllUAVFlags` / `AllFlags` —— 预组合掩码'],
     [('第 227~239 行', '纹理类能力位（第 1~13 位）'),
      ('第 241~246 行', '缓冲类能力位（第 14~19 位）'),
      ('第 248~250 行', 'UAV 类能力位（第 20~22 位）'),
      ('第 255~261 行', '预组合掩码，省得每次手写一串位或')],
     '像体检报告：一张表打勾，勾完还能直接拿「全部纹理项」这种套餐。'),
    ('七、`UE_PIXELFORMAT_TO_UINT8` 宏', 265, 268,
     ['`UE_PIXELFORMAT_TO_UINT8(argument)` —— 把 `EPixelFormat` 显式转 `uint8`，消除隐式转换警告'],
     [('第 268 行', '过渡期兼容宏')],
     '像翻译官：两种类型混用时，帮忙把话转成对方听得懂的。'),
    ('八、`IsETCBlockCompressedPixelFormat`', 270, 283,
     ['`PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS` —— 临时关掉「switch 没覆盖全部枚举值」警告',
      '`PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS` —— 恢复警告'],
     [('第 272 行', '关警告'),
      ('第 275~278 行', '4 个 ETC 格式'),
      ('第 281 行', '恢复警告')],
     '像临时把报警器静音：只查几个值，别一直响。'),
    ('九、`IsASTCBlockCompressedTextureFormat`', 285, 310,
     ['`FORCEINLINE` —— 强制内联，避免函数调用开销',
      '`default: return false;` —— 其余格式直接返回 false'],
     [('第 290~304 行', '15 个 ASTC 格式（普通 / HDR / RG）'),
      ('第 309 行', '恢复警告（在 `return` 之后，不可达，引擎原文如此）')],
     '像查名单：名单上有就 true，没有就 false。'),
    ('十、`IsBlockCompressedFormat`', 312, 328,
     ['（本段无新元素，全是前面见过的写法）'],
     [('第 317~323 行', 'DXT1/3/5 + BC4/5/6H/7')],
     '同上，只是换了一份名单。'),
    ('十一、`IsAnyBlockCompressedPixelFormat`', 330, 333,
     ['（本段无新元素）'],
     [('第 332 行', '三个判断取并集')],
     '像「三种证件有一个就算齐全」。'),
    ('十二、`IsHDR`', 335, 341,
     ['（本段无新元素）'],
     [('第 339~340 行', '浮点 + BC6H + ASTC HDR 系列')],
     '像判断「这张图是不是高动态范围」。'),
    ('十三、`IsInteger`', 343, 369,
     ['（本段无新元素）'],
     [('第 348~364 行', '17 个 UINT / SINT 格式')],
     '同上，名单换成整数格式。'),
    ('十四、`IsFloatFormat`', 371, 391,
     ['（本段无新元素）'],
     [('第 377~386 行', '10 个浮点格式')],
     '注意它**不含全部**浮点格式，另见 `IsHDR`。'),
    ('十五、`IsDepthOrStencilFormat`', 393, 407,
     ['（本段无新元素）'],
     [('第 398~402 行', 'D24 / DepthStencil / X24_G8 / ShadowDepth / R32F')],
     '像判断「这块内存是不是深度用的」。'),
    ('十六、`IsStencilFormat`', 409, 420,
     ['（本段无新元素）'],
     [('第 414~415 行', '只有 DepthStencil 和 X24_G8')],
     '比上一个更窄：只问「带不带模板」。'),
    ('十七、`IsDXTCBlockCompressedTextureFormat`', 422, 439,
     ['（本段无新元素）'],
     [('第 427~433 行', '与 `IsBlockCompressedFormat` 同一份名单')],
     '同一份名单，换个名字给不同调用点用。'),
    ('十八、`RequiresBlock4Alignment`', 441, 445,
     ['（本段无新元素）'],
     [('第 444 行', '直接复用 DXTC 判断')],
     '像「要不要按 4 的倍数切」——只有 BC/DXTC 要。'),
    ('十九、`struct FPixelFormatInfo`', 447, 502,
     ['`FPixelFormatInfo() = delete` —— 禁止默认构造，必须传全参数',
      '`BlockSizeX/Y/Z` —— 压缩块尺寸（未压缩格式为 1）',
      '`BlockBytes` —— 一个压缩块多少字节',
      '`NumComponents` —— 通道数',
      '`Capabilities` —— 能力位（RHI 初始化）',
      '`PlatformFormat` —— 平台原生格式号（RHI 初始化）',
      '`Supported : 1` —— 1 位位域',
      '`bIs24BitUnormDepthStencil : 1` —— 1 位位域',
      '`Get2DImageSizeInBytes` 等 6 个 —— 尺寸计算函数',
      '`GetBlockCountForWidth/Height` —— 算块数'],
     [('第 454 行', '禁默认构造'),
      ('第 455~463 行', '构造函数 9 个参数'),
      ('第 465~471 行', '7 个基础字段'),
      ('第 473~483 行', 'RHI 填充字段 + 2 个位域'),
      ('第 490~495 行', '6 个尺寸计算函数'),
      ('第 500~501 行', '2 个块数计算函数')],
     '像一张商品规格表：尺寸、重量、支持哪些功能，全写在一行里。'),
    ('二十、`GetPixelFormatString` / `GetPixelFormatFromString`', 505, 516,
     ['`GetPixelFormatString` —— 枚举转字符串（如 `PF_B8G8R8A8`）',
      '`GetPixelFormatFromString` —— 字符串转枚举（不区分大小写）'],
     [('第 510 行', '枚举转字符串'),
      ('第 516 行', '字符串转枚举')],
     '像身份证号与姓名的互查窗口。'),
    ('二十一、`GPixelFormats[PF_MAX]`', 519, 519,
     ['`GPixelFormats` —— 全局格式表，下标就是 `EPixelFormat` 的值'],
     [('第 519 行', '全局表声明')],
     '像一本字典：拿编号直接翻到那一页。'),
    ('二十二、`UE::PixelFormat::HasCapabilities`', 521, 527,
     ['`namespace UE::PixelFormat` —— C++17 嵌套命名空间写法',
      '`EnumHasAllFlags` —— 判断是否包含**全部**指定标志'],
     [('第 523 行', '函数声明'),
      ('第 525 行', '查表 + 判位')],
     '像问「这台机器支不支持这套功能」——必须全支持才算支持。'),
]

md = []
md.append('# 12 — `PixelFormat.h` 逐行注释版（超详细）')
md.append('')
md.append('> 依据：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\PixelFormat.h`（**527 行**）')
md.append('> 你新建文件：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\PixelFormat.h`')
md.append('> **✅ 本节可独立编译**——只 include `CoreTypes.h` + `Misc/EnumClassFlags.h`。')
md.append('> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。')
md.append('')
md.append('---')
md.append('')
md.append('## 一、这个文件解决什么问题（为什么）')
md.append('')
md.append('显卡里的纹理不是「一张图」，而是一段**按固定规则排布的字节**。`PixelFormat.h` 就是 UE 的**像素格式字典**：')
md.append('')
md.append('- `EPixelFormat` 枚举 = **身份证号**（`PF_B8G8R8A8` = 2 号、`PF_DXT1` = 5 号…）')
md.append('- `FPixelFormatInfo` 结构 = **说明书**（块多大、几字节、几通道、支持什么）')
md.append('- `IsXxx()` 函数 = **快速判断**（是不是压缩 / 浮点 / 深度）')
md.append('')
md.append('**说个场景**：你导入一张 PNG，引擎要决定「在显存里怎么存它」——存成 `PF_B8G8R8A8`（4 字节/像素）还是 `PF_DXT5`（压缩后约 1 字节/像素）？这个决定就是 `EPixelFormat` 的一个值，之后渲染、RHI、Cook 打包全靠它查表。')
md.append('')
md.append('> 🔑 **一句话**：`PixelFormat.h` = **GPU 像素排布的「身份证 + 说明书」**。')
md.append('')
md.append('---')
md.append('')
md.append('## 二、逐段代码（每行上方都有中文注释）')
md.append('')

for title, s, e_, new_elems, rows, analogy in segments:
    md.append('### %s（第 %d~%d 行）' % (title, s, e_))
    md.append('')
    md.append('```cpp')
    for el in range(s, e_ + 1):
        if el in emap:
            md.append(ours[emap[el] - 1])
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
md.append('## 四、C++ 语法小抄（本文件用到的）')
md.append('')
md.append('| 语法 | 含义 | 例子 |')
md.append('|---|---|---|')
md.append('| `enum X : uint8` | 普通枚举，底层类型 `uint8`，可隐式转整数 | `enum EPixelFormat : uint8` |')
md.append('| `enum class X : uint32` | 强类型枚举，不能隐式转整数 | `enum class EPixelFormatCapabilities : uint32` |')
md.append('| `1 << n` | 第 n 位置 1 | `R = 1 << 0` |')
md.append('| 位或 | 组合标志 | `RGBA = R 或 G 或 B 或 A` |')
md.append('| `ENUM_CLASS_FLAGS(X)` | 给 `enum class` 生成位运算与判位工具 | `ENUM_CLASS_FLAGS(EPixelFormatChannelFlags)` |')
md.append('| `FORCEINLINE` | 强制内联 | `FORCEINLINE bool IsHDR(...)` |')
md.append('| `X() = delete` | 禁止调用该构造函数 | `FPixelFormatInfo() = delete;` |')
md.append('| `uint8 X : 1` | 位域，只占 1 位 | `uint8 Supported : 1;` |')
md.append('| `namespace A::B` | C++17 嵌套命名空间 | `namespace UE::PixelFormat` |')
md.append('')
md.append('---')
md.append('')
md.append('## 五、我们这版 vs 引擎的差异')
md.append('')
md.append('| 项 | 引擎 | 我们 | 说明 |')
md.append('|---|---|---|---|')
md.append('| 代码行 | 527 行 | 912 行 | 多出的是每行上方的中文注释 |')
md.append('| 非注释代码行 | 451 行 | 451 行 | **逐字一致** |')
md.append('| 注释语言 | 英文 | 中文 | 全部译成中文 |')
md.append('| 行尾注释 | 英文 | 中文 | 18 处，只译注释，代码不动 |')
md.append('| 引擎错字 / 特殊空白 | `op(PF_R8G8B8)` 后无空格 | 照抄 | 不改 |')
md.append('')
md.append('---')
md.append('')
md.append('## 待确认')
md.append('')
md.append('1. 还原文件已生成：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\PixelFormat.h`（912 行）')
md.append('2. 脚本验收：`codeOurs=451 / codeEngine=451 / diff=0`（剔除行尾注释后逐字一致）')
md.append('3. 确认后本节完结，下一节教 `CoreGlobals.h`（854 行）')
md.append('')
md.append('---')
md.append('')
md.append('## 源码依据')
md.append('')
md.append('- 引擎：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\PixelFormat.h`（527 行）')
md.append('- 还原：`E:\\ue5\\game_design\\code\\ue5Learing\\ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\PixelFormat.h`（912 行）')
md.append('- 总 md：`AI_Learning_Notes\\从零复刻ue5\\Engine\\Source\\Runtime\\Core\\Public\\06_PixelFormat.h_总md.md`')

with io.open(MD, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(md))
print('md lines =', len(md))