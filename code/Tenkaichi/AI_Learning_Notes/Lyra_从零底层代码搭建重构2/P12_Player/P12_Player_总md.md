# P12 Player —— 阶段总 md

> **定位**：本文件是 P12 Player（玩家类）的**阶段总 md**（为什么 + 全景 + 依赖分析 + 教学顺序）。
> 每个类另有自己的总 md + `.h` + `.cpp` 教学 md（铁律 35）。
>
> **建立时间**：2026-11-04
> **更新时间**：2026-11-05（教学顺序改为**纯数字顺序**，依赖注释挂起，不自作聪明重排）
> **教学状态**：✅ 完成 —— P12.1~P12.8 全部完成（断点挂起见注释备忘录）

---

## 一、本阶段要解决什么

P06 讲了"**角色 Pawn**"（车），P04 讲了"**相机**"（看路），P03 讲了"**输入**"（方向盘）。但一个玩家要真正玩起来，光有车 + 看路 + 方向盘还不够——还需要三个"玩家侧"的类把这一切串起来，以及几个辅助类：

| 问题 | 谁来回答 | 一句话 |
|------|----------|--------|
| **谁在操作这个角色？**（输入/相机/重生/作弊） | `ALyraPlayerController`（玩家控制器） | 玩家的大脑，司机 |
| **玩家的档案信息存哪？**（队伍/连接状态/PawnData/统计） | `ALyraPlayerState`（玩家状态） | 行驶证，全客户端可见 |
| **本地这台电脑上的"玩家"是谁？**（本地设置/音频/队伍观察） | `ULyraLocalPlayer`（本地玩家） | 车主本人，只在本地 |
| **调试指令从哪输入？**（无敌/加血/杀自己/切相机） | `ULyraCheatManager`（作弊管理器） | 后台控制台 |
| **AI 机器人怎么当"玩家"？**（Bot 也走 PlayerController 流程） | `ALyraPlayerBotController`（机器人控制器） | AI 司机 |
| **调试相机怎么开？**（自由飞行观察场景） | `ALyraDebugCameraController`（调试相机控制器） | 上帝视角 |
| **玩家从哪里出生？**（出生点选择/占用/重生） | `ALyraPlayerStart` + `ULyraPlayerSpawningManagerComponent` | 停车场车位 |

一句话：**P12 = 玩家侧的"大脑 + 档案 + 车主 + 调试 + 机器人 + 出生点"**，把 P03~P11 的输入/相机/角色/队伍/GAS 全串起来。

---

## 二、全景：8 个教学内容（16 文件）

| 编号 | 类 | 文件 | 作用 | 状态 |
|------|------|------|------|------|
| P12.1 | `ALyraPlayerController` | `.h` + `.cpp` | 玩家控制器（司机）：输入/相机/队伍/回放/作弊/自动奔跑/重生 | ✅ 已完成（`.cpp` 挂起） |
| P12.2 | `ALyraPlayerState` | `.h` + `.cpp` | 玩家状态（行驶证）：队伍/小队/连接类型/PawnData/统计标签/属性集 | ✅ 已完成（`.cpp` 挂起） |
| P12.3 | `ULyraLocalPlayer` | `.h` + `.cpp` | 本地玩家（车主）：本地/共享设置/音频设备/队伍观察 | ✅ 已完成（`.cpp` 挂起） |
| P12.4 | `ULyraCheatManager` | `.h` + `.cpp` | 作弊管理器：无敌/加血/杀自己/加标签/切相机 | ✅ 已完成（`.cpp` 整体挂起 P12-3） |
| P12.5 | `ALyraPlayerBotController` | `.h` + `.cpp` | 机器人控制器：AI 玩家，驱动队伍态度 + 重生 | ✅ 已完成（`.cpp` 2 处挂起 P12-4） |
| P12.6 | `ALyraDebugCameraController` | `.h` + `.cpp` | 调试相机控制器：自由飞行观察场景 | ✅ 已完成（`.cpp` 死结挂起 P12-3） |
| P12.7 | `ALyraPlayerStart` | `.h` + `.cpp` | 玩家出生点：占用/释放/空位检测 | ✅ 已完成（零依赖，全编译） |
| P12.8 | `ULyraPlayerSpawningManagerComponent` | `.h` + `.cpp` | 出生管理组件：从 GameMode 代理的出生点选择逻辑 | ✅ 全部完成 |

