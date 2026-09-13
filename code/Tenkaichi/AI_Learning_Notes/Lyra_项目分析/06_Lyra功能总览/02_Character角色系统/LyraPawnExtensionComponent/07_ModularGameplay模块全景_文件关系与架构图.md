# 07 — `ModularGameplay` 模块全景：文件关系与架构图

> **定位**：把这个模块里的**所有文件**摆在一起，讲清楚它们各自的角色、彼此怎么协作。前几篇只讲了单个类，这篇是"全家福"。
>
> 不讲具体实现，只讲原理、职责和关系图。

---

## 一、这个模块是干嘛的？（一句话）

> **`ModularGameplay` = 一套"模块化游戏框架"基础设施。它让游戏能用"组件 + 状态"的方式，把功能像插积木一样插到 Pawn/Controller/PlayerState/GameState 上，并让这些组件能协调初始化顺序。**

它的核心解决两件事：
1. **动态装配**：不用写死在类里，也能给 Actor 自动装上需要的组件（按类匹配）。
2. **初始化协调**：多个组件谁先初始化、依赖谁，用"初始化状态（InitState）"这套机制排队。

---

## 二、文件清单（一共就这些，很规整）

```
ModularGameplay/Source/ModularGameplay/
├─ Public/
│  ├─ ModularGameplayModule.h          （模块入口，可忽略）
│  └─ Components/
│     ├─ GameFrameworkComponent.h          ★ 组件基类（一切组件的根）
│     ├─ GameFrameworkComponentManager.h   ★ 大管家（核心中的核心）
│     ├─ GameFrameworkComponentDelegates.h ★ 委托/参数（消息格式）
│     ├─ GameFrameworkInitStateInterface.h ★ 初始化状态接口（"电话"）
│     ├─ PawnComponent.h                   （Pawn 专用组件基类）
│     ├─ ControllerComponent.h             （Controller 专用组件基类）
│     ├─ PlayerStateComponent.h            （PlayerState 专用组件基类）
│     └─ GameStateComponent.h              （GameState 专用组件基类）
└─ Private/  （对应的 .cpp 实现，一一对应）
```

> ★ = 四个核心文件，理解了它们就理解了整个模块。

---

## 三、四大核心角色 + 它们的职责

| 文件 | 角色 | 一句话职责 |
|------|------|-----------|
| `UGameFrameworkComponent` | **组件基类** | 所有框架组件的"祖宗"，给它们实体 + 框架血统 |
| `UGameFrameworkComponentManager` | **大管家** | 子系统，统一管理"装配请求"和"初始化状态" |
| `IGameFrameworkInitStateInterface` | **状态接口** | 一张"门禁卡"，让组件能跟大管家对话操作系统 |
| `FActorInitStateChangedParams` / 委托 | **消息格式** | 状态变化时传递的"通知单" |

再加上四个"专用组件基类"（`Pawn/Controller/PlayerState/GameState Component`），它们都是 `UGameFrameworkComponent` 的子类，只是各自多了一个方便取 Owner 的函数。

---

## 四、总架构图（最重要！）

