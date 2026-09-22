# 00 — System 目录开篇：这一整个文件夹在干嘛、每个文件管什么

> **定位**：`Source/LyraGame/System/` 只有 14 组 `.h/.cpp`，却是全 Lyra 的"**总管家层**"——不负责某个玩法（射击/装备），负责"**游戏进程怎么启动、全局数据怎么拿、网络怎么优化**"这类**全项目地基级**的事。
>
> **一句话**：Character 管"一个角色怎么动"，System 管"**整个游戏进程怎么活**"。

---

## 一、先给全貌：14 组文件分 5 派

```
 LyraGame/System/（14组 = 13个类/库 + 1个辅助结构）
 ═══════════════════════════════════════════════════════════
 派系A 进程级骨架（挂在哪、由谁实例化）       文件
 ┌──────────────────────────────────────────────────────┐
 │ 引擎最底层启动 →  ULyraGameEngine   (引擎一启动就有)   │
 │ 游戏会话层     →  ULyraGameInstance (每局游戏的大脑)   │
 │ 比赛生命周期   →  ALyraGameSession  (开始/结束钩子)    │
 │ 资源大管家     →  ULyraAssetManager (全局加载/缓存)     │
 └──────────────────────────────────────────────────────┘
 派系B 全局数据与加载
 ┌──────────────────────────────────────────────────────┐
 │ ULyraGameData         全局数据资产(伤害GE/治疗GE...)  │
 │ FLyraAssetManagerStartupJob  启动加载任务的进度包装   │
 └──────────────────────────────────────────────────────┘
 派系C 网络与性能扩展
 ┌──────────────────────────────────────────────────────┐
 │ ULyraReplicationGraph   大战场网络复制优化            │
 │  +Settings / +Types     它的配置(可CVar调)与分类枚举   │
 │ ULyraSignificanceManager 对象重要性分级(省性能)       │
 └──────────────────────────────────────────────────────┘
 派系D 通用计数 Tag
 ┌──────────────────────────────────────────────────────┐
 │ FGameplayTagStackContainer  "带层数的Tag"(可网络复制)  │
 └──────────────────────────────────────────────────────┘
 派系E 静态工具库（到处能调的函数）
 ┌──────────────────────────────────────────────────────┐
 │ ULyraSystemStatics     全局杂项(下一局/材质/组件查询)  │
 │ ULyraActorUtilities    网络模式判断(单机/客户端/服)    │
 │ ULyraDevelopmentStatics 开发期辅助(PIE跳过流程等)      │
 └──────────────────────────────────────────────────────┘
 ═══════════════════════════════════════════════════════════
```

---

## 二、逐个文件一句话（按上表顺序）

### 派系 A：进程级骨架（"游戏进程怎么活"）

| 文件 | 继承谁 | 它管什么 |
|---|---|---|
| `LyraGameEngine.h/.cpp` | `UGameEngine` | 引擎最底层的游戏引擎对象；覆写 `Init()`。全进程启动的第一站（在 DefaultEngine.ini 里指定用它） |
| `LyraGameInstance.h/.cpp` | `UCommonGameInstance` | **全 Lyra 真正的"启动大脑"**：`Init()`、账号初始化（`HandlerUserInitialized`）、网络加密、拿主 PlayerController。上一章 `LyraGameModule.cpp` 是"门卫"，它是"总经理" |
| `LyraGameSession.h/.cpp` | `AGameSession` | 一局比赛的会话：自动登录、`HandleMatchHasStarted/Ended`（比赛开始/结束时的钩子） |
| `LyraAssetManager.h/.cpp` | `UAssetManager` | **全局资源大管家**：统一加载/缓存，提供 `GetGameData()`、`GetDefaultPawnData()`、同步加载软引用、启动期批量加载 |

### 派系 B：全局数据与加载

