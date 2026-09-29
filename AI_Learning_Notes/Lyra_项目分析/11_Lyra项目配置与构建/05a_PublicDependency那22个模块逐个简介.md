# 05a — LyraGame 的 Public 依赖逐个简介：这 22 个"底层模块"都是干嘛的

> **定位**：`05` 讲了 Public / Private 的概念。这篇把 `LyraGame.Build.cs` 里 Public 名单上的模块**一个一个过一遍**——每个在引擎源码里是什么、给 Lyra 提供什么。
>
> **先更正**：对照源码（L24-45），Public 名单实际是 **22 个**（不是 24），本文按 22 讲。

---

## 一、先认清两件事：它们不全是"插件"，来源也分两边

1. **它们是"模块（Module）"不是"插件"**——`01` 讲过两者区别。这些是代码依赖，不是 `uproject` 里的开关。
2. **来源分两边**：
   - 绝大多数是**引擎自带**（源码在 `Engine/` 里）；
   - 少数是 **Lyra 项目自带**（在 `Lyra/Plugins/`，前面 `01a` 验证过）。

---

## 二、逐个简介：22 个分 6 组，每组一个"地基"到"上层"的位置

### 组 0：纯地基（你写的任何模块几乎都依赖）

| 模块 | 底层是啥（源码核心类） | Lyra 拿它干嘛 |
|---|---|---|
| `Core` | UE 最底层库：`FString/FName/TArray/日志/内存/断言` | 一切代码都站在它上面 |
| `CoreUObject` | **UObject 体系**：`UClass`/反射/GC/`UObject` 基类 | 所有 `UCLASS` 类的地基 |
| `ApplicationCore` | 跨平台应用层：窗口/鼠标键盘/平台信息抽象 | 处理"我在哪个平台跑"的琐事 |
| `Engine` | 游戏运行主体：`World/Actor/GameMode/Component` | 建关卡、跑 Actor 全靠它 |

### 组 1：GAS 战斗三兄弟（引擎）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `GameplayTags` | `FGameplayTag` 分层标签 + 容器 | 所有状态/伤害类型打标（`State.Dead`…） |
| `GameplayTasks` | 异步任务 `UGameplayTask` | GA 的后台等待/延迟执行 |
| `GameplayAbilities` | **GAS**：`ASC/UGameplayAbility/UGameplayEffect/UAttributeSet` | 技能、伤害、属性、Buff 全部战斗 |

### 组 2：AI 与"模块化玩法"（引擎 + Lyra）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `AIModule` | 传统 AI：`AAIController`/行为树/黑板/感知 | NPC 决策、寻敌 |
| `ModularGameplay` | `UGameplayBehavior` 等模块化行为基类 | 让 Pawn 可动态挂行为片段 |
| `ModularGameplayActors` | `AModularActor/Pawn/Character` 基类 | 角色不用整块写死，靠组合 |
| `GameFeatures` | GameFeature 插件框架（注册/状态机） | 把玩法包动态装载（`02` 讲过） |

### 组 3：数据与大规模网络优化（引擎）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `DataRegistry` | 数据注册表：一堆数据资产运行时统一查询 | 读数值表（武器伤害、成长曲线） |
| `ReplicationGraph` | 网络复制图（按需复制 Actor） | 32 人同屏只同步看得见的 |
| `SignificanceManager` | 重要性管理器 | 远处的 AI/特效降频省 CPU |
| `Hotfix` | 热修复：运行时拉补丁 ini/蓝图 | 上线后不改包也能修数值 |
| `PropertyPath` | 属性路径解析工具（`FPropertyPath`） | UI/动画绑定时寻址属性用 |

### 组 4：表现与控制流（引擎为主）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `Niagara` | 粒子特效系统 | 枪火/爆炸/命中特效 |
| `ControlFlows` | UI 流程编排（把菜单流程当流程图画） | 主菜单→设置→游戏的跳转控制 |

### 组 5：Lyra 项目自带的（只有这 2 个，在 `Lyra/Plugins/`）

| 模块 | 底层是啥 | Lyra 拿它干嘛 |
|---|---|---|
| `CommonLoadingScreen` | Lyra 自己的加载画面模块 | 进游戏/切图时显示统一风格 Loading |
| `AsyncMixin` | 异步工具（`AsyncMixin` 混合类） | UI/组件里安全做异步加载，不闪崩 |

> ⚠️ 注意：`CommonLoadingScreen`、`AsyncMixin` 这两个模块**不在引擎里**，它们来自 `Lyra/Plugins/`（`01a` 的"项目自带"清单）。这也解释了为什么 LyraGame 要特意在 Build.cs 里加它们——Lyra 自己的代码互相依赖。

---

## 三、底层分层图：源码模块从地基往上盖

```
   ════════════════ LyraGame（你正在看的模块，住在最顶层）════════════════
   ▲ 它 Public 依赖了下面这些（层号越大越"接地"）
 │
 │ ⑥ Lyra 自家组件层（Plugins/ 里，Lyra 自写）
 │     CommonLoadingScreen        AsyncMixin
 │
 │ ⑤ 表现 & 流程层（引擎）
 │     Niagara                    ControlFlows
 │
 │ ④ 网络优化 & 数据层（引擎）
 │     ReplicationGraph          SignificanceManager
 │     Hotfix                    DataRegistry          PropertyPath
 │
 │ ③ AI & 玩法模块化层（引擎）
 │     AIModule                  ModularGameplay
 │     GameFeatures              ModularGameplayActors
 │
 │ ② GAS 战斗层（引擎 Plugins/Runtime）
 │     GameplayAbilities
 │       ├── GameplayTags        （标签语言，被 GA 用）
 │       └── GameplayTasks       （异步任务，被 GA 用）
 │
 │ ① 引擎地基层（Engine/Source/Runtime）
 │     Core    CoreUObject    ApplicationCore
 │     Engine  PhysicsCore    CoreOnline
 └────────────────────────────────────────────────────────
  依赖方向：越往下越基础 → LyraGame 站在全部 22 个之上
```

**读图要点**：
- **越靠下的模块越"底层"**，LyraGame 直接站在全部 22 个上面；
- 整棵树的**根是 `Core`**——它不依赖别人，谁都依赖它；
- GAS 三兄弟自己在第 ② 层内部还有依赖：`GameplayAbilities → GameplayTags / GameplayTasks`。

---

## 四、本篇一句话

这 22 个 Public 依赖 = LyraGame 声明"**我写头文件时会 include 这些模块，请让我的调用方也能用**"。想记住它们不用背清单，按"地基 → GAS → AI/模块化 → 数据/网络 → 表现/流程 → Lyra 自家"这 6 层记，每层挑 1-2 个代表作即可。剩下的 Private 名单（26 个）是 Lyra 藏起来的实现细节，详见 `05` 篇第五节。