```
                        ┌──────────────────────────────────────────────┐
                        │        UGameFrameworkComponentManager         │
                        │           （大管家 / 子系统）                  │
                        │                                              │
                        │  ① 装配系统：按类匹配，给 Actor 自动装组件     │
                        │     AddReceiver / AddComponentRequest         │
                        │                                              │
                        │  ② 初始化状态系统：记录每个 Feature 的状态      │
                        │     RegisterInitState / ChangeFeatureInitState│
                        │     GetInitStateForFeature / 广播通知          │
                        └───────────────┬──────────────────┬───────────┘
                                        │                  │
                    管理"装配/状态"      │                  │  被接口"打电话"找
                                        ▼                  ▼
   ┌──────────────────────────────────────────┐   ┌───────────────────────────────┐
   │        各类 Actor（receiver）              │   │  IGameFrameworkInitStateInterface│
   │   APawn / AController / APlayerState ...  │   │      （初始化状态接口 = 门禁卡）  │
   │                                          │   │                                │
   │   身上挂着各种 UGameFrameworkComponent：   │   │  自己不存状态，每个函数都回头    │
   │   ┌────────────────────────────────────┐ │   │  找大管家：                     │
   │   │ UPawnComponent                     │ │   │  GetOwningActor() → 找宿主       │
   │   │ UControllerComponent               │ │   │  GetComponentManager() → 找管家  │
   │   │ UPlayerStateComponent              │ │   │  Manager->ChangeFeature...(…)   │
   │   │ UGameStateComponent                │ │   └───────────────┬────────────────┘
   │   │ ULyraPawnExtensionComponent(Lyra)  │ │                   │
   │   └────────────────────────────────────┘ │                   │ 实现(继承)
   └──────────────────────────────────────────┘                   ▼
              ▲                                  ┌───────────────────────────────────┐
              │ 继承                              │ LyraPawnExtensionComponent 等      │
              │                                  │ "既【是】组件，又【会】操作状态"    │
   ┌──────────┴───────────┐                      └───────────────────────────────────┘
   │ UGameFrameworkComponent │
   │   （所有组件的基类）     │
   │ 继承自 UActorComponent  │
   └─────────────────────────┘
```

---

## 五、两条主线（一定要分清！）

这个模块其实有**两套独立但配合**的机制，别混：

### 主线 A：组件装配系统（"自动装零件"）

> 解决：**"某类 Actor 出现时，自动给它装上指定的组件"**（不用写死在类里）。

```
游戏启动时注册请求：
   "以后凡是 APawn，都给它装一个 XXComponent"
        │  (AddComponentRequest / AddReceiver)
        ▼
   大管家记住这个请求
        │
   当某个 APawn 出生 ──► 大管家自动给它 new 出 XXComponent 挂上
   当请求句柄销毁 ──► 大管家把对应组件拆掉（引用计数，多人登记要全撤才拆）
```

关键类：`UGameFrameworkComponentManager` + `FComponentRequestHandle`（请求句柄，销毁即撤销）。

### 主线 B：初始化状态系统（"排队初始化"）

> 解决：**"多个组件/特性谁先初始化、依赖谁，用 GameplayTag 表示状态，排队推进"**。

```
组件说："我要报到，我的特性叫 X，当前状态是 WaitingOnAvatar"
        │  (RegisterInitStateFeature / TryToChangeInitState)
        ▼
   大管家记录：ActorA 的 FeatureX = WaitingOnAvatar
        │
   条件满足 → 组件调用 TryToChangeInitState(DataAvailable)
        │
   大管家更新状态 + 广播通知监听者（发 FActorInitStateChangedParams 通知单）
```

关键类：`UGameFrameworkComponentManager`（状态存储+广播）+ `IGameFrameworkInitStateInterface`（组件用来打电话的接口）+ `FActorInitStateChangedParams`（通知单内容）。

> **主线 B 是"建在主线 A 的地基上"的**：因为想参与状态系统的对象，往往就是那些被装配的组件。

---

## 六、类继承关系图（纵向）

```
UObject
 └─ UActorComponent                    （引擎核心：我是组件）
      └─ UGameFrameworkComponent        ★ 框架组件基类（第04/05篇）
           ├─ UPawnComponent            （专为 Pawn，能 GetPawn）
           │    └─ ULyraPawnExtensionComponent  （Lyra 的具体实现）
           ├─ UControllerComponent      （专为 Controller，能 GetController/Pawn）
           ├─ UPlayerStateComponent     （专为 PlayerState，能 GetPlayerState/Pawn）
           └─ UGameStateComponent       （专为 GameState，能 GetGameState/GameMode）

UInterface
 └─ UGameFrameworkInitStateInterface
      └─ class IGameFrameworkInitStateInterface  ★ 初始化状态接口（第06篇）
                 ↑ 被各种对象"额外实现"（如 LyraPawnExtensionComponent）
```