| 文件 | 类型 | 它管什么 |
|---|---|---|
| `LyraGameData.h/.cpp` | `UPrimaryDataAsset` | 一份"全局游戏数据"资产：默认伤害 GE、治疗 GE、动态标签 GE（都是 `TSoftClassPtr` 软引用）。全项目通过 AssetManager 拿这一份 |
| `LyraAssetManagerStartupJob.h/.cpp` | `struct` | 给 AssetManager 启动期加载用的"任务包装"，带子进度回调（加载到 30%、70%…显示在启动画面上） |

### 派系 C：网络与性能扩展

| 文件 | 继承谁 | 它管什么 |
|---|---|---|
| `LyraReplicationGraph.h/.cpp` | `UReplicationGraph` | **大战场网络优化核心**：给每类 Actor 定复制策略（空间网格化/常相关/按距离…），只把该同步的发给该看到的人 |
| `LyraReplicationGraphSettings.h/.cpp` | `UDeveloperSettingsBackedByCVars` | 上面那个的开关/参数面板，每个选项还能直接变成控制台变量（CVar）调 |
| `LyraReplicationGraphTypes.h` | 枚举/类型 | RepGraph 内部用的分类枚举（Actor 属于哪种复制节点） |
| `LyraSignificanceManager.h/.cpp` | `USignificanceManager` | 对象"重要性"分级器，目前是空扩展点——Lyra 留好口子，让远处低重要性的东西降级更新 |

### 派系 D：通用计数 Tag

| 文件 | 类型 | 它管什么 |
|---|---|---|
| `GameplayTagStack.h/.cpp` | `USTRUCT` | "**带层数的标签**"：比如 `Status.Invulnerable` 叠 2 层就免疫 2 次。用 `FFastArraySerializer` 实现，**能高效网络复制** |

### 派系 E：静态工具库

| 文件 | 类型 | 它管什么 |
|---|---|---|
| `LyraSystemStatics.h/.cpp` | 蓝图函数库 | 全局杂活：`PlayNextGame`（打完这局进下个体验）、PrimaryAssetId→资产软引用、给 Actor 所有 Mesh 批量设材质参数、按类型找组件 |
| `LyraActorUtilities.h/.cpp` | 蓝图函数库 | 给蓝图暴露"当前是 单机/专服/监听服/客户端"的网络模式判断 |
| `LyraDevelopmentStatics.h/.cpp` | 蓝图函数库 | **只活在开发期**：PIE 时是否跳过热身直进玩法、是否加载编辑器背景、按短名找类（cheat 用）等 |

---

## 三、它们怎么串成"启动链"（结合上章阅读顺序）

```
 引擎 exe 启动
   │
   ▼
 ULyraGameEngine::Init()        ← LyraGameEngine（进程第一站）
   │
   ▼
 ULyraAssetManager 启动加载      ← 派系A管家 + 派系B数据开始进场
   │     ▲ FLyraAssetManagerStartupJob 报告进度
   ▼
 ULyraGameInstance::Init()      ← 真·大脑：玩家账号初始化
   │
   ▼
 玩家登录/进房 ──► ALyraGameSession（比赛开始/结束）
   │
   ▼
 进图开打 ──► ULyraReplicationGraph 负责"谁看谁"的网络复制
              + ULyraSignificanceManager 负责"远处降级省性能"
   │
   ▼
 战斗中：GameplayTagStack 叠层数（无敌层/弹药层）、
         各处调 SystemStatics/DevelopmentStatics 静态函数
```

> 阅读地图：这一章往后会逐个深入，**建议先啃 `LyraGameInstance`**（它是启动流程的"总经理"，和你在 `04` 看的 LyraGameModule 正好接上）；再啃 `LyraAssetManager`（资源怎么全局管），然后 RepGraph（大战场网络）。工具库类（派系 E）随时用到随时查即可。

---

## 四、本篇一句话

`System/` = Lyra 的"**进程管家层**"：GameEngine/GameInstance/GameSession 管"进程怎么活"，AssetManager + GameData 管"资源/数据怎么拿"，RepGraph/SignificanceManager 管"大战场怎么不卡"，剩下是通用 Tag 计数和几个到处能调的静态工具库。**它是理解 Lyra 启动流程的第一站，也是往后深入每个大系统前先要认识的"地基目录"。**
