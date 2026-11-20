# 03 — `LyraClient.Target.cs` 逐行详解：一个 Target 到底在干嘛

> **定位**：上一篇（`02`）把 12 个 `.cs` 全过了一遍，但一口气太多，反而抓不住重点。这一篇只啃**一个文件**——`Source/LyraClient.Target.cs`（全工程最短的 Target，17 行）。读懂它，10 个 Target.cs 就全懂了。

---

## 一、先回答最大的疑问：这个文件根本不在游戏里

`LyraClient.Target.cs` **不会被编译进游戏**，运行时你永远见不到它的代码。它是写给 **UBT（Unreal Build Tool，虚幻构建工具）** 看的"**订单**"。

> 💡 **场景记忆**：把它想成你去奶茶店点单——
> - 奶茶本身（游戏逻辑）是 `Source/LyraGame/` 里的 `.h/.cpp`
> - **这张 Target.cs 是"点单纸"**：要中杯（Client 类型）、加珍珠（LyraGame 模块）
> - UBT 是店员，照着点单纸做，做出你指定的那杯

UBT 只在**编译 / 打包**那一刻读它。所以改它必须重新编译才生效。

---

## 二、什么时候会用到"LyraClient"这个 Target

| 你做的操作 | 用的是哪个 Target | 产出 |
|---|---|---|
| 在编辑器里 F5 开发 | `LyraEditor` | UnrealEditor.exe |
| 打包一个"发给玩家的安装包" | **`LyraClient`** | 玩家的客户端 exe |
| 部署线上专用服务器 | `LyraServer` | 无画面的服务器程序 |
| 打包单机测试 | `LyraGame` | 完整游戏进程 |

> 💡 **场景记忆**：你要上架 Steam 的"游戏本体"就是**用 `LyraClient` 打出来的**——它决定了玩家拿到的是一个"纯客户端"，不含服务器逻辑，省体积也防作弊（作弊者不能在自己机器上偷开服务器）。

---

## 三、整文件逐行解剖

### 原文件 + 右侧批注

```
 LyraClient.Target.cs（17 行）                      │ 一句话解释
 ══════════════════════════════════════════════════┼════════════════
 1 // Copyright Epic Games, Inc. All Rights Reserved│ 版权头，无用可忽略
 2                                                 │
 3 using UnrealBuildTool;                           │ 引入 UBT 的类库
 4 using System.Collections.Generic;                │ 引入 C# 集合库
 5                                                 │
 6 public class LyraClientTarget : TargetRules      │ ★ 类名 = 文件名，
 7 {                                               │   必须继承 TargetRules
 8   public LyraClientTarget(TargetInfo Target)     │ 构造函数：UBT 造这个
 9     : base(Target)                               │   对象时进来执行
10   {                                             │
11     Type = TargetType.Client;                    │ ★★ 核心：声明这是
12                                                 │   "客户端"目标
13     ExtraModuleNames.AddRange(                   │ ★ 打包时要编哪些
14       new string[] { "LyraGame" });              │   模块 → LyraGame
15                                                 │
16     LyraGameTarget.ApplySharedLyraTargetSettings(│ 调用共享设置方法
17       this);                                     │   （定义在 Game
18   }                                             │    目标里，见 02）
19 }                                               │
 ══════════════════════════════════════════════════┴════════════════
```

### 三句话背下这 4 个关键点

| 行 | 关键代码 | 干嘛的 |
|---|---|---|
| 6 | `class LyraClientTarget : TargetRules` | 类名必须 = `文件名去掉.cs`，UBT 靠这个名字找到它 |
| 11 | `Type = TargetType.Client` | 声明构建类型（最重要的一行） |
| 13-14 | `ExtraModuleNames.Add("LyraGame")` | 让这个程序把 `LyraGame` 模块编进来 |
| 16 | `LyraGameTarget.ApplySharedLyraTargetSettings(this)` | 套用 Lyra 统一的编译规矩 |

> 有没有发现：**它自己一行"游戏逻辑"都没写**。因为 Target.cs 的职责从来不是"做什么功能"，而是"**这个 exe 该按什么类型、带哪些模块、用哪套配置去编译**"。

---

## 四、拆开每个词，都配一个能记住的场景

### ① `TargetRules`（基类）—— UBT 的"表格模板"

TargetRules 是 UBT 提供的一个 C# 基类，里面预定义了上百个可配置属性（`Type`、`ExtraModuleNames`、`bUseChecksInShipping`…）。你的 Target 继承它 = 领了一张空白订单，往里面填自己的选项。

> 💡 **场景**：UBT 启动时会去 `Source/` 下扫描**所有继承 TargetRules 的类**，把它们列成"可以编的程序清单"——这也是为什么类名要和文件名一致，不然 UBT 找不到。

### ② `Type = TargetType.Client` —— 整个文件最重要的一句

