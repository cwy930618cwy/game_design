# P07 Teams —— 阶段总 md

> **定位**：P07 Teams 阶段的**目录/路线图**（这一节有哪些类、各依赖什么、先教谁）。
> 具体某一个类怎么教，由**该类自己的总 md** 负责（铁律 35：上级总 md 不能替代子类总 md）。
>
> **最后更新**：2026-10-29（P07.9 三件套教完并落盘 + **P07.10 总 md 已建，停等确认**）

---

## 一、这一节要解决什么

P05 搭好 GAS、P06 把 GAS 接到角色上。现在角色会放技能、有血量、会死了。
但 **Lyra 是对战游戏**，马上要面对一个问题：**"这一枪打的是队友还是敌人？"**

- 伤害计算要知道"instigator 和 target 是不是一伙的"（`CanCauseDamage`）
- UI 要知道"这个玩家该显示成什么颜色"（`TeamDisplayAsset`）
- 角色/Pawn/PlayerState 都得能回答"你几队的"（`TeamAgentInterface`）
- 中途换队，所有相关显示要自动刷新（委托广播）

一句话：**P07 = 给游戏加"阵营"这一层身份，并回答"谁跟谁一伙"**。

---

## 二、全景：Lyra `Teams/` 文件夹 22 个文件（11 个类）

对照 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Teams\`：

| 编号 | Lyra 文件 | 工程文件 | 教学 md | 状态 |
|------|----------|---------|---------|------|
| **P07.1** | `LyraTeamAgentInterface` | `TenkaichiTeamAgentInterface.h/.cpp` ✅ | `P07.1_*_总md/_h/_cpp.md` | ✅ 已教已落盘 |
| **P07.2** | `LyraTeamDisplayAsset` | `TenkaichiTeamDisplayAsset.h/.cpp` ✅ | `P07.2_*_总md/_h/_cpp.md` | ✅ 已教已落盘（① 待解） |
| **P07.3** | `LyraTeamInfoBase` | `TenkaichiTeamInfoBase.h/.cpp` ✅ | `P07.3_*_总md/_h/_cpp.md` | ✅ 已教已落盘（② 待解） |
| **P07.4** | `LyraTeamPublicInfo` | `TenkaichiTeamPublicInfo.h/.cpp` ✅ | `P07.4_*_总md/_h/_cpp.md` | ✅ 已教已落盘（③ 待解） |
| **P07.5** | `LyraTeamPrivateInfo` | `TenkaichiTeamPrivateInfo.h/.cpp` ✅ | `P07.5_*_总md/_h/_cpp.md` | ✅ 已教已落盘（零挂起） |
| **P07.6** | `LyraTeamSubsystem` | `TenkaichiTeamSubsystem.h/.cpp` ✅ | `P07.6_*_总md/_h/_cpp.md` | ✅ 已教已落盘（④ 待解） |
| **P07.7** | `LyraTeamStatics` | `TenkaichiTeamStatics.h/.cpp` ✅ | `P07.7_*_总md/_h/_cpp.md` | ✅ 已教已落盘（⑤ 待解） |
| **P07.8** | `LyraTeamCheats` | ✅ 已落盘 | `P07.8_*_总md/_h/_cpp.md` | ✅ **已教已落盘（零挂起）→ ④ 钥匙就位，链式 ④②⑤①③㉕ 全解** |
| **P07.9** | `AsyncAction_ObserveTeam` | 🟡 `.h` 已落盘 / `.cpp` 待落盘 | `P07.9_AsyncAction_ObserveTeam_总md/_h/_cpp.md` | ✅ **三件套已教（2026-10-28）**，`.h`/`.cpp` 均已落盘，零挂起 |
| **P07.10** | `AsyncAction_ObserveTeamColors` | ❌ 未落盘 | `P07.10_AsyncAction_ObserveTeamColors_总md.md`（已建） | ✅ **三件套已教完（2026-10-29）**，`.h` 已落盘（文件名已修正），`.cpp` 待落盘（⑤ 已解 → 零挂起） |
| **P07.11** | `LyraTeamCreationComponent` | ❌ 未落盘 | `P07.11_TenkaichiTeamCreationComponent_总md.md`（已建） | ✅ **三件套已教完（2026-10-29）**，待落盘（依赖 **P13 GameModes**：`.h` 零挂起 / `.cpp` 挂 5 处，已登记 **㉛**） |

> **工程现状**：`Teams\` 已落盘 **16 / 22** 文件（8 类），**缺 6 文件（3 类）** = 本阶段要教的。
> **Teams 目录 `#if 0` 已清零**（2026-10-27 链式全解），Lyra 前缀 0 处。

