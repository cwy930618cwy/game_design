# Lyra `System` 目录总览 —— 13 个核心类分类详解

> **定位**：拆解 `Source/LyraGame/System/` 目录下的全部类。
>
> **源码位置**：`e:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\`
>
> **一句话**：`System` 目录放的是 Lyra 的**底层基础设施**——引擎入口、资源管理、网络同步、工具库，是支撑上层玩法的"水电煤"。共 **27 个文件（约 13 个类/结构）**，可归为 **5 大类**。

---

## 一、全景分类图

```
Lyra/System/（27 文件）
│
├─ ① 引擎与入口（3）        ← 游戏运行的"起点"
│   ├─ ULyraGameEngine        引擎子类（最早期初始化）
│   ├─ ULyraGameInstance      游戏实例（全局单例，跨关卡）
│   └─ ALyraGameSession       游戏会话（一局比赛的服务器状态）
│
├─ ② 资源管理（3）          ← 加载/管理 DataAsset
│   ├─ ULyraAssetManager      资源管理器（继承 UAssetManager）★
│   ├─ FLyraAssetManagerStartupJob  启动加载任务（进度上报）
│   └─ ULyraGameData          全局数据资产（存伤害/治疗 GE 等）
│
├─ ③ 网络同步（4）          ← 多人同步优化
│   ├─ ULyraReplicationGraph         复制图（决定"谁同步给谁"）★
│   ├─ ULyraReplicationGraphSettings 复制图配置（CVar 可调）
│   ├─ LyraReplicationGraphTypes.h   复制图枚举/结构
│   └─ ULyraSignificanceManager      重要性管理器（占位子类）
│
├─ ④ 工具静态库（3）        ← 蓝图函数库
│   ├─ ULyraSystemStatics     运行时通用工具（材质/组件/资产）
│   ├─ ULyraDevelopmentStatics 开发期工具（PIE/调试专用）
│   └─ ULyraActorUtilities    Actor 工具（网络模式判断）
│
└─ ⑤ 数据结构（1）          ← 可复制的 Tag 栈
    └─ FGameplayTagStack / FGameplayTagStackContainer  带计数的 Tag 栈（支持网络增量同步）
