# 08 — 模板残留：`LyraGame.h` / `LyraGame.cpp`（**删掉，不是新建**）

> 对象：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.h`（101 B）
> 　　　`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.cpp`（356 B）
> 对照：`E:\ue5\LyraStarterGame\Source\LyraGame\`（**根下没有这两个文件**）
> **结论：Lyra 源根本没有这两个文件，是建工程模板的残留，删掉。**
> 📌 **你已经在上一轮执行完了**（实测两个文件都不存在了），这份 md 留作记录。

---

## 一、这两个文件原来长什么样

```cpp
// ===== LyraGame.h（101 B / 6 行）=====
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
```

```cpp
// ===== LyraGame.cpp（356 B / 7 行）=====
// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGame.h"

// 注意：主游戏模块的注册（IMPLEMENT_PRIMARY_GAME_MODULE）已统一放在
// LyraGameModule.cpp 中，这里不能再重复定义，否则会重复定义 InitializeModule()。
// 本文件是从旧 Tenkaichi.cpp 改名遗留的占位文件，可按需删除。
```

---

## 二、来历：模板自动生成的

```
建工程时选「C++ 空白模板」
   ↓
UE 自动生成 Source\<工程名>\<工程名>.h + <工程名>.cpp
   ↓
最初是 Tenkaichi.h / Tenkaichi.cpp
   ↓
改模块名时（Tenkaichi → LyraGame）顺手把文件名也改了
   ↓
现在是 LyraGame.h / LyraGame.cpp  ← 但你根本不需要它们
```

---

## 三、Lyra 源里到底有什么（实查）

```
E:\ue5\LyraStarterGame\Source\LyraGame\  根目录（7 个文件）：
├─ LyraGame.Build.cs         1805 B
├─ LyraGameModule.cpp         358 B   ← 模块入口只在这里
├─ LyraGameplayTags.cpp      6344 B
├─ LyraGameplayTags.h        3051 B
├─ LyraLogChannels.cpp        881 B
├─ LyraLogChannels.h          486 B
└─ README.md                  198 B

不存在：
✗ LyraGame.h
✗ LyraGame.cpp
```

**Lyra 的模块入口只有 `LyraGameModule.cpp` 一个文件**（20 行 / 358 B），没有 `LyraGame.h` / `LyraGame.cpp` 这一对。

---

## 四、为什么必须删（4 条）

### 理由 1：Lyra 源没有 → 违反「零改名 + 一比一」

讲义铁律：类名 / 文件名 / 目录名全部保持源项目原名。Lyra 没有这两个文件，我们也不该有。

### 理由 2：全工程 0 处引用

```
搜索 #include "LyraGame.h"  →  0 处（除 LyraGame.cpp 自己第 3 行）
```

删掉不影响任何代码。

### 理由 3：留着会绊住后面

| 隐患 | 说明 |
|---|---|
| 删 `.h` 后 `.cpp` 编不过 | `.cpp` 第 3 行 `#include "LyraGame.h"` 会找不到头文件 |
| 与 `LyraGameModule.cpp` 混淆 | 新手会以为「模块入口是 LyraGame.cpp」 |
| UBT 白编一个空目标文件 | 增加编译时间 |

### 理由 4：你自己已经在文件里写了「可按需删除」

`.cpp` 第 5-7 行那段中文注释就是证据 —— 你早就察觉了。

---

## 五、删除流程（3 步）

### 第 1 步：关掉所有开着的东西

```
① 关闭 UE 编辑器
② 关闭 Rider / Visual Studio
```

不关的话文件被占用，删不掉。

### 第 2 步：删掉这 2 个文件

```powershell
Remove-Item "E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.h"   -Force
Remove-Item "E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.cpp" -Force
```

### 第 3 步：清中间产物（**关键，不做会报错**）

```powershell
Remove-Item "E:\ue5\game_design\code\Tenkaichi\Intermediate" -Recurse -Force -EA SilentlyContinue
Remove-Item "E:\ue5\game_design\code\Tenkaichi\Binaries"     -Recurse -Force -EA SilentlyContinue
Remove-Item "E:\ue5\game_design\code\Tenkaichi\.vs"          -Recurse -Force -EA SilentlyContinue
Remove-Item "E:\ue5\game_design\code\Tenkaichi\.idea"        -Recurse -Force -EA SilentlyContinue
```

**为什么必须清**：UBT 的 `Intermediate\Build\` 里存着「要编译哪些文件」的清单。删了源文件但清单还在，下次编译会报：

```
error : Unable to find source file 'LyraGame.cpp'
```

> ✅ 这四个目录全是**可再生的中间产物**，删了不影响源码，下次打开工程自动重建。

---

## 六、验证（已通过）

```powershell
$src = "E:\ue5\game_design\code\Tenkaichi\Source"
"LyraGame.h   存在: " + (Test-Path "$src\LyraGame\LyraGame.h")
"LyraGame.cpp 存在: " + (Test-Path "$src\LyraGame\LyraGame.cpp")
Get-ChildItem "$src\LyraGame" -File | ForEach-Object { "  " + $_.Name }
```

**实测结果（2026-11-23）**：

```
LyraGame.h   存在: False     ✅
LyraGame.cpp 存在: False     ✅

Source\LyraGame\ 下现有：
  LyraGame.Build.cs
  LyraGameModule.cpp
```

**本轮验收通过。**

---

## 七、删完之后 `Source\` 长这样

```
Source\
├─ LyraGame.Target.cs            ← 02 轮
├─ LyraEditor.Target.cs          ← 03 轮
├─ LyraGame\
│  ├─ LyraGame.Build.cs          ← 04 轮
│  └─ LyraGameModule.cpp         ← 05 轮
└─ LyraEditor\
   ├─ LyraEditor.Build.cs        ← 06 轮（占位，S22 换正式版）
   └─ LyraEditor.cpp             ← 07 轮（占位，S22 换正式版）
```

**6 个文件**。接下来 09~12 轮要新建 4 个：

```
Source\LyraGame\
├─ LyraLogChannels.h        ← 09 轮（已发讲义）
├─ LyraLogChannels.cpp      ← 10 轮（下一轮）
├─ LyraGameplayTags.h       ← 11 轮
└─ LyraGameplayTags.cpp     ← 12 轮
```

---

## 八、为什么这份讲义没有「复制文件」那一件

两件套（总 md + 复制文件 md）是给「**要新建的文件**」用的，有代码可抄。

「要删除的文件」没有代码可抄，所以这份 md 只有一件事：**告诉你怎么删**。

这是整条复刻路线里唯二的两个「反向操作」（另一个是开工时删掉的 `Source\Tenkaichi\` 目录）。

---

## 九、一句话

**`LyraGame.h` / `LyraGame.cpp` = 建工程时 C++ 空白模板留下的，Lyra 源里没有，全工程 0 处引用。已删除，验收通过。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.h`（原 6 行 / 101 B，**现已删除**）
- `E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.cpp`（原 7 行 / 356 B，**现已删除**）
- `E:\ue5\LyraStarterGame\Source\LyraGame\`（根目录实查 7 个文件，**无** `LyraGame.h` / `LyraGame.cpp`）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp:1-20`（Lyra 真正的模块入口，20 行 / 358 B）
- `E:\ue5\LyraStarterGame\Source\LyraGame\README.md`（Lyra 对该目录的说明文件）