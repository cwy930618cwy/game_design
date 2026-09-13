# AModularPawn（类） vs ModularGameplay（插件）——区别

> **定位**：彻底分清这两个"名字几乎一样、但完全不是一层"的东西。
>
> 一句话破题：**`AModularPawn` 是一个具体的 Pawn 基类（实体类，在 Lyra 里）；`ModularGameplay` 是引擎的一个插件/框架（抽象方案）。它俩只是名字像，根本不是一回事。**

---

## 一、为什么你会混？因为名字太像 + 来源不同

你看到 `LyraPawn.h` 第 5 行：

```cpp
#include "ModularPawn.h"          // ← 引的是一个【类】
```

又看到引擎目录 `.../Plugins/Runtime/ModularGameplay` —— 一个**插件**。

两个都带 "Modular"，于是懵了。但关键线索藏在一个细节里：

> **`AModularPawn` 这个类，根本不在引擎的 `ModularGameplay` 插件里！**

我用工具全盘搜索引擎，`ModularGameplay` 插件里**根本没有 `ModularPawn.h`**。它的真实位置是：

```
E:\ue5\LyraStarterGame5.6\LyraStarterGame\Plugins\ModularGameplayActors\
    Source\ModularGameplayActors\Public\ModularPawn.h   ← 在 Lyra 自己的插件里！
```

**看到没？这里又冒出一个新名字：`ModularGameplayActors`（又一个插件）。** 你现在有三个"Modular 开头"的东西，先别晕，下面一张表理清。

---

## 二、三个 "Modular xxx" 到底各是谁（核心对照表）

| 名字 | 是什么 | 在哪 | 能 new / 能继承吗 |
|------|--------|------|------------------|
| **`ModularGameplay`** | 引擎**插件/框架**（提供组件化+状态化底座） | `Engine/Plugins/Runtime/ModularGameplay` | ❌ 是插件，不是类 |
| **`ModularGameplayActors`** | Lyra 的**插件**（放一批可被 GameFeature 扩展的 Actor 基类） | `Lyra/Plugins/ModularGameplayActors` | ❌ 是插件，不是类 |
| **`AModularPawn`** | 一个**具体的 C++ 类**（Pawn 基类） | 上面那个 `ModularGameplayActors` 插件里 | ✅ 是类，能继承 |

> **关键**：前两个是"插件"（一堆文件的集合），第三个是"类"（一个能被继承的具体类型）。

---

## 三、它们仨的真实关系（继承链 + 依赖链）

### 3.1 继承链（类的层面）

看源码，`AModularPawn` 的定义极其简单：

```cpp
// ModularPawn.h（在 Lyra 的 ModularGameplayActors 插件里）
class AModularPawn : public APawn     // ← 就是 APawn 的子类
{
    virtual void PreInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(...) override;
};
```

而 `LyraPawn` 又继承它：

```cpp
// LyraPawn.h
class ALyraPawn : public AModularPawn, public ILyraTeamAgentInterface   // ← 第20行
```

所以继承链是：

```
APawn（引擎原生）
  └─ AModularPawn（Lyra 的 ModularGameplayActors 插件提供）
       └─ ALyraPawn（你的游戏具体 Pawn）
```

### 3.2 依赖链（插件层面）

从 `.cs` 模块依赖能看到：

```
LyraGame（你的游戏模块）
   ├─ 依赖 ModularGameplay          ← 引擎框架（用它的组件系统）
   └─ 依赖 ModularGameplayActors    ← Lyra 插件（用 AModularPawn 等基类）

ModularGameplayActors（Lyra 插件）
   └─ 依赖 ModularGameplay          ← 它建立在引擎框架之上
```

> **也就是说**：`ModularGameplayActors` 这个 Lyra 插件，是**站在引擎 `ModularGameplay` 框架肩膀上**造的一批"现成可用的 Actor 基类"，`AModularPawn` 就是其中之一。

---

## 四、用类比彻底讲透

### 类比：盖房子

| 概念 | 对应 |
|------|------|
| **ModularGameplay（引擎插件）** | **"预制装配式建筑体系"**（一套标准、方法论，教你怎么模块化盖房） |
| **ModularGameplayActors（Lyra 插件）** | **"开发商按这套体系建好的一批样板房"**（拿来就能住的成品户型） |
| **AModularPawn（类）** | **"其中一种户型：标准 Pawn 户型"**（一个具体能住人的房子型号） |
| **ALyraPawn（类）** | **"你在这户型基础上改造的自家房子"**（加了队伍系统的最终版） |

- 你不能"继承"建筑体系（ModularGameplay）——它是规则。
- 你能继承一个具体户型（AModularPawn），然后改造成自家房（ALyraPawn）。
- 样板房（Actors 插件）是按体系（ModularGameplay）造的。