```

---

## 二、逐类详解

### ① 引擎与入口（3 个）

这三个是"游戏从启动到一局结束"的**生命周期骨架**，都继承自引擎原生类。

#### `ULyraGameEngine : public UGameEngine`
- **角色**：引擎本身的子类，**最早期**的初始化入口。
- **做的事**：只重写了 `Init()`，在引擎启动的最底层做 Lyra 专属初始化。
- **配置**：在 `DefaultEngine.ini` 里通过 `GameEngine` 类名指定。
- **类比**：整栋大楼的"总闸"，通电那一刻就接管。

#### `ULyraGameInstance : public UCommonGameInstance`
- **角色**：**游戏实例**，跨关卡存在的全局单例（不会因为切地图而销毁）。
- **做的事**：
  - 处理用户登录初始化（`HandlerUserInitialized`）。
  - 网络加密密钥处理（`ReceivedNetworkEncryptionToken/Ack`）。
  - 客户端旅行到会话前的 URL 处理（`OnPreClientTravelToSession`）。
  - 判断能否加入请求的会话（`CanJoinRequestedSession`）。
- **类比**：大楼的"物业中心"，贯穿你在大楼里的整个活动，不管换到哪个楼层（关卡）。

#### `ALyraGameSession : public AGameSession`
- **角色**：**游戏会话**，管理"一局比赛"的服务器侧状态。
- **做的事**：
  - 禁用自动登录（`ProcessAutoLogin`）。
  - 比赛开始/结束回调（`HandleMatchHasStarted/Ended`）。
- **类比**：一局游戏的"裁判/计时器"，管这一局什么时候开始、什么时候结束。

> **三者关系**：`GameEngine`（进程级）→ 创建 `GameInstance`（跨关卡）→ 每局创建 `GameSession`（单局）。

---

### ② 资源管理（3 个）★重点

Lyra 大量用 `UPrimaryDataAsset`（前面学过的数据资产），这批类就是**负责加载和管理这些资产**的。

#### `ULyraAssetManager : public UAssetManager` ★
- **角色**：资源管理器，Lyra 加载资产的**核心枢纽**。
- **做的事**：
  - 提供 `GetAsset<T>()` / `GetSubclass<T>()` 模板方法（同步加载软引用资产）。
  - 管理全局游戏数据 `LyraGameDataPath` 和默认 Pawn 数据 `DefaultPawnData`。
  - 启动时执行 `StartupJobs`（一批加载任务），并上报进度给加载界面。
  - 初始化 GameplayCue 管理器（`InitializeGameplayCueManager`）。
  - 记录所有已加载资产（`LoadedAssets`，线程安全）。
- **配置**：在 `DefaultEngine.ini` 里通过 `AssetManagerClassName` 指定。
- **类比**：大楼的"中央仓库管理员"，你要什么资产都找他，他还管"启动时该搬哪些货进来"。

#### `FLyraAssetManagerStartupJob`（结构体，非 UObject）
- **角色**：一个"启动加载任务"，配合上面的 Manager 用。
- **做的事**：
  - 封装一个加载函数 `JobFunc` + 任务名 `JobName` + 权重 `JobWeight`。
  - 上报子步骤进度（`UpdateSubstepProgress`），喂给加载界面显示百分比。
  - 内部对进度采样做了限流（最多 60 次/秒，因为 `GetProgress()` 很贵）。
- **类比**：仓库管理员手里的"进货清单"上每一条任务。

#### `ULyraGameData : public UPrimaryDataAsset`
- **角色**：**全局游戏数据资产**（存数据的容器，不可变）。
- **存什么**：
  - `DamageGameplayEffect_SetByCaller`：伤害 GE（用 SetByCaller 传数值）。
  - `HealGameplayEffect_SetByCaller`：治疗 GE。
  - `DynamicTagGameplayEffect`：动态增删 Tag 的 GE。
- **获取**：`ULyraGameData::Get()` 静态方法，或通过 AssetManager 加载。
- **类比**：仓库里那个"全局配置箱"，装着伤害/治疗等通用配方。

> **三者关系**：`AssetManager` 启动时跑 `StartupJob` 列表，其中之一就是加载 `GameData`（里面存着各种 GE 的软引用）。

---

### ③ 网络同步（4 个）★难点

这批是**多人游戏的网络优化**核心——决定"哪些 Actor 同步给哪个客户端、以什么频率同步"。

#### `ULyraReplicationGraph : public UReplicationGraph` ★
- **角色**：**复制图**，UE 网络同步的"调度中枢"。
- **做的事**：
  - `InitGlobalActorClassSettings`：给每个 Actor 类设定复制策略。
  - `InitGlobalGraphNodes`：建立全局节点（网格节点 `GridNode`、常驻相关节点 `AlwaysRelevantNode`）。
  - `RouteAddNetworkActorToNodes`：把新 Actor 路由到合适的节点。
  - 用 `ClassRepNodePolicies` 记录每个类的路由策略。
- **关键成员**：
  - `GridNode`（`UReplicationGraphNode_GridSpatialization2D`）：按 2D 空间网格做距离剔除。
  - `AlwaysRelevantNode`：永远同步给所有人的 Actor（如 GameState）。
- **类比**：快递分拣中心，决定"哪些包裹（Actor）发往哪个站点（客户端）"。

#### 同文件里的两个自定义节点类
| 类 | 作用 |
|----|------|
| `ULyraReplicationGraphNode_AlwaysRelevant_ForConnection` | 处理"每个连接的常驻相关 Actor"，支持按流关卡可见性同步 |
| `ULyraReplicationGraphNode_PlayerStateFrequencyLimiter` | **限频**同步 PlayerState：跟踪所有但每帧只返回子集（默认 2 个/帧），优化大量玩家场景 |

#### `ULyraReplicationGraphSettings : public UDeveloperSettingsBackedByCVars`
- **角色**：复制图的**配置项**（可在编辑器/CVar 运行时调）。
- **关键配置**：
  - `bDisableReplicationGraph`：是否禁用复制图（默认 true，即默认关闭，按需开启）。
  - 快速共享路径（FastSharedPath）相关带宽/剔除距离。
  - 空间网格参数（`SpatialGridCellSize`、`SpatialBiasX/Y`）。
  - 动态 Actor 频率分桶（`DynamicActorFrequencyBuckets = 3`）。
- **类比**：分拣中心的"参数控制面板"，可调网格大小、频率等。

#### `LyraReplicationGraphTypes.h`（枚举 + 结构）
- **`EClassRepNodeMapping`**：Actor 类的路由策略枚举：
  - `NotRouted`（特殊处理，如 PlayerState 限频节点）
  - `RelevantAllConnections`（常驻相关）
  - `Spatialize_Static`（不动的，网格节点，不必每帧更新）
  - `Spatialize_Dynamic`（常动的，每帧更新）
  - `Spatialize_Dormancy`（休眠时当静态，唤醒时当动态）
- **`FRepGraphActorClassSettings`**：给某个类配置复制策略的结构（可指定蓝图类）。

#### `ULyraSignificanceManager : public USignificanceManager`
- **角色**：重要性管理器，**几乎是空的占位子类**（没加任何逻辑）。
- **作用**：预留扩展点——将来可按"重要性"决定 Actor 的更新频率（远处/不重要的降低频率）。
- **类比**：分拣中心的"优先级评估员"，目前还是空岗、备用。

---

### ④ 工具静态库（3 个）

都是 `UBlueprintFunctionLibrary` 子类，提供**蓝图可调用的静态工具函数**。

#### `ULyraSystemStatics`（运行时通用工具）
| 函数 | 作用 |
|------|------|
| `GetTypedSoftObjectReferenceFromPrimaryAssetId` | 从 PrimaryAssetId 拿软引用（即使未加载） |
| `GetPrimaryAssetIdFromUserFacingExperienceName` | 从体验名拿 PrimaryAssetId |
| `PlayNextGame` | 玩下一局（仅服务器） |
| `SetScalarParameterValueOnAllMeshComponents` | 给身上所有网格组件设标量材质参数 |
| `SetVectorParameterValueOnAllMeshComponents` | 设向量材质参数 |
| `SetColorParameterValueOnAllMeshComponents` | 设颜色材质参数 |
| `FindComponentsByClass` | 找某类的所有组件 |

> **特点**：偏"运行时表现"——材质参数、组件查找、资产引用。

#### `ULyraDevelopmentStatics`（开发期工具）
| 函数 | 作用 |
|------|------|
| `ShouldSkipDirectlyToGameplay` | 是否跳过暖场直接开打（PIE 调试用） |
| `ShouldLoadCosmeticBackgrounds` | 编辑器里是否加载装饰背景 |
| `CanPlayerBotsAttack` | 玩家机器人能否攻击 |
| `FindPlayInEditorAuthorityWorld` | 找适合跑服务器作弊的世界 |
| `FindClassByShortName` | 用短名找类（作弊控制台友好） |

> **特点**：偏"开发/调试"——PIE 测试、作弊、跳过流程。

#### `ULyraActorUtilities`（Actor 工具）
- 就一个函数 `SwitchOnNetMode`：返回当前网络模式（Standalone / DedicatedServer / ListenServer / Client）。
- 顺带定义了 `EBlueprintExposedNetMode` 枚举（把引擎的 NetMode 暴露给蓝图，便于做分支）。
- **类比**：一个小工具——"你现在是服务器还是客户端？"

---

### ⑤ 数据结构（1 组）

#### `FGameplayTagStack` + `FGameplayTagStackContainer`
- **角色**：**带计数的 GameplayTag 栈**，支持网络增量同步。
- **核心概念**：
  - `FGameplayTagStack`：一个 Tag + 一个计数 `StackCount`（如"中毒叠了 3 层"）。
  - `FGameplayTagStackContainer`：一堆 Tag 栈的容器，提供 `AddStack / RemoveStack / GetStackCount / ContainsTag`。
- **关键技术**：
  - 继承 `FFastArraySerializer`，用 **FastArray 增量同步**（只同步变化的部分，省带宽）。
  - 内部维护 `TagToCountMap` 加速查询。
  - 有 `PreReplicatedRemove / PostReplicatedAdd / PostReplicatedChange` 回调。
- **类比**：一个"可叠加的 Buff 计数器"，且能高效同步给所有客户端。

> **用途**：需要"叠层"机制的场景（如中毒叠 3 层、护盾叠层），比普通 Tag 多了"数量"维度，且天然支持网络同步。

---

## 三、按"重要程度"排序的记忆清单

| 优先级 | 类 | 为什么重要 |
|--------|-----|-----------|
| ★★★ | `ULyraAssetManager` | 所有 DataAsset 加载的枢纽，启动流程核心 |
| ★★★ | `ULyraReplicationGraph` | 多人同步的调度中枢，网络优化关键 |
| ★★☆ | `ULyraGameInstance` | 全局单例，登录/加密/旅行都在这 |
| ★★☆ | `FGameplayTagStackContainer` | 带计数的可同步 Tag，叠层机制基础 |
| ★★☆ | `ULyraGameData` | 全局数据资产，存伤害/治疗 GE |
| ★☆☆ | `ULyraReplicationGraphSettings` | 复制图配置 |
| ★☆☆ | `ULyraSystemStatics` | 运行时工具集 |
| ★☆☆ | `FLyraAssetManagerStartupJob` | 启动任务结构 |
| ☆ | `ULyraGameEngine` / `ALyraGameSession` | 生命周期骨架，改动少 |
| ☆ | `ULyraDevelopmentStatics` / `ULyraActorUtilities` | 调试/小工具 |
| ☆ | `ULyraSignificanceManager` | 空占位，备用扩展 |

---

## 四、总结（一句话记忆）

```
System 目录 = Lyra 的"底层基础设施层"，13 个类分 5 组：

  ① 引擎入口：GameEngine → GameInstance → GameSession（生命周期骨架）
  ② 资源管理：AssetManager（枢纽）+ StartupJob（任务）+ GameData（全局数据）★
  ③ 网络同步：ReplicationGraph（调度）+ Settings（配置）+ Types（枚举）+ Significance（占位）★
  ④ 工具库：SystemStatics（运行时）/ DevelopmentStatics（调试）/ ActorUtilities（网络模式）
  ⑤ 数据结构：GameplayTagStack（带计数、可增量同步的 Tag 栈）

两个最核心：ULyraAssetManager（资源枢纽）、ULyraReplicationGraph（网络调度）。
```

**一句话**：`System` 目录是 Lyra 的**基础设施层**——`ULyraAssetManager` 管所有数据资产加载、`ULyraReplicationGraph` 管多人网络同步调度，这俩是重点；其余是引擎入口（Engine/Instance/Session）、蓝图工具库（3 个 Statics）和一个可同步的 Tag 栈数据结构。

---

## 五、下一步

- 深入 `ULyraAssetManager` 的启动流程（`StartInitialLoading` / `DoAllStartupJobs`）源码。
- 深入 `ULyraReplicationGraph` 的路由逻辑（`RouteAddNetworkActorToNodes`）。
- 看 `FGameplayTagStackContainer` 如何被 GAS 组件使用（叠层 Buff）。
- 对比 `ULyraGameInstance` 与 Experience 系统如何配合。