> 命名注意（铁律 24）：**所有类都有 `Lyra` 前缀 → 换成 `Tenkaichi`**（`ALyraPlayerController`→`ATenkaichiPlayerController` 等）。枚举 `ELyraPlayerStartLocationOccupancy` → `ETenkaichiPlayerStartLocationOccupancy`。

---

## 三、依赖分析（铁律 30/31）

```
P12.1 PlayerController ── 依赖 P04 CameraAssistInterface + S1 CommonPlayerController + P07 TeamAgentInterface + P05 GAS + P12.2 PlayerState + P14 HUD
P12.2 PlayerState ── 依赖 S1 ModularPlayerState + P02 GameplayTagStack + P07 TeamAgentInterface + P05 GAS(HealthSet/CombatSet) + P06 PawnData + P13 Experience
P12.3 LocalPlayer ── 依赖 S1 CommonLocalPlayer + P07 TeamAgentInterface + P18 Settings(Local/Shared) + P07 TeamSubsystem
P12.4 CheatManager ── 依赖 P12.1 PlayerController + P05 GAS + P06 Health/PawnExtension + P14 AssetManager/GameData + P18 DeveloperSettings
P12.5 PlayerBotController ── 依赖 S1 ModularAIController + P12.2 PlayerState + P13 GameMode + P05 GAS
P12.6 DebugCameraController ── 引擎 ADebugCameraController（0 项目依赖）
P12.7 PlayerStart ── 引擎 APlayerStart + GameplayTagContainer（0 项目依赖）
P12.8 PlayerSpawningManagerComponent ── 引擎 UGameStateComponent + P12.7 PlayerStart + P13 GameMode（friend）
```

**已就位 ✅：** P04 Camera（含 `CameraAssistInterface`）、P05 GAS、P06 Character、P07 Teams、P02 GameplayTagStack、S1 插件（CommonGame/ModularGameplayActors）。

**关键断点（⚠️ 注释挂起）：**

| 断点 | 依赖 | 处理 |
|------|------|------|
| `PlayerController.cpp` | P05 GAS + P14 HUD + P18 Settings + Replay + P13 GameState + CheatManager | `.cpp` 整体挂起 |
| `PlayerState.cpp` | P05 GAS(HealthSet/CombatSet) + P13 Experience + P14 GameData | `.cpp` 整体挂起 |
| `LocalPlayer.cpp` | P18 Settings(Local/Shared) | `.cpp` 整体挂起 |
| `CheatManager.cpp` | P05 GAS + P06 Health/PawnExtension + P14 AssetManager/GameData + P18 DeveloperSettings | `.cpp` 整体挂起 |
| `PlayerBotController.cpp` | P13 GameMode + P05 GAS | 仅 `ServerRestartController` + `GameModes/TenkaichiGameMode.h` include 挂起（登记 P12-4） |
| `PlayerSpawningManagerComponent.cpp` | P13 GameMode（friend）+ 出生代理 | `.cpp` 整体挂起 |

> 按死结处理铁律：**代码照写 + 注释挂起 + 登记备忘录 + 继续推进**，不停下纠结。

---

## 四、教学顺序（严格按编号顺序，铁律 02/20/29/35）

> ⚠️ **2026-11-05 修正**：教学顺序**严格按数字编号 P12.1→P12.8 顺延**，不再按"依赖轻重"重排。**依赖未就位的 `.cpp` 一律注释挂起继续推进**（死结处理铁律），绝不自作聪明换顺序。

| 序 | 编号 | 内容 | 处理 |
|----|------|------|------|
| 1 | **P12.1** | `ALyraPlayerController` | ✅ 已完成（`.cpp` 挂起） |
| 2 | **P12.2** | `ALyraPlayerState` | ✅ 已完成（`.cpp` 挂起） |
| 3 | **P12.3** | `ULyraLocalPlayer` | ✅ 已完成（`.cpp` 挂起） |
| 4 | **P12.4** | `ULyraCheatManager` | ✅ 已完成（`.cpp` 整体挂起登记 P12-3） |
| 5 | **P12.5** | `ALyraPlayerBotController` | ✅ 已完成（`.cpp` 2 处挂起登记 P12-4） |
| 6 | **P12.6** | `ALyraDebugCameraController` | ✅ 已完成（`.cpp` 死结挂起登记 P12-3） |
| 7 | **P12.7** | `ALyraPlayerStart` | ✅ 已完成（零依赖，全编译） |
| 8 | **P12.8** | `ALyraPlayerSpawningManagerComponent` | ✅ 全部完成 |