### 教学 md 清单（2026-10-27 重命名后）

| 编号 | 文件 |
|------|------|
| 节总览 | `P07_Teams_总md.md`（本文件） |
| **全局总结** | **`P07_一张图看懂_全局总结.md`**（反晕眩版：球赛类比 + 广播链 + 11 类职责 + 混淆点） |
| P07.1 | `P07.1_TenkaichiTeamAgentInterface_总md/_h/_cpp.md` |
| P07.2 | `P07.2_TenkaichiTeamDisplayAsset_总md/_h/_cpp.md` |
| P07.3 | `P07.3_TenkaichiTeamInfoBase_总md/_h/_cpp.md` + `P07.3-9_TeamInfo三件套_小结.md` |
| P07.4 | `P07.4_TenkaichiTeamPublicInfo_总md/_h/_cpp.md` |
| P07.5 | `P07.5_TenkaichiTeamPrivateInfo_总md/_h/_cpp.md` |
| P07.6 | `P07.6_TenkaichiTeamSubsystem_总md/_h/_cpp.md` |
| P07.7 | `P07.7_TenkaichiTeamStatics_总md/_h/_cpp.md` |
| P07.8 | `P07.8_TenkaichiTeamCheats_总md/_h/_cpp.md` |
| P07.9 | `P07.9_AsyncAction_ObserveTeam_总md/_h/_cpp.md` |

> 📌 **命名规范（铁律 37）**：统一 `P07.x_Tenkaichi类名_总md/_h/_cpp`，下划线分隔，类从 `.1` 起编。
> 旧编号（`4.1.x` / `4.2.x` / `4.4.x` / `4.5.x`）**已全部作废**（2026-10-27 重命名完成）。

---

## 三、依赖分析（P07 全阶段，铁律 30/31）

统一方向依赖树（被依赖在下）：

```
ULyraTeamCreationComponent（P07.11，最重）
   │ 依赖
   ├── UGameStateComponent（引擎 ModularGameplay 插件 ①，只讲不拆）
   │    源码：D:\ue5\Epic Games\UE_5.6\Engine\Plugins\Runtime\ModularGameplay\...\GameStateComponent.h
   ├── P13 GameModes：ExperienceDefinition / ExperienceManagerComponent ⚠️ 未就位
   └── ATenkaichiGameMode::OnGameModePlayerInitialized 委托 ⚠️ 未就位（LyraGameMode.h:72）
   └── ATenkaichiPlayerState（P12 ✅ 已落盘）
        └─ IsOnlyASpectator() 是引擎 APlayerState 自带（PlayerState.h:362），✅ 无需补

ULyraAsyncAction_ObserveTeamColors（P07.10）
   │ 依赖
   ├── TeamAgentInterface（P07.1 ✅）
   ├── TeamStatics（P07.7 ✅，⑤ 已解）
   └── TeamSubsystem（P07.6 ✅，④ 已解）

ULyraTeamCheats（P07.8）
   │ 依赖
   ├── UCheatManagerExtension（引擎 ✅）
   └── TeamSubsystem（P07.6 ✅，④ 已解）

ULyraAsyncAction_ObserveTeam（P07.9）
   │ 依赖
   ├── UCancellableAsyncAction（引擎 ✅）
   └── TeamAgentInterface（P07.1 ✅）→ 零挂起

UTenkaichiTeamSubsystem（P07.6，核心枢纽）
   │ 依赖
   ├── UWorldSubsystem（引擎 ✅）
   ├── TeamInfoBase / PublicInfo / PrivateInfo / DisplayAsset（P07.3~P07.5 ✅）
   ├── ATenkaichiPlayerState（P12 ✅ 已落盘）
   ├── UAbilitySystemGlobals（P05 ✅ 已落盘）
   └── UTenkaichiTeamCheats（P07.8 ✅ 已落盘 → ④ 已解）

UTenkaichiTeamStatics（P07.7）→ ✅ ⑤ 已解
ATenkaichiTeamInfoBase（P07.3）→ ✅ ② 已解
ATenkaichiTeamPublicInfo（P07.4）→ ✅ ③ 已解
UTenkaichiTeamDisplayAsset（P07.2）→ ✅ ① 已解
ITenkaichiTeamAgentInterface（P07.1，最底，纯接口）
```

### 依赖就位情况

