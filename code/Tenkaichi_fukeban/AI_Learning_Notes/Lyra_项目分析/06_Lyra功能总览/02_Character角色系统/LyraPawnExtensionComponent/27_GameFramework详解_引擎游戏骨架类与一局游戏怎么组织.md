# 27 — GameFramework 详解：UE 的"游戏骨架类"与一局游戏怎么组织

> **定位**：你问"GameFramework 是什么"——它对应 `.cpp` include 里的 `GameFramework/Pawn.h`、`GameFramework/Controller.h`。这一篇讲清 **UE 引擎层"GameFramework"那套游戏骨架类**：是什么、有哪些成员、各自干嘛、怎么配合、和 ModularGameplay 那套同名东西怎么区分。
>
> **衔接**：第 26 篇（cpp 的 include）、第 05 篇（UPawnComponent）。这篇讲的是"总指挥服务的那个身体（Pawn）所属的更大骨架"。

---

## 〇、一句话先说清

> **UE 的 GameFramework = 支撑"一局游戏怎么运转"的基础类集合**，都在引擎目录 `Engine/.../Classes/GameFramework/` 下：`APawn`（身体）、`AController`（大脑）、`AGameModeBase`（裁判）、`AGameStateBase`（记分牌）、`APlayerState`（档案）……
>
> 它回答的问题是：**"谁在玩、谁控制谁、游戏规则谁定、输赢和状态怎么同步。"**

```
你学的 ULyraPawnExtensionComponent（总指挥，组件）
        │ 挂在谁身上？
        ▼
   APawn / ACharacter（引擎 GameFramework 的身体类）
        │ 属于哪个骨架？
        ▼
   AGameMode / AGameState / APlayerState / AController ...
   （引擎 GameFramework 的"一局游戏运转骨架"）
```

---

## 一、GameFramework 是干嘛的？（先给故事）

想象一局多人对战，引擎得回答几个**基础问题**才能跑起来：

| 问题 | 引擎靠什么回答 |
|---|---|
| 谁是裁判？用什么规则？ | `AGameModeBase` |
| 玩家从哪出来？ | GameMode 决定 Spawn 什么 Pawn |
| 玩家用什么控制角色？ | `AController`（玩家→PlayerController，AI→AIController） |
| 角色身体是谁？ | `APawn` / `ACharacter` |
| 玩家死了分数保留在哪？ | `APlayerState`（档案，跨重生保留） |
| 全局面貌（还剩几人）同步给所有人？ | `AGameStateBase` |

**一句话故事**：

> 开一局游戏 = **裁判（GameMode）定规则并喊"开始"** → 每个玩家进来 → 生成"**档案**"（PlayerState）+ 派个"**大脑**"（Controller）→ 大脑去控制一个"**身体**"（Pawn/Character）→ 打起来后，**记分牌**（GameState）把全局面貌同步给所有人。

GameFramework 就是这套"**一局游戏的骨架**"。**没有它，引擎不知道"游戏"是什么。**

---

## 二、骨架成员逐个看（核心 6 类）

| 类 | 前缀 | 大白话 | 谁创建 / 生命周期 |
|---|---|---|---|
| `AGameModeBase` | A | **裁判/规则**：用什么 Pawn、几人开、怎么算赢 | 服务器，每局一个 |
| `AGameStateBase` | A | **记分牌**：全局面貌，广播给所有人 | 服务器创建，**复制**到客户端 |
| `APlayerState` | A | **玩家档案**：分数/队伍，跨死亡保留 | 服务器，每个玩家一个，**复制** |
| `AController` | A | **大脑**：控制 Pawn（玩家→PlayerController / AI→AIController） | 服务器 |
| `APawn` | A | **身体**：被控制、能移动的对象 | GameMode Spawn |
| `ACharacter` | A | Pawn 的**人形版**：自带胶囊体 + CharacterMovement | 同上 |

> **记忆**：GameMode 定规则、GameState 给全貌、PlayerState 存档案、Controller 当大脑、Pawn/Character 当身体。

---

## 三、它们怎么配合？（一局游戏的完整流程）

```
① 服务器开房 → 生成 GameMode（裁判）+ GameState（记分牌）
        │
② 玩家加入 → 服务器为 ta 建 PlayerState（档案）
        │
③ 给玩家派 Controller（大脑）→ 需要"身体"了
        │
④ GameMode 问："这个玩家该用什么 Pawn？"（PawnData/DefaultPawnClass）
        │  → 服务器 Spawn 一个 APawn（身体）
        │  → Controller 控制它（Possess）→ 玩家"活"了
        ▼
⑤ 玩家玩 → 数据进 PlayerState（档案），状态同步到 GameState（记分牌）
        │
⑥ 玩家死了 → 身体销毁，但档案（PlayerState）还在
        │  → 重生时 GameMode 再 Spawn 一个新身体 → 换身体不换档案
```

> **重点观察：Pawn 是最"短命"的，PlayerState/Controller 最"长命"。** 死了换身体，但你的分数（PlayerState）和大局（GameState）都还在——这就是为什么它们要分开。

---

## 四、网络视角：谁在服务器、谁复制到客户端

UE 的 GameFramework 带**明确的主从（权威）关系**：

