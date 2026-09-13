# 14 — INavAgentInterface 导航接口详解

> **定位**：解释 `INavAgentInterface`——**导航代理接口**。`APawn` 实现它，所以 AI 才能让 Pawn 寻路。
>
> **一句话**：`INavAgentInterface` 定义"**一个能被 AI 寻路（Navigation）的 Agent 该提供什么信息**"——比如"你在哪"、"你要怎么走"。`APawn` 实现它，AI 就能指挥 Pawn 沿导航路径移动。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/AI/Navigation/NavAgentInterface.h`

---

## 一、先搞懂背景：UE 的 AI 寻路（Navigation）

UE 的 AI 让角色（Pawn）**自动寻路移动**，靠的是**导航系统（Navigation）**：

```
AI 想让角色移动到某个点
  → 导航系统算出一条路径
  → 角色沿路径走

但导航系统需要知道"角色在哪、角色要往哪偏移"
  → 这些信息由"角色（Pawn）"提供
  → 所以 Pawn 要实现 INavAgentInterface
```

**`INavAgentInterface` = 角色和导航系统之间的"接口协议"**——告诉导航系统"我是谁、我在哪、我怎么走"。

---

## 二、这个接口干嘛的（核心方法）

从源码看，它定义了几个关键方法，都是给导航系统用的：

```cpp
// NavAgentInterface.h（真实源码）

// ① 获取 Agent 的导航属性（移动能力）
ENGINE_API virtual const FNavAgentProperties& GetNavAgentPropertiesRef() const;

// ② 获取 Agent 的位置（★核心：告诉导航系统"我在哪"）
virtual FVector GetNavAgentLocation() const PURE_VIRTUAL(...);

// ③ 获取移动目标时的偏移（作为移动目标时）
virtual FVector GetMoveGoalOffset(const AActor* MovingActor) const { return FVector::ZeroVector; }

// ④ 判断是否正在跟随路径
virtual bool IsFollowingAPath() const { return false; }

// ⑤ 获取路径跟随代理
virtual IPathFollowingAgentInterface* GetPathFollowingAgent() const { return nullptr; }
```

**核心方法**：

| 方法 | 干嘛 | 强制吗 |
|------|------|:---:|
| `GetNavAgentPropertiesRef()` | 导航属性（移动能力） | ❌ 有默认 |
| `GetNavAgentLocation()` | **位置**（"我在哪"） | ✅ 纯虚，必须实现 |
| `GetMoveGoalOffset()` | 移动目标偏移 | ❌ 有默认 |
| `IsFollowingAPath()` | 是否在跟随路径 | ❌ 有默认 |
| `GetPathFollowingAgent()` | 路径跟随代理 | ❌ 有默认 |

---

## 三、为什么 APawn 要实现它（配场景）

**`APawn` 实现 `INavAgentInterface`**，所以 AI 能指挥 Pawn 寻路：

```cpp
// APawn 声明（真实源码 Pawn.h 第 42 行）
class APawn : public AActor, public INavAgentInterface
//                              ↑ 实现导航接口
{
    ...
};
```

**场景：AI 让敌人角色（Pawn）移动到玩家位置**

```
AI 控制器（AIController）说"移动到玩家那"
  → 导航系统算路径
  → 导航系统问角色（Pawn）："你在哪？" → GetNavAgentLocation()
  → 角色告诉它位置
  → 角色沿路径走（GetPathFollowingAgent）
```

**因为 Pawn 实现了 INavAgentInterface**，导航系统才能通过统一接口问它"位置在哪、怎么走"。

---

## 四、为什么 `GetNavAgentLocation()` 是纯虚（必须实现）？

看源码第 33 行：

```cpp
virtual FVector GetNavAgentLocation() const PURE_VIRTUAL(INavAgentInterface::GetNavAgentLocation, return FVector::ZeroVector;);
```

**`PURE_VIRTUAL`** 就是 `= 0` 的 UE 写法（纯虚函数）。所以它**必须被实现**——因为：

**"角色的位置在哪"只有角色自己知道**，接口提供不了通用实现。每个角色（Pawn）必须自己告诉导航系统"我的位置"。

```
为什么必须实现：
  导航系统需要知道"角色在哪"才能算路径
  但"位置"每个角色不同（可能用根组件位置、可能用脚底位置）
  → 接口无法提供统一答案
  → 必须每个 Pawn 自己实现 GetNavAgentLocation()
```

---

## 五、其他方法为什么有默认实现（不强制）？

看源码，大部分方法**有默认实现**（大括号里有内容）：

```cpp
virtual FVector GetMoveGoalOffset(...) const { return FVector::ZeroVector; }  // 默认偏移 0
virtual bool IsFollowingAPath() const { return false; }   // 默认"没在跟随"
virtual IPathFollowingAgentInterface* GetPathFollowingAgent() const { return nullptr; }  // 默认空
```

**为什么不强制**：这些方法有"通用默认答案"，大部分角色用默认就行，只有特殊需要才重写。

```
GetMoveGoalOffset → 默认 0（大部分角色不需要偏移）
IsFollowingAPath → 默认 false（大部分不需要特殊处理）
```

**只有 `GetNavAgentLocation()` 没有通用答案**（位置必须自己知道），所以必须实现。

---

## 六、总结：这个接口在干嘛

```
INavAgentInterface = 导航接口
  APawn 实现它 → AI 能指挥 Pawn 寻路

核心方法：
  GetNavAgentLocation() ← ★必须实现（告诉导航系统"我在哪"）
  GetNavAgentPropertiesRef() ← 可选（移动能力）
  GetMoveGoalOffset() ← 可选（目标偏移）
  IsFollowingAPath() ← 可选（是否在跟随）
  GetPathFollowingAgent() ← 可选（路径跟随代理）

为什么必须实现 GetNavAgentLocation：
  "位置"每个角色不同，接口给不了通用答案，必须自己实现
```

**一句话**：`INavAgentInterface` 是**导航接口**，`APawn` 实现它所以 AI 能指挥寻路。核心是 `GetNavAgentLocation()`（告诉导航系统"我在哪"），**它必须实现**（因为位置只有角色自己知道）；其他方法有默认实现（可选）。

---

## 七、下一步

理解了导航接口，下一步可以看 **`APawn` 怎么具体实现 `GetNavAgentLocation()`**，或深入 AI 导航系统（NavMesh/寻路）。