| 依赖 | 状态 |
|------|------|
| P05 `TenkaichiAbilitySystemGlobals` / ASC | ✅ 已落盘 |
| P06 `TenkaichiPawn`（实现 TeamAgentInterface） | ✅ 已落盘 |
| P12 `TenkaichiPlayerState`（`GetTeamId()`/`SetGenericTeamId()`/`GetStatTagStackCount()`） | ✅ 已落盘 |
| 第 0 步 `TenkaichiLogChannels`（`LogTenkaichiTeams` 通道） | ✅ 已落盘 |
| **P07.8 `TenkaichiTeamCheats`** | ✅ **已落盘（2026-10-27）→ ④ 已解** |
| **P13 GameModes**（`ExperienceManagerComponent` / `ExperienceDefinition` / `GameMode`） | ❌ 未落盘（P07.11 挂起源，需 `#if 0` 挂起登记 ㉛） |
| `APlayerState::IsOnlyASpectator()` | ✅ **引擎自带**（`PlayerState.h:362`），工程无需补 |
| `UCheatManagerExtension` | ✅ 引擎自带（`CheatManager.h:71`），只讲不拆（教学分级 ①） |
| `UGameStateComponent` | ✅ 引擎 ModularGameplay 插件自带，只讲不拆（教学分级 ①） |

---

## 四、🔓 阶段开头必做：翻注释备忘录（铁律 34）

P07 是 **Teams 链式挂起的解锁时机**。台账 `教学经验新对话必看\注释备忘录.md` 相关条目：

| 条目 | 内容 | 阻塞点 | 状态 |
|------|------|--------|------|
| ~~**④**~~ | `TenkaichiTeamSubsystem.cpp` 整个 `.cpp`（449 行 `#if 0`） | 只剩 `TenkaichiTeamCheats.h`（P07.8 已落盘） | ✅ **已解（2026-10-27）** |
| ~~**②**~~ | `TenkaichiTeamInfoBase.cpp` 整个 `.cpp` | ④ 的实现 | ✅ **已解（2026-10-27）** |
| ~~**③**~~ | `TenkaichiTeamPublicInfo.cpp` 整个 `.cpp` | ② 的实现 | ✅ **已解（2026-10-27）** |
| ~~**⑤**~~ | `TenkaichiTeamStatics.cpp` 整个 `.cpp` | ④ 的实现 | ✅ **已解（2026-10-27）** |
| ~~**①**~~ | `TenkaichiTeamDisplayAsset.cpp` 的 `PostEditChangeProperty` 段 | ④ | ✅ **已解（2026-10-27）** |
| ~~**㉕**~~ | `TenkaichiDamageExecution.cpp` `Execute_Implementation` 函数体 | P05.5 ✅ + ④ `CanCauseDamage` | ✅ **已解（2026-10-27）** |
| ~~⑦~~ | `HeroComponent.h`+`.cpp` | P05 ✅ + **P15 GameFeatures** | ⏸ 继续挂（本阶段不解） |

> ✅ **链式全解完成（2026-10-27）**：P07.8（TeamCheats）落盘 → ④ → ②⑤① → ③ → ㉕ 全部放开。
> **`Teams\` 目录 `#if 0` 已清零**，Lyra 前缀 0 处（顺带把 7 处 `LyraPS` → `TenkaichiPS`）。
> 这条链一解，**P05 的伤害执行真正闭环**（`CanCauseDamage` 可链接）。

---

## 五、教学顺序（铁律 20/29/35）

每个类都走「**自己的总 md → `.h` → `.cpp`**」三步，一步一反馈。

| 序 | 内容 | 状态 |
|----|------|------|
| 1 | P07.1 `TenkaichiTeamAgentInterface` | ✅ 三件套已建，已落盘，零挂起 |
| 2 | P07.2 `TenkaichiTeamDisplayAsset` | ✅ 三件套已建，已落盘，✅ **① 已解** |
| 3 | P07.3 `TenkaichiTeamInfoBase` | ✅ 三件套已建，已落盘，✅ **② 已解** |
| 4 | P07.4 `TenkaichiTeamPublicInfo` | ✅ 三件套已建，已落盘，✅ **③ 已解** |
| 5 | P07.5 `TenkaichiTeamPrivateInfo` | ✅ 三件套已建，已落盘，零挂起 |
| 6 | P07.6 `TenkaichiTeamSubsystem` | ✅ 三件套已建，已落盘，✅ **④ 已解** |
| 7 | P07.7 `TenkaichiTeamStatics` | ✅ 三件套已建，已落盘，✅ **⑤ 已解** |
| **8** | **P07.8 `TenkaichiTeamCheats`** | ✅ **三件套已建，已落盘（零挂起）→ ④②⑤①③㉕ 链式全解** |
| 9 | P07.9 `AsyncAction_ObserveTeam` | ✅ **三件套已建 + `.h`/`.cpp` 均已落盘（2026-10-28）→ `.cpp` 待落盘** |
| 10 | P07.10 `AsyncAction_ObserveTeamColors` | ✅ **三件套已教完（2026-10-29）**，`.h` 已落盘（**文件名已从 `UAsyncAction...` 修正为 `AsyncAction...`**），`.cpp` 待落盘（⑤ 已解 → 零挂起） |
| 11 | P07.11 `TenkaichiTeamCreationComponent` | ✅ **三件套已教完（2026-10-29）**，`.h`/`.cpp` 待落盘（依赖 P13：`.h` 零挂起 / `.cpp` 挂 5 处，已登记 **㉛**） |