### 类比：编程世界

| 概念 | 对应 |
|------|------|
| ModularGameplay | 一个**框架/库**（如 Spring、React） |
| ModularGameplayActors | 基于该框架写的**一套基础组件包** |
| AModularPawn | 组件包里**某一个具体基类** |

---

## 五、那 `AModularPawn` 到底解决了啥？（为啥要有它）

看它的注释和实现就懂了：

```cpp
/** Minimal class that supports extension by game feature plugins */   // 最小化、支持 GameFeature 扩展的类
```

**作用**：提供一个"干净、最小"的 Pawn 基类，专门设计成**能被 GameFeature 插件安全扩展**。

- 它只重写了 `PreInitializeComponents / BeginPlay / EndPlay` 这几个关键点。
- 目的：让 GameFeature 插件能在这些时机往 Pawn 上"装组件"（配合 ModularGameplay 框架的组件装配系统）。
- 你继承它得到 `ALyraPawn`，再叠加队伍系统（`ILyraTeamAgentInterface`）等业务。

> **一句话**：`AModularPawn` = Epic 给你准备好的"可被 GameFeature 插拔扩展的 Pawn 模板"，你拿来当起点，省得自己处理组件装配时机。

---

## 六、一张图看清全局

```
┌─────────────────────────────────────────────────────────────┐
│  引擎层                                                       │
│                                                               │
│  ModularGameplay（插件/框架）                                  │
│     "组件化 + 状态化"的底层方案                                │
│     提供：UGameFrameworkComponent、Manager、接口…             │
└───────────────────────────┬──────────────────────────────────┘
                            │ 依赖（站在它肩膀上）
                            ▼
┌─────────────────────────────────────────────────────────────┐
│  Lyra 层                                                      │
│                                                               │
│  ModularGameplayActors（Lyra 插件）                            │
│     一批"可被 GameFeature 扩展"的 Actor 基类：                 │
│        ├─ AModularPawn      ★（就是它！APawn 的最小化子类）   │
│        ├─ AModularCharacter                                 │
│        ├─ AModularController                                │
│        └─ AModularPlayerState                               │
└───────────────────────────┬──────────────────────────────────┘
                            │ 继承
                            ▼
┌─────────────────────────────────────────────────────────────┐
│  你的游戏                                                     │
│                                                               │
│  ALyraPawn : public AModularPawn + ILyraTeamAgentInterface   │
│     （在模板基础上加队伍系统等业务）                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`AModularPawn` 在引擎 ModularGameplay 插件里" | ❌ 它在 Lyra 的 **ModularGameplayActors** 插件里 |
| "ModularGameplay 和 ModularGameplayActors 是一个东西" | ❌ 前者是**引擎框架**，后者是 **Lyra 基于它造的基类插件** |
| "AModularPawn 是个框架" | ❌ 它就是一个**具体的类**（APawn 子类），能继承 |
| "我可以直接继承 ModularGameplay" | ❌ 它是插件不是类；你要继承的是它产出的基类（如 AModularPawn） |

---

## 八、总结（一句话记忆）

```
ModularGameplay         = 【引擎框架/插件】抽象方案，不能继承
ModularGameplayActors   = 【Lyra 插件】基于框架造的一批 Actor 基类
AModularPawn            = 【具体类】Actors 插件里的一个 Pawn 基类（能继承）

关系链：
  ModularGameplay(引擎框架)
       ↓ 支撑
  ModularGameplayActors(Lyra插件，造现成基类)
       ↓ 包含
  AModularPawn(具体 Pawn 基类)
       ↓ 继承
  ALyraPawn(你的最终 Pawn)

判断口诀：
  说"一套方案/框架/插件" → ModularGameplay
  说"一批现成基类/插件"   → ModularGameplayActors
  说"能继承的那个类"       → AModularPawn
```

**一句话**：`ModularGameplay` 是**引擎里的底层框架/插件**（抽象方案，不能继承）；`AModularPawn` 是 **Lyra 的 `ModularGameplayActors` 插件里一个具体的 Pawn 基类**（能继承的实体类），它建立在 ModularGameplay 框架之上、专供 GameFeature 扩展。你继承 `AModularPawn` 得到 `ALyraPawn`。**它俩只是名字像，一个是"框架"、一个是"框架产出的一个类"，根本不是一层。**

---

## 九、下一步

- 看 `ModularGameplayActors` 插件里其他基类（Character/Controller/PlayerState）。
- 回顾 GameFeature 如何在 `PreInitializeComponents` 时机往这些 Pawn 装组件。
- 对比 `AModularPawn` 与之前学的 `UPawnComponent` 体系如何配合。