> **注意**：`LyraPawnExtensionComponent` 在图里出现了两次视角——
> 纵向看它是 `UPawnComponent` 的子孙（**身份**），同时它又"横向"实现了 `IGameFrameworkInitStateInterface`（**技能**）。这就是第 06 篇讲的"类 + 接口"双重身份。

---

## 七、谁依赖谁？（箭头方向 = 调用方向）

```
四个专用组件基类 ──► UGameFrameworkComponent（继承）
                          │
LyraPawnExtensionComponent ┼──► UGameFrameworkComponent（继承，拿身份）
                          └──► IGameFrameworkInitStateInterface（实现，拿技能）
                                      │
IGameFrameworkInitStateInterface ──────► UGameFrameworkComponentManager（打电话：GetComponentManager）
                                              │
UGameFrameworkComponentManager ───────────────► 管着所有 Actor 的组件 & 状态
                                              │
FActorInitStateChangedParams / 委托 ◄─────────┘（状态变化时由大管家发出）
```

**一句话串起来**：
> 组件**继承** `UGameFrameworkComponent` 拿到身份，**实现** `IGameFrameworkInitStateInterface` 拿到"打电话"的技能，通过接口**找到** `UGameFrameworkComponentManager`（大管家），大管家负责**装配组件**和**管理初始化状态**，状态变化时发出**委托通知**。

---

## 八、常见误区

| 误区 | 正确理解 |
|------|---------|
| "大管家是组件" | ❌ 大管家是 `UGameInstanceSubsystem`（子系统），不是组件；组件是被它管理的对象 |
| "InitState 是一套新的组件" | ❌ InitState 是一套**状态协调机制**，组件还是那些组件 |
| "四个专用组件基类各搞一套" | ❌ 它们都只是 `UGameFrameworkComponent` 的子类，仅多了个 `GetOwner` 便捷函数 |
| "接口和大管家是竞争关系" | ❌ 接口是"电话"，大管家是"后台"，接口每句话都是打给大管家的 |
| "装配系统和状态系统是同一套" | ❌ 两套独立机制，但状态系统常建在装配系统之上 |

---

## 九、总结（一张图记忆）

```
ModularGameplay 模块 = 模块化游戏框架基础设施
─────────────────────────────────────────────
两大机制：
  A. 组件装配系统  —— 按类匹配，自动给 Actor 装/拆组件（引用计数）
  B. 初始化状态系统 —— 用 GameplayTag 排队协调多个特性的初始化

四个核心文件：
  • UGameFrameworkComponent         = 组件基类（身份/实体）
  • UGameFrameworkComponentManager  = 大管家（装配 + 状态，子系统）
  • IGameFrameworkInitStateInterface= 门禁卡/电话（跟管家对话）
  • FActorInitStateChangedParams    = 通知单（状态变化的消息格式）

四个专用组件基类：Pawn / Controller / PlayerState / GameState Component
   （都是 UGameFrameworkComponent 子类，各加一个 GetOwner 便捷函数）

关系一句话：
  组件继承基类拿【身份】，实现接口拿【电话】，打给【大管家】，
  大管家管【装配】和【状态】，变化时发【通知单】。
```

**一句话**：`ModularGameplay` 就是一个"组件化 + 状态化"的游戏框架底座——`UGameFrameworkComponent` 是所有组件的基类，`UGameFrameworkComponentManager`（大管家/子系统）统一管"装配"和"初始化状态"两件事，`IGameFrameworkInitStateInterface` 是组件用来跟大管家对话的接口，四个专用组件基类（Pawn/Controller/PlayerState/GameState）只是针对四种 Actor 的便利封装。

---

## 十、下一步

- 深入"组件装配系统"的完整流程（`AddReceiver` → `CreateComponentOnInstance`）。
- 深入"初始化状态系统"的状态机推进与回调队列（`StateChangeQueue` 防递归）。
- 看 Lyra 是如何具体使用这套设施（`LyraPawnExtensionComponent` 的状态链）。