`Type` 决定**产物的性质**。UE 共 5 种，Lyra 用了 4 种：

| TargetType | 产物 | 典型用途 | 特点 |
|---|---|---|---|
| `Game` | 完整游戏进程 | 单机/测试/ListenServer | 客户端逻辑 + 可开房 |
| `Client` | **纯客户端** | **发售给玩家** | 只连别人开的服，不跑服务器逻辑 |
| `Server` | 专用服务器 | 云主机 | 无画面无输入，跑权威逻辑 |
| `Editor` | 编辑器 | 开发 | 游戏 + 编辑器工具合体 |
| `Program` | 独立工具 | 命令行小工具 | Lyra 没用 |

> 💡 **场景记忆**：同样一份代码，`Type=Game` 打出来能"自己建房自己玩"；`Type=Client` 打出来只能"进别人建的房"。玩家版一般用 Client——体积小、服务器逻辑不发出去（更安全）。

### ③ `ExtraModuleNames.Add("LyraGame")` —— 我要这个模块

Target 是"壳"，模块才是"肉"。这行说：编我这个 exe 时，把 `LyraGame` 模块（= `Source/LyraGame/` 整目录的代码）也编译进去，否则启动时没有主模块。

> 💡 **场景记忆**：`01` 篇里 `.uproject` 的 `Modules` 声明"项目有哪些模块"，这里的 `ExtraModuleNames` 声明"**这个目标具体用哪些模块**"。将来你若拆了第二个游戏模块 `MyMod`，想让客户端也带上它，就在这行数组里再加一个 `"MyMod"`。

### ④ `LyraGameTarget.ApplySharedLyraTargetSettings(this)` —— 别重复造轮子

共享规矩（编译版本、警告等级、安全加固、自动开关 GameFeature 等几十项）都集中写在 `LyraGameTarget` 里的一个 static 方法里（`02` 篇第五节讲过）。这里只需一行调用，就全部套用。

> 若不这么干，10 个 Target 每个都复制粘贴那几十行设置，改一处要改十处——这就是"共享设置 + 继承"的工程意义。

---

## 五、它和别的 Target 什么关系（一张图）

```
                ┌────────────────────────────────────┐
                │ LyraGameTarget（最全的样板）         │
                │ Type=Game                          │
                │ 内含共享方法 ApplySharedSettings()  │
                └───────┬────────────────────────────┘
                        │ 被以下 Target 调用/继承
        ┌───────────────┼────────────────┐
        ▼               ▼                ▼
 LyraClientTarget  LyraServerTarget  LyraEditorTarget
 Type=Client       Type=Server        Type=Editor
 带 LyraGame 模块   带 LyraGame 模块    带 LyraGame+LyraEditor
                                       +RemoteSession 插件
```

而 **CustomConfig 变体**（`LyraGameSteamTarget`、`LyraClientSteamTarget`？——实际 Lyra 只给 Game/Server 做了变体）继承上面对应类，再加一行 `CustomConfig="Steam"` 换平台配置，见 `02` 篇。

> 注意观察：**`LyraClient.Target.cs` 不是被继承的根，而是一个"独立小文件"**——它不继承 `LyraGameTarget` 的类，只是**调用**了它上面的静态方法。分清"继承类"（EOS/Steam 变体那样）与"调用共享方法"（Client/Editor/Server 那样），就不乱了。

---

## 六、最后：一个流程图式的"打包时它怎么被用"

```
你在打包 UI 选 LyraClient（或命令行传 -Target=LyraClient）
        │
        ▼
UBT 扫描 Source/ 找到 class LyraClientTarget
        │
        ▼
执行构造函数：Type=Client
        │
        ▼
ExtraModuleNames → 把 LyraGame 模块拉进来编
        │
        ▼
ApplyShared... → 套上 Lyra 统一规矩（含自动带/不带哪些 GameFeature）
        │
        ▼
产出：玩家客户端 exe（不含服务器逻辑）
```

---

## 七、对照你仓库 `code`：你也有这俩文件

你的 `code/Source/` 根目录本来就有：

```csharp
// code.Target.cs       Type = Game        ← 对应 LyraGameTarget
// codeEditor.Target.cs Type = Editor      ← 对应 LyraEditorTarget
```

你现在还没写 `Client/Server`，因为还没有需要分发的线上场景。等哪天要"打包给朋友玩 / 开专服"，就在 `Source/` 里照着 `LyraClient.Target.cs` 复制两个：一个 `Type=Client`、一个 `Type=Server`，各加 `ExtraModuleNames` 带上你的 `code` 模块即可——这就是 Lyra 这套写法的全部秘密。

> **本篇一句话**：`LyraClient.Target.cs` = 告诉 UBT"**给我编一个只有客户端逻辑、带上 LyraGame 模块、套用 Lyra 统一规矩的 exe**"，仅此而已。
