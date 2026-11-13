# P15 GameFeatures —— GameFeatureAction 实现层（总 md）

> **定位**：本文件是 P15（GameFeatures）的**总入口 + 全景地图**。目标：理解并一比一还原 Lyra 在 `Source/LyraGame/GameFeatures/` 底下的"游戏特性动作（GameFeatureAction）"。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\GameFeatures\`（16 个文件，8 个类）
> 命名映射：`Lyra`→`Tenkaichi`，其余一字不改（铁律 24）。**注意：`GameFeatureAction_XXX` 系列类名无 `Lyra` 前缀，原样保留**（铁律 24：无前缀不换）。

---

## 〇、为什么先建这个总 md（铁律 35）

P15 是一个独立教学内容（8 个类），先建本总 md（全景 + 依赖 + 教学顺序），**停下等确认**。

> 上一节 P14（System 收口）已全部完成。P15 是"GameFeature 动作"的实现层——P13 经验系统里 `ExperienceManagerComponent` 执行的 `GameFeatureAction`（`Action->OnGameFeatureActivating`）就是这里定义的。

---

## 一、这一节解决什么问题（先讲为什么）

P13 里讲过：经验加载完会"执行 Action"（`OnExperienceFullLoadCompleted` 里 `Action->OnGameFeatureActivating`）。那这些 Action 到底是什么？

**GameFeatureAction** 是"游戏特性插件激活时要做的事"的封装。当一个 GameFeature 插件被激活，它带的 `UGameFeatureData` 里有一串 `GameFeatureAction`，每个 Action 做一件具体的事：加能力（AddAbilities）、加输入（AddInputBinding/ContextMapping）、加 UI（AddWidget）、加 GameplayCue 路径（AddGameplayCuePath）等。

> **类比**：GameFeature 插件是"一个可插拔的 DLC 模块"，`GameFeatureAction` 是"这个模块激活时要执行的清单"——每项清单是一个具体动作。P15 就是定义"各种动作"。

---

## 二、全景地图（8 个类）

| 类 | 作用 | 依赖 |
|----|------|------|
| `GameFeatureAction_WorldActionBase` | **基类**：世界特定的动作（激活/停用时处理 world） | 引擎 GameFeatures |
| `GameFeatureAction_AddAbilities` | 给 Actor 加能力（GAS） | P05 GAS + P01 Messages + P12 |
| `GameFeatureAction_AddGameplayCuePath` | 加 GameplayCue 路径 | P05 GAS |
| `GameFeatureAction_AddInputBinding` | 加输入绑定 | P03 Input |
| `GameFeatureAction_AddInputContextMapping` | 加输入上下文映射 | P03 Input |
| `GameFeatureAction_AddWidget` | 加 UI 控件 | **P16 UI（挂起）** |
| `GameFeatureAction_SplitscreenConfig` | 分屏配置 | 引擎 |
| `LyraGameFeaturePolicy` | GameFeature 策略（发现/加载模式）+ 2 个观察者 | 引擎 GameFeatures |

---

## 三、依赖分析（铁律 30/31）

| 类 | `.h` 依赖 | 状态 |
|----|----------|------|
| `WorldActionBase` | 引擎 `GameFeatureAction.h` + `GameFeaturesSubsystem.h` | ✅ 零挂起 |
| `AddGameplayCuePath` | 引擎 + `GameFeatureAction.h` | ✅ 零挂起 |
| `SplitscreenConfig` | 引擎 | ✅ 零挂起 |
| `GameFeaturePolicy` | 引擎 `GameFeaturesProjectPolicies.h` | ✅ 零挂起（`.cpp` 依赖同组类） |
| `AddAbilities` | P05 GAS + P01 Messages + P12 PawnData | ✅ 基本零挂起 |
| `AddInputBinding` / `AddInputContextMapping` | P03 Input + P12 | ✅ 基本零挂起 |
| `AddWidget` | **P16 UI（`UI/` 未建）** | ⚠️ 挂起 |

> ⚠️ **主要断点**：`AddWidget` 依赖 P16 的 UI 控件（`UCommonActivatableWidget` 等），需要挂起。其余大部分依赖 P01/P03/P05/P12，已就位。

> 具体每个类的断点，在教到那个类时精确分析（铁律 30：查真实依赖）。

---

## 四、教学顺序建议（按依赖自下而上）

| 顺序 | 类 | 处理 |
|------|----|------|
| P15.1 | `GameFeatureAction_WorldActionBase` | 基类，最先教（被所有 Action 依赖） |
| P15.2 | `GameFeatureAction_AddGameplayCuePath` | 简单（GAS） |
| P15.3 | `GameFeatureAction_SplitscreenConfig` | 简单（引擎） |
| P15.4 | `GameFeatureAction_AddAbilities` | 核心（GAS + Messages + PawnData） |
| P15.5 | `GameFeatureAction_AddInputBinding` | 输入 |
| P15.6 | `GameFeatureAction_AddInputContextMapping` | 输入（更大） |
| P15.7 | `GameFeatureAction_AddWidget` | UI（挂起 P16） |
| P15.8 | `LyraGameFeaturePolicy` | 策略（收口，依赖同组） |

---

## 五、待确认

1. GameFeatureAction 是"插件激活时执行的清单项"理解了吗？
2. 8 个类按 **基类 → 简单 Action → 核心 Action → 输入 → UI → 策略** 的顺序教，OK 吗？
3. `AddWidget` 挂起 P16 UI，其余基本零挂起，认可吗？
4. 确认后我**逐个类**教：每个类先建自己的总 md → 停 → 教 `.h` → 停 → 教 `.cpp`（铁律 20/29/35）。

---

## 六、源码依据

- `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\GameFeatures\`（16 文件，8 类）
- 你的工程：`Source/TenkaichiGame/GameFeatures/`（目录待建）