| 对象 | 谁权威 | 客户端有吗 |
|---|---|---|
| `GameMode` | **只有服务器**有 | ❌ 客户端不存在 GameMode |
| `GameState` | 服务器生成 | ✅ **复制**给所有客户端 |
| `PlayerState` | 服务器生成 | ✅ 复制（自己+别人） |
| `Controller` | 服务器 | 本地的 PlayerController 有代理 |
| `Pawn` | 服务器 | ✅ 复制（属性同步） |

> **意义**：规则（GameMode）只在服务器算，防止作弊；能看到的全局信息（GameState/PlayerState）复制给大家。**这套"权威分工"是 Lyra 一切网络逻辑的地基**（比如前面学的 PawnData 带 ReplicatedUsing 就是在这个框架下同步的）。

---

## 五、关键区分：引擎 GameFramework vs ModularGameplay 的 GameFrameworkComponent

这是最容易混的（第 26 篇里两种 include 长得像）。**它们不是一回事：**

| | 引擎 GameFramework（本篇） | ModularGameplay 的 GameFrameworkComponent |
|---|---|---|
| 在哪 | 引擎 `Engine/GameFramework/` | 插件 `ModularGameplay/Components/` |
| 例子 | `APawn`、`AController`、`AGameModeBase` | `UGameFrameworkComponent`、`UGameFrameworkComponentManager` |
| 是啥 | 一局游戏的**骨架实体类**（Actor） | 给这些 Actor **加功能的组件体系** |
| 前缀 | `A`（Actor） | `U`（组件） |
| 管什么 | "一局游戏谁是谁、规则谁定" | "组件怎么装配、怎么按状态初始化" |

**两者的关系（一句话）**：

> 引擎 GameFramework 提供"**身体、大脑、裁判**"这些实体；ModularGameplay 提供"**往这些实体身上插零件、协调零件初始化**"的机制。**`ULyraPawnExtensionComponent` 是后者做出来的一个"零件"，装在 APawn（前者）身上。**

**把 Lyra 的实际继承链放进来，一眼看清：**

```
引擎 GameFramework 层（本篇）
   APawn ◄──── 引擎：能被 Controller 控制的"身体"
     │
ModularGameplay 层（Modular Pawn，把 GameFramework 和组件体系接起来）
   AModularPawn ◄──── UE 提供的"能挂 PawnComponent 的 Pawn"
     │
Lyra 层
   ALyraPawn : AModularPawn            ← Lyra 的身体类（第 01 篇看过）
   ULyraPawnExtensionComponent : UPawnComponent  ← 挂在 ALyraPawn 身上的总指挥
```

> **对照源码确认**：`LyraPawn.h` 第 20 行 `class ALyraPawn : public AModularPawn`——Lyra 的身体**不是直接继承 APawn**，而是继承"**模块化 Pawn**"（AModularPawn），因为 Lyra 要给 Pawn 装一堆组件。而 `AModularPawn` 的爷爷就是 `APawn`（引擎 GameFramework）。

---

## 六、那 `GameFramework/Pawn.h`、`GameFramework/Controller.h` 的 include 里拉的是谁？

回到 `.cpp` include（第 26 篇），路径前缀是**引擎目录**：

```cpp
#include "GameFramework/Controller.h"   // ← 引擎的 AController（大脑）
#include "GameFramework/Pawn.h"         // ← 引擎的 APawn（身体）
```

它俩在 `.cpp` 里被用于（对照源码）：
- `GetPawn<APawn>()`（L45/228）→ 拿自己的身体（返回引擎 APawn）
- `GetController<AController>()`（L251）→ 检查"有没有大脑控制我"
- `Pawn->HasAuthority()` / `IsLocallyControlled()`（L245~246）→ 网络权威判断（第四节的分工）

> 所以 include 的 `GameFramework/Pawn.h` 是**引擎那套骨架的 APawn**，不是 ModularGameplay 的组件——**`ULyraPawnExtensionComponent` 作为组件挂在 APawn 上，它当然要知道"身体"长什么样。**

---

## 七、总结一句话

> **UE 的 GameFramework = 引擎层的"一局游戏骨架类"**：`GameMode`（裁判）+ `GameState`（记分牌）+ `PlayerState`（玩家档案）+ `Controller`（大脑）+ `Pawn`/`Character`（身体），共同回答"一局游戏怎么组织"。它有严格的服务器权威分工（规则只在服务器、状态复制给客户端）。**它和 ModularGameplay 的 `UGameFrameworkComponent`（给 Actor 加功能的组件体系）是两层东西**——Lyra 的 `ALyraPawn`（继承 `AModularPawn`→`APawn`）是骨架里的"身体"，你学的 `ULyraPawnExtensionComponent`（继承 `UPawnComponent`）是 ModularGameplay 体系给这个身体做的"总指挥组件"。

---

## 八、下一步

- 去看引擎 `GameFramework/GameModeBase.h`、`PlayerController.h` 的类注释，理解每个骨架类的官方定位。
- 追 `AModularPawn`：它比 `APawn` 多加了什么（为什么 Lyra 不直接用 APawn）。
- 结合前面学的 `LyraPawnData.PawnClass`（配置单指定 Pawn 类），理解 GameMode 怎么用配置选"身体"。
