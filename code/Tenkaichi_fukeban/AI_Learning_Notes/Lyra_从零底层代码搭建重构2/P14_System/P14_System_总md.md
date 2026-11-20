# P14 System 收口 —— 游戏全局单例与资产管理（总 md）

> **定位**：本文件是 P14（System 收口）的**总入口 + 全景地图**。目标：理解并一比一还原 Lyra 在 `Source/LyraGame/System/` 底下的"全局单例 + 资产管理 + 网络复制图"。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\`（27 个文件）
> 命名映射：`Lyra`→`Tenkaichi`，其余一字不改（铁律 24）。

---

## 〇、为什么先建这个总 md（铁律 35）

P14 是一个**独立教学内容**（一大块：AssetManager + GameInstance + ReplicationGraph 收口类），不能直接跳到教 `.h`/`.cpp`。先建本总 md（全景 + 依赖 + 教学顺序），**停下等确认**。

> 上一节 P13（GameModes）已全部教完落盘。P14 是**解开 P13 大量挂起项的关键**（AssetManager、GameSession 都在这一节）。

---

## 一、这一节解决什么问题（先讲为什么）

P13 讲了"一场游戏怎么开局"，但还缺几个**全局收口**的东西：

1. **资产管理**（AssetManager）：谁统一负责加载/缓存游戏资产？Lyra 的 `ULyraAssetManager` 是引擎 `UAssetManager` 的游戏定制版——全局单例，负责"默认 Pawn 数据""游戏全局数据""启动时批量加载"。
2. **游戏实例**（GameInstance）：谁在游戏启动时做全局初始化？`ULyraGameInstance` 是引擎 `UGameInstance` 的游戏定制版——整个游戏进程的生命周期管理者。
3. **网络复制图**（ReplicationGraph）：谁决定"哪些 Actor 要复制给哪些客户端"？`ULyraReplicationGraph` 是 Lyra 自定义的复制图，优化大规模场景的网络带宽。

> **类比**：P13 的 GameMode/GameState 是"一场戏"的导演和记分板；P14 的 AssetManager/GameInstance/ReplicationGraph 是"整个剧组"的后勤（道具库管理员、制片人、对讲机调度员）——全局只有一个，从头到尾服务。

---

## 二、全景地图（Lyra System 目录 27 个文件）

### 已教（P02 System 叶子阶段 ✅）

| 类 | 状态 |
|----|------|
| `GameplayTagStack` | ✅ P02 |
| `LyraActorUtilities` | ✅ P02 |
| `LyraSystemStatics` | ✅ P02 |
| `LyraDevelopmentStatics` | ✅ P02 |
| `LyraGameData` | ✅ P02 |

### 已教（本阶段 P14，教程已迁移至本目录 ✅）

| 类 | 状态 |
|----|------|
| `LyraGameEngine` | ✅ P14.0（重构版1 阶段三教的，教程 `P14.0_TenkaichiGameEngine_*` 已迁移） |
| `LyraAssetManagerStartupJob` | ✅ P14.1 |

### 待教（本阶段 P14）

| 组 | 类 | 干什么的 |
|----|----|---------|
| **收口类** | `LyraAssetManager` | 资产管理单例（默认 Pawn/游戏数据/启动加载） |
| | `LyraAssetManagerStartupJob` | 启动加载任务（追踪启动进度） |
| | `LyraGameData` | 全局游戏数据（伤害/治疗/动态 tag 的 GE） |
| | `LyraGameInstance` | 游戏实例（全局生命周期 + 主玩家控制器） |
| | `LyraGameSession` | 游戏会话（`AGameSession` 定制版） |
| **复制图** | `LyraReplicationGraph` | 自定义网络复制图（41.8KB 大文件） |
| | `LyraReplicationGraphSettings` | 复制图配置 |
| | `LyraReplicationGraphTypes` | 复制图类型定义 |
| | `LyraSignificanceManager` | 重要性管理器（决定复制优先级） |

---

## 三、依赖分析（铁律 30/31）

我读了关键 `.h` + `.cpp`，结论如下：

### 能独立编译的（`.h` 依赖干净）

| 类 | `.h` 依赖 | 状态 |
|----|----------|------|
| `GameData` | `Engine/DataAsset.h` + `UGameplayEffect`（前置） | ✅ 零挂起（`.h` 已落盘，`.cpp` 的 `Get()` 挂 AssetManager） |
| `AssetManagerStartupJob` | 引擎 `FStreamableHandle` | ✅ 零挂起 |
| `GameSession` | 引擎 `AGameSession` | ✅ 零挂起 |
| `SignificanceManager` | 引擎 `USignificanceManager` | ✅ 零挂起 |

### 断点/依赖较重的

| 类 | 依赖 | 处理 |
|----|------|------|
| `AssetManager` | `GameData` ✅ + `PawnData` ✅ + `AssetManagerStartupJob`（同组）+ GAS GameplayCueManager（P05 ✅） | ⚠️ 需按依赖顺序 |
| `GameInstance` | `CommonGameInstance`（S1 ✅）+ `PlayerController`（P12 ✅）+ CommonUser/CommonSession（S1 ✅） | ✅ 基本可教 |
| `ReplicationGraph` | 引擎 `UReplicationGraph` + 各复制节点类 | ⚠️ 大文件，拆细 |

---

## 四、教学顺序建议（按依赖自下而上）

| 顺序 | 类 | 依赖 | 处理 |
|------|----|------|------|
| P14.0 | `GameEngine` | 引擎 | ✅ 已教（重构版1 阶段三迁移，`P14.0_TenkaichiGameEngine_*`） |
| P14.1 | `AssetManagerStartupJob` | 引擎 | ✅ 已教（独立，被 AssetManager 依赖） |
| P14.2 | `GameData` | 引擎 + `UGameplayEffect` | ✅ 已教（P02.2，`.h` 落盘；`.cpp` 的 `Get()` 挂 AssetManager，登记 ⑮） |
| P14.3 | `AssetManager` | StartupJob + GameData + PawnData + GAS | 🟡 待教（核心收口类，解开 P13.5-a/b + P13.7-d + ⑮） |
| P14.4 | `GameSession` | 引擎 | ⬜ 待教（解开 P13.7-b） |
| P14.5 | `GameInstance` | CommonGame/CommonUser + PlayerController | ✅ 基本可教 |
| P14.6 | `SignificanceManager` + `ReplicationGraphSettings` + `ReplicationGraphTypes` | 引擎 | ✅ 独立（小文件，合并教） |
| P14.7 | `ReplicationGraph` | Settings + Types + 各节点 | ⚠️ 大文件（41.8KB），拆细 |

> ⚠️ **核心现实**：P14 的 AssetManager/GameInstance 是"收口类"，会**无底拖出** GAS + GameFeatures + CommonGame 等。但按死结处理铁律：**死结没关系，代码照写 + 注释挂起 + 登记备忘录 + 继续推进**。

---

## 五、🎯 本阶段能解开的挂起项（备忘录对账）

P14 教完后，能解开大量 P13/P12 的挂起项：

| 挂起项 | 依赖 | 解开时机 |
|--------|------|---------|
| P13.5-a/b（SetCurrentExperience/StartExperienceLoad） | AssetManager | P14.3 教完 |
| P13.7-b（GameSessionClass） | GameSession | P14.4 教完 |
| P13.7-d（HandleMatchAssignment + GetDefaultPawnData） | AssetManager | P14.3 教完 |
| ⑮（GameData::Get） | AssetManager | P14.3 教完 |
| ㉘（HealthComponent DamageSelfDestruct） | GameData | P14.2 已教（GameData 已就位，可解） |
| ㉓-c（AbilitySystemComponent Anim 段） | P16 | 不在本阶段 |

> 这些在对应类教完后，我会逐个提醒你解开（铁律 34）。

---

## 六、待确认

1. P14 范围按 **收口类（AssetManager/GameInstance/GameSession/StartupJob）+ 复制图（ReplicationGraph 系列）** 教，OK 吗？（`GameData` 已在 P02 教过，本阶段只补 `.cpp` 的 `Get()`）
2. 教学顺序按 **P14.1 StartupJob → P14.3 AssetManager → P14.4 GameSession → P14.5 GameInstance → P14.6 复制图小文件 → P14.7 ReplicationGraph** 走，OK 吗？
3. 断点类按铁律 33/35 **代码照写 + 注释挂起 + 登记备忘录**，OK 吗？
4. 确认后我**逐个类**教：每个类先建自己的总 md → 停 → 教 `.h` → 停 → 教 `.cpp`（铁律 20/29/35）。

---

## 七、源码依据

- 收口类：`LyraAssetManager.h/.cpp`、`LyraAssetManagerStartupJob.h/.cpp`、`LyraGameData.h/.cpp`、`LyraGameInstance.h/.cpp`、`LyraGameSession.h/.cpp`
- 复制图：`LyraReplicationGraph.h/.cpp`、`LyraReplicationGraphSettings.h/.cpp`、`LyraReplicationGraphTypes.h`、`LyraSignificanceManager.h/.cpp`
- 你的工程：`Source/TenkaichiGame/System/`（目录已存在，含 6 个已教文件）