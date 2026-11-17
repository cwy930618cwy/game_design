# 06 — `PixelFormat.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h`（**527 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\PixelFormat.h`
> **✅ 本节可独立编译**——只 include `CoreTypes.h` + `Misc/EnumClassFlags.h`，无工程内类依赖。

---

## 一、这个文件解决什么问题（为什么）

显卡里的纹理不是「一张图」，而是一段**按固定规则排布的字节**。`PixelFormat.h` 就是 UE 的**像素格式字典**：它用 `EPixelFormat` 枚举给每一种排布方式编了号（`PF_B8G8R8A8` = 每像素 4 字节 BGRA、`PF_DXT1` = 4×4 块压缩…），再配一组 `IsXxx()` 判断函数和 `FPixelFormatInfo` 描述表，让上层代码能问「这个格式是不是压缩的 / 是不是浮点 / 占多少字节」。

**说个场景**：你导入一张 PNG，引擎要决定「在显存里怎么存它」——存成 `PF_B8G8R8A8`（不压缩、4 字节/像素）还是 `PF_DXT5`（压缩、1 字节/像素）？这个决定就是 `EPixelFormat` 的一个值。之后渲染管线、RHI、Cook 打包全靠这个值查表。

> 🔑 **一句话**：`PixelFormat.h` = **GPU 像素排布的「身份证 + 说明书」**，枚举是身份证号，`FPixelFormatInfo` 是说明书。

---

## 二、内部结构全景

```
PixelFormat.h（527 行）
├─ 第 1~11 行    版权头 + #pragma once + 2 个 include + PF_MAX 防重定义
├─ 第 13 行      前置声明 enum class EPixelFormatCapabilities
├─ 第 15~112 行  ① enum EPixelFormat : uint8（94 个值，PF_Unknown=0 … PF_MAX=94）
├─ 第 113~207 行 ② FOREACH_ENUM_EPIXELFORMAT(op) 宏（92 个 op(...)，供反射/序列化遍历）
├─ 第 210~222 行 ③ enum class EPixelFormatChannelFlags : uint8（R/G/B/A 位标志）
├─ 第 224~263 行 ④ enum class EPixelFormatCapabilities : uint32（纹理/缓冲/UAV 能力位）
├─ 第 268 行     ⑤ UE_PIXELFORMAT_TO_UINT8 宏
├─ 第 270~445 行 ⑥ 格式判断函数（10 个）
│    ├─ IsETCBlockCompressedPixelFormat
│    ├─ IsASTCBlockCompressedTextureFormat
│    ├─ IsBlockCompressedFormat
│    ├─ IsAnyBlockCompressedPixelFormat
│    ├─ IsHDR / IsInteger / IsFloatFormat
│    ├─ IsDepthOrStencilFormat / IsStencilFormat
│    ├─ IsDXTCBlockCompressedTextureFormat
│    └─ RequiresBlock4Alignment
├─ 第 452~502 行 ⑦ struct FPixelFormatInfo（格式描述表 + 6 个尺寸计算函数 + 2 个块数计算函数）
├─ 第 510~516 行 ⑧ GetPixelFormatString / GetPixelFormatFromString
├─ 第 519 行     ⑨ extern GPixelFormats[PF_MAX]（全局格式表）
└─ 第 521~527 行 ⑩ namespace UE::PixelFormat::HasCapabilities
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `PixelFormat.h` | 无（只 include `CoreTypes.h` + `Misc/EnumClassFlags.h`） | ✅ 能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `uint8` / `uint32` / `uint64` / `int32` / `TCHAR` |
| `Misc/EnumClassFlags.h` | `ENUM_CLASS_FLAGS` 宏（给 enum class 生成位运算） |

---

## 四、怎么教（推进节奏）

**先建本总 md（已建）→ 停，等确认 → 建 `答疑文件\12_PixelFormat.h逐行注释版.md`（每行都有中文注释）→ 生成还原文件 → 脚本验收 → 停 → 教 `CoreGlobals.h`（854 行）**。

---

**下一步**：你确认这个总 md 没问题（懂了/继续），我就建 `PixelFormat.h` 的超详细注释版讲义并生成还原文件。