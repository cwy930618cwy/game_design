# `OnControllerChangedTeam` 详解：它是什么？怎么被触发？在干嘛？

> **定位**：讲清楚 `LyraPawn.h` 第 52~54 行这个 `OnControllerChangedTeam` 函数——它是"队伍变化通知"的**接收端（回调函数）**，是理解整个队伍变化流程闭环的关键一环。
>
> **关联**：
> - [28_成员函数指针详解_代码里拿它干嘛去了](./28_成员函数指针详解_代码里拿它干嘛去了.md) — 它的地址被交给委托
> - [29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解](./29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解.md) — 它内部调用的广播
> - [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 谁调用它
> - [32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic](./32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic.md) — 它登记在哪个委托上
>
> **一句话**：`OnControllerChangedTeam` 是一个"**回调函数（槽）**"——它把自己登记到 Controller 的"队伍变化通知器"上，等 Controller 的队伍一变，系统就**自动回头调用它**，让它把新队伍同步到 Pawn 身上。它是"Controller 队伍变化 → Pawn 队伍跟着变"这座桥梁的接收端。

---

## 一、先看它的声明和实现

### 声明（`LyraPawn.h` 第 52~54 行）

```cpp
private:
    UFUNCTION()
    UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);
```

### 实现（`LyraPawn.cpp` 第 101~106 行）

```cpp
void ALyraPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    const FGenericTeamId MyOldTeamID = MyTeamID;          // ① 快照旧队伍
    MyTeamID = IntegerToGenericTeamId(NewTeam);           // ② 更新成新队伍
    ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);  // ③ 广播变化
}
```

---

## 二、它是什么？——一个"回调函数 / 槽"

**回调函数（callback）** = "不是你现在主动调它，而是**你把它登记好，等将来某个事件发生，系统自动来调它**。"

- 名字里的 `On...` 是 UE 的命名惯例，表示"当……发生时"（`OnControllerChangedTeam` = "当 Controller 的队伍变了时"）。
- 它是个普通的成员函数，但它的特殊之处在于：**它的地址被交给了一个委托**（还记得第 28 篇吗？）。

```cpp
// Possessed 里登记的（第 47 行）：
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .AddDynamic(this, &ThisClass::OnControllerChangedTeam);   // ← 把它的地址交出去登记
```

> **类比**：你在快递柜留了个"到货请打这个电话"的号码（把函数地址交出去）。平时它躺着不动，**只有货到了（事件发生），快递柜才打给你（回调它）**。

---

## 三、它是怎么被触发的？（完整链条）

这是理解它的**关键**——它不是被人直接调的，而是被"委托广播"间接触发的：

```
① Controller 的队伍变了
        ↓
② Controller 那头的委托被触发 .Broadcast(对象, 旧队, 新队)
        ↓
③ 委托遍历"联系人名单"，按登记的地址找到 OnControllerChangedTeam
        ↓
④ 自动调用 OnControllerChangedTeam(this, OldTeam, NewTeam)   ← 它在这一步被触发
        ↓
⑤ 它内部把 Pawn 的队伍也跟着更新 + 再广播出去
```

**所以"谁在调它"的答案是**：**Controller 那边的委托（`GetTeamChangedDelegateChecked()`）在广播时，顺着登记的地址回调了它。** 不是你写的代码直接调，是委托机制自动调。

---

## 四、它内部在干嘛？（逐行）

```cpp
void ALyraPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    const FGenericTeamId MyOldTeamID = MyTeamID;                    // ① 记住"我现在的队伍"
    MyTeamID = IntegerToGenericTeamId(NewTeam);                     // ② 把我的队伍改成新队伍
    ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);   // ③ 通知大家"我变了"
}
```

| 行 | 在干嘛 | 为什么 |
|----|--------|--------|
| ① | 快照旧的 `MyTeamID` | 广播需要"旧值→新值"一对，先存旧值 |
| ② | 把 Pawn 自己的队伍更新成 Controller 的新队伍 | **核心**：Controller 换队了，Pawn 要跟着换 |
| ③ | 广播"我的队伍从旧变新" | 让所有监听我的人（UI、计分板）也知道我变了 |

> **一句话**：它的任务就是 **"Controller 换队 → 我（Pawn）跟着换队 → 通知大家"**。

---

## 五、三个参数分别是什么？

| 参数 | 含义 |
|------|------|
| `UObject* TeamAgent` | 发生变化的那个对象（就是 Controller 自己，`this`） |
| `int32 OldTeam` | 变化前的队伍 ID |
| `int32 NewTeam` | 变化后的队伍 ID |

这三个参数**不是随便定的**——它们正好对应委托宏声明时的三个参数（第 33 篇讲的 `DECLARE_..._ThreeParams(..., UObject*, ..., int32, ..., int32, ...)`）。**委托广播时传什么，回调就收什么。**

---

## 六、注意：它和 `OnRep_MyTeamID` 是两回事

别和下面这个混了（第 108 行，紧挨着它）：

```cpp
void ALyraPawn::OnRep_MyTeamID(FGenericTeamId OldTeamID)   // ← 网络复制回调
{
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

| | `OnControllerChangedTeam` | `OnRep_MyTeamID` |
|---|---|---|
| 触发者 | Controller 的委托广播 | 网络复制（服务器改了 `MyTeamID`，同步到客户端） |
| 触发时机 | Controller 队伍变化时 | `MyTeamID` 这个属性被网络同步时 |
| 作用 | 让 Pawn 跟上 Controller 的新队伍 | 客户端收到新队伍数据后也广播一下 |

> **记忆**：`OnController...` 跟的是"Controller 变了"，`OnRep_...` 跟的是"网络同步过来了"。两者都会最终调 `ConditionalBroadcastTeamChanged`，但**触发来源不同**。

---

## 七、一张图看懂它在整个流程中的位置

```
【Controller 侧】                          【Pawn 侧 (ALyraPawn)】

Controller 队伍变化
      │
      ▼
委托 .Broadcast(this, 旧, 新)
      │
      │  顺着登记的地址回调
      ▼
                              OnControllerChangedTeam(旧, 新)   ← 本函数
                                      │
                        ┌─────────────┼─────────────┐
                        ▼                             ▼
                  MyTeamID = 新队伍          ConditionalBroadcastTeamChanged
                  (Pawn 跟上换队)            (再通知所有监听我的人)
```

---

## 八、常见误区

| 误区 | 正确理解 |
|------|---------|
| "它是被某行代码直接调用的" | ❌ 是被委托 `.Broadcast` 间接回调的，没有代码直接写 `OnControllerChangedTeam(...)` |
| "它只是个普通函数，没啥特殊" | ⚠️ 特殊在它被登记到委托上，成了"事件响应槽" |
| "它和 OnRep_MyTeamID 是一回事" | ❌ 前者跟 Controller 变化，后者跟网络复制，触发源不同 |
| "它只改自己的队伍，不广播" | ❌ 它最后调了 `ConditionalBroadcastTeamChanged` 广播变化 |

---

## 九、总结

```
Q：LyraPawn.h 第 52~54 行的 OnControllerChangedTeam 在干嘛？
A：它是一个【回调函数/槽】，负责"Controller 换队 → Pawn 跟着换队 → 通知大家"。

  • 它被 AddDynamic 登记到 Controller 的队伍变化委托上
  • 当 Controller 队伍变化时，委托 Broadcast，自动回调它
  • 它内部：快照旧值 → 更新 MyTeamID 为新队伍 → ConditionalBroadcastTeamChanged 广播

  三个参数(TeamAgent, OldTeam, NewTeam)对应委托宏声明的参数，广播传啥就收啥。

  注意区分：OnControllerChangedTeam(跟Controller变) vs OnRep_MyTeamID(跟网络同步)
```

**一句话**：`OnControllerChangedTeam` 是一个**回调函数（槽）**，被登记到 Controller 的队伍变化委托上。当 Controller 队伍改变时，委托自动回调它，它就把 Pawn 自己的队伍跟着更新，并通过 `ConditionalBroadcastTeamChanged` 广播出去。它是"Controller 换队 → Pawn 跟着换队"这条链路的**接收端**，也是整个队伍变化通知闭环闭合的地方。

---

## 十、下一步

- [28_成员函数指针详解_代码里拿它干嘛去了](./28_成员函数指针详解_代码里拿它干嘛去了.md) — 它的地址被交给委托
- [29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解](./29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解.md) — 它内部调的广播
- [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 谁触发它
- [32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic](./32_类型vs对象_为什么OnTeamChangedDelegate啥都没有却能AddDynamic.md) — 它登记在哪