> ⚠️ 说明：**按编号顺序教**。每个类的 `.cpp` 即使依赖 P05/P06/P13/P14/P18 未就位，也**代码照 Lyra 完整写上 + `#if 0` 注释挂起**（死结处理铁律），到依赖就位那一轮再解开。

---

## 五、核心难点预读

1. **PlayerController = 司机**：不占物理位置，管"操作 + 表现"，不负责复制。只有本地玩家有它。
2. **PlayerState = 行驶证**：服务器权威，复制给所有客户端。存队伍/小队/连接状态/PawnData/统计标签。
3. **LocalPlayer = 车主**：只存在于本地，管设置（本地/共享）和音频设备。
4. **CheatManager 作弊**：`exec` 函数 + `GetOuterAPlayerController()` 拿到玩家控制器，`ServerCheat` 转发到服务器执行。
5. **PlayerBotController 机器人**：AI 也走"Controller + PlayerState"流程，队伍态度由 PlayerState 驱动（`GetGenericTeamId` 查 PlayerState）。
6. **PlayerStart 出生点**：`TryClaim` 占用 + `CheckUnclaimed` 定时释放 + `GetLocationOccupancy` 三态检测（Empty/Partial/Full）。
7. **SpawningManagerComponent**：挂在 GameState 上，用 `friend class ALyraGameMode` 让 GameMode 代理出生点选择逻辑（P13）。

---

## 六、命名对照（铁律 24）

| Lyra | Tenkaichi | 说明 |
|------|-----------|------|
| `ALyraPlayerController` | `ATenkaichiPlayerController` | ✅ 有前缀，换 |
| `ALyraReplayPlayerController` | `ATenkaichiReplayPlayerController` | ✅ 有前缀，换（PlayerController 内嵌子类） |
| `ALyraPlayerState` | `ATenkaichiPlayerState` | ✅ 有前缀，换 |
| `ULyraLocalPlayer` | `UTenkaichiLocalPlayer` | ✅ 有前缀，换 |
| `ULyraCheatManager` | `UTenkaichiCheatManager` | ✅ 有前缀，换 |
| `ALyraPlayerBotController` | `ATenkaichiPlayerBotController` | ✅ 有前缀，换 |
| `ALyraDebugCameraController` | `ATenkaichiDebugCameraController` | ✅ 有前缀，换 |
| `ALyraPlayerStart` | `ATenkaichiPlayerStart` | ✅ 有前缀，换 |
| `ULyraPlayerSpawningManagerComponent` | `UTenkaichiPlayerSpawningManagerComponent` | ✅ 有前缀，换 |
| `ELyraPlayerStartLocationOccupancy` | `ETenkaichiPlayerStartLocationOccupancy` | ✅ 有前缀，换 |

---

## 七、教学节奏（铁律 20/29/35）

1. 🟡 本阶段总 md（现在这一步）
2. ✅ **P12.1~P12.7** `PlayerController`/`PlayerState`/`LocalPlayer`/`CheatManager`/`PlayerBotController`/`DebugCameraController`/`PlayerStart`（已完成，`.cpp` 断点挂起）
3. ⬜ **P12.8**（最后一个）：`PlayerSpawningManagerComponent`

> ⚠️ 注意：
> - **严格按编号顺序教**，不按依赖轻重重排。依赖未就位的 `.cpp` → **代码照写 + 注释挂起**（死结处理铁律），到依赖就位那一步再解开。
> - 每到一个新阶段开头，先翻注释备忘录，把依赖已就位的条目提醒用户解开（铁律 34）。

---

## 八、一句话结论

P12 Player 是玩家侧的"大脑 + 档案 + 车主 + 调试 + 机器人 + 出生点"，8 个教学内容（16 文件）。依赖 P04 Camera + P05 GAS + P06 Character + P07 Teams + P02 GameplayTagStack + S1 插件（已就位）。所有 `.cpp` 硬依赖 P13 Experience / P14 System / P18 Settings（挂起），代码照写 + 注释挂起继续推进。所有类都换 `Tenkaichi` 前缀。