---

## 六、几个要记住的点

1. **P07 = 给引擎的 `FGenericTeamId` 套一层 Lyra 的"队伍管理层"**。引擎只给了一个 `uint8` 队号，Lyra 补上"队伍信息实体 + 外观资产 + 查询子系统 + 变更广播"。
2. **Public vs Private 双信息 Actor**：`TeamPublicInfo` 全客户端可见（含外观资产），`TeamPrivateInfo` 只服务器可见（存内部数据）。同一个队号挂两个 Actor，是"信息分区"的标准做法。
3. **`TeamSubsystem` 是枢纽**：所有"谁哪队""能不能伤害"都过它。它也是 P05 伤害执行的最后一块拼图。
4. **`UTenkaichiTeamCheats` 虽小却是钥匙**：`TeamSubsystem::Initialize` 里就 `NewObject` 它 → 没有它整个 `.cpp` 编译不过。这就是"小类卡大类"的典型。
5. **`UGameStateComponent`**（引擎 ModularGameplay 插件 ①，只讲不拆）：P07.11 的基类，提供 `GetGameStateChecked<T>()` 类型转换模板。
6. **死结处理铁律**：P07.11 依赖 P13 GameModes 未就位 → **代码照写 + `#if 0` 挂起 + 登记 ㉛ + 继续推进**，不停下纠结。

---

## 七、下一步

**教 P07.10 `AsyncAction_ObserveTeamColors`** —— 同族的"监听队伍**颜色**"版本。
它比 P07.9 多依赖 P07.7 `TeamStatics`（⑤ 已解）+ P07.6 `TeamSubsystem`（④ 已解），
所以同样**零挂起、写完即通**。

- ✅ P07.9 `AsyncAction_ObserveTeam` 三件套已教完（2026-10-28），`.h`/`.cpp` **均已落盘**
- ✅ **P07.10 三件套已教完**（2026-10-29）：总 md + `.h`（55 行，**已落盘，文件名已修正**）+ `.cpp`（109 行）
- ⬜ P07.10 的 **`.cpp` 待你落盘**（文件名必须是 `AsyncAction_ObserveTeamColors.cpp`，不带 `U`）

- ✅ **P07.11 三件套已教完**（2026-10-29）：总 md + `.h`（64 行）+ `.cpp`（190 行）
- ⬜ P07.11 的 **`.h`/`.cpp` 待你落盘**（文件名必须带 `Tenkaichi` 前缀）

> 🎉 **P07 Teams 阶段全部完成（2026-10-29）**
> - 11 个类 / 22 文件 **教学 + 代码落盘** 齐平
> - **符号级对账通过**：`P07_全阶段符号级对账报告.md`
>   （22/22 文件齐平、0 处 Lyra 残留、0 处符号缺口、0 处擅自添加、0 处编译级 Bug）
> - 剩 **㉛ 5 处 P13 挂起**（全在 `TenkaichiTeamCreationComponent.cpp`）
>
> ➡️ **下一步：进 P08 Inventory（16 文件）**，按铁律 35 先建第一个类自己的总 md。

挂起方案（死结处理铁律 + 铁律 33）：**`.h` 零挂起可完整照写**（7 个类型全是前置声明）；
**`.cpp` 只挂 P13 相关 5 处**，其余 8 个函数全部照写、一字不少。已登记注释备忘录 **㉛**，待 P13 就位后解开。

> 📌 本阶段最后一个是 P07.11 `TenkaichiTeamCreationComponent`（依赖 P13 → `#if 0` 挂起 + 登记 ㉛）。
