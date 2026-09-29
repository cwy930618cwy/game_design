# ILyraTeamAgentInterface 的三个实现函数：`SetGenericTeamId` / `GetGenericTeamId` / `GetOnTeamIndexChangedDelegate`

> **定位**：讲清楚 `LyraPawn.h` 第 38~42 行这三个"接口实现函数"各自在干嘛、为什么要这么写。它们是实现 `ILyraTeamAgentInterface` 接口的关键三件套。
>
> **关联**：
> - [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 接口整体定义
> - [29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解](./29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解.md) — 队伍变化广播
> - [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 委托怎么用
> - [10_接口继承接口详解](./10_接口继承接口详解.md) — 接口如何实现
>
> **一句话**：这三个函数是 `ALyraPawn` 对"队伍接口"的答卷——`GetGenericTeamId` 负责"读"（我属于哪队），`SetGenericTeamId` 负责"写"（把我换到某队，且带一堆安全检查），`GetOnTeamIndexChangedDelegate` 负责"给联系方式"（把队伍变化的通知器交出去）。

---

## 一、先看这三行声明（LyraPawn.h 第 38~42 行）

```cpp
//~ILyraTeamAgentInterface interface
UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
UE_API virtual FGenericTeamId GetGenericTeamId() const override;
UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
//~End of ILyraTeamAgentInterface interface
```

**一眼能看出的信息**：
- 三个都带 `virtual ... override` —— 说明它们是**覆盖接口里的函数**（还记得第 19 篇讲的 `override` 表态吗）。
- 它们的存在意义：**接口规定"你必须提供这三个能力"，LyraPawn 来具体实现**。

先回顾接口那边是怎么"出题"的（`LyraTeamAgentInterface.h`）：

```cpp
virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }  // 接口给的默认（返回空）
```

接口说："想要队伍功能？你得告诉我：① 怎么读队伍、② 怎么改队伍、③ 队伍变了怎么通知我。" LyraPawn 就是来回答这三问的。

---

## 二、函数一：`GetGenericTeamId`（读——我属于哪队）

真实源码（`LyraPawn.cpp` 第 91~94 行）：

```cpp
FGenericTeamId ALyraPawn::GetGenericTeamId() const
{
    return MyTeamID;   // ← 就一行：把当前队伍吐出去
}
```

### 它在干嘛

> **"别人问我'你属于哪个队伍'，我就回答 `MyTeamID`。"**

### 关键点

- **极简**——只有一个 `return MyTeamID;`。它就是个" getter（取值器）"。
- **`const` 修饰**：承诺"这个函数不会修改 Pawn 的任何数据"（只读不写）。
- 它是接口要求的标准能力——外部系统（计分板、敌我识别等）靠它知道"这家伙是哪队的"。

> **类比**：别人问你"你工号多少"，你答"我是 A101"。就这么简单直接。

---

## 三、函数二：`SetGenericTeamId`（写——把我换到某队，重点！）

真实源码（`LyraPawn.cpp` 第 70~89 行）——**这是三个里最值得讲的**：

```cpp
void ALyraPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (GetController() == nullptr)            // 情况A：没有被 Controller 附身
    {
        if (HasAuthority())                    // 情况A1：有权威（服务器）
        {
            const FGenericTeamId OldTeamID = MyTeamID;
            MyTeamID = NewTeamID;              // 改队伍
            ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);  // 广播变化
        }
        else                                   // 情况A2：没权威（客户端）
        {
            UE_LOG(LogLyraTeams, Error, TEXT("...except on the authority"));  // 报错拒绝
        }
    }
    else                                       // 情况B：正被 Controller 附身
    {
        UE_LOG(LogLyraTeams, Error, TEXT("...it's driven by the associated controller"));  // 报错拒绝
    }
}
```

### 它在干嘛

> **"把我换到某个队伍。但没那么容易——得先过两道安检。"**

### 两道安全检查（为什么这么严？）

| 检查 | 代码 | 不通过会怎样 | 为什么 |
|------|------|-------------|--------|
| **① 不能被 Controller 附身** | `if (GetController() == nullptr)` | 否则报错："被控制的 Pawn 不能手动改队伍，队伍由 Controller 决定" | 一个有主的 Pawn，队伍跟着主人走，不允许外界强行改 |
| **② 必须在服务器（有权威）** | `if (HasAuthority())` | 否则报错："只能在权威端改队伍" | 队伍是**网络同步**的关键数据，只能由服务器拍板，客户端不许私自改 |

只有**两道都通过**（没被控制 + 是服务器），才真正执行：

```cpp
const FGenericTeamId OldTeamID = MyTeamID;     // 快照旧值（还记得第29篇的"先快照"吗）
MyTeamID = NewTeamID;                          // 改成新队伍
ConditionalBroadcastTeamChanged(...);          // 变了就广播
```

> **类比**：你想改自己的部门（队伍），但——
> - 如果你已经有直属领导（被 Controller 附身），那部门由领导定，你不能自己改；
> - 如果是在分公司（客户端）瞎改不算数，必须总公司（服务器）批准。
> 两个条件都满足，才能真改，并通知大家。

### 这里又出现 `ConditionalBroadcastTeamChanged`

和第 29 篇是同一个函数——**队伍一变就广播**。注意它依然**先快照旧值再改**，顺序不能反。

---

## 四、函数三：`GetOnTeamIndexChangedDelegate`（给联系方式——把通知器交出去）

真实源码（`LyraPawn.cpp` 第 96~99 行）：

```cpp
FOnLyraTeamIndexChangedDelegate* ALyraPawn::GetOnTeamIndexChangedDelegate()
{
    return &OnTeamChangedDelegate;   // ← 把"队伍变化通知器"的地址交出去
}
```

### 它在干嘛

> **"别人想知道'队伍变了该怎么通知我'，我就把那个通知器的门牌号给他。"**

### 关键点

- 返回的是**指针**（`...Delegate*`）——指向 Pawn 内部那个叫 `OnTeamChangedDelegate` 的委托成员。
- 别人拿到它后，就能用 `AddDynamic` 登记监听（还记得第 24/28 篇吗？）。
- 它是接口要求的：接口里默认返回 `nullptr`（没有通知器），LyraPawn 真正实现成"返回我的通知器"。

### 它和 `GetTeamChangedDelegateChecked` 的关系（容易混）

```
GetOnTeamIndexChangedDelegate()      → 返回 &OnTeamChangedDelegate（裸指针，可能为空的"门牌号"）
        ↓ 被谁调用？
GetTeamChangedDelegateChecked()      → 拿到指针后 check(它不为空)，再解引用返回引用
```

回忆第 24 篇：`GetTeamChangedDelegateChecked()` 内部就是调了 `GetOnTeamIndexChangedDelegate()` 拿到指针，然后 `check(Result)` 确保非空，再 `return *Result`。**一个是"给门牌号"，一个是"确认门牌存在后开门"**。

> **类比**：
> - `GetOnTeamIndexChangedDelegate` = 把客服电话号码条递给你（可能有时候没人值班，号码为空）。
> - `GetTeamChangedDelegateChecked` = 确认有人值班后，把电话听筒递给你（保证一定打得通）。

---

## 五、三个函数合起来 = 一套完整的"队伍自助服务"

| 函数 | 职责 | 类比 |
|------|------|------|
| `GetGenericTeamId()` | **读**：我属于哪队 | 查工号 |
| `SetGenericTeamId()` | **写**：把我换队（带安检） | 申请转部门（要审批） |
| `GetOnTeamIndexChangedDelegate()` | **通知**：队伍变了联系我 | 留客服电话 |

外部系统只要拿着这三样，就能完整地"查询队伍、改变队伍、订阅队伍变化"——这正是"接口"的意义：**定义一组标准操作，让任何实现了它的类都能被统一对待**。

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`SetGenericTeamId` 随便什么时候都能改队伍" | ❌ 必须没被 Controller 附身 + 在服务器端，否则报错拒绝 |
| "`GetGenericTeamId` 会改队伍" | ❌ 它是 `const`，只读不写 |
| "`GetOnTeamIndexChangedDelegate` 和 `GetTeamChangedDelegateChecked` 一样" | ❌ 前者给裸指针（可能空），后者确认非空后给引用 |
| "客户端也能改队伍" | ❌ `HasAuthority()` 检查会拦住，只有服务器能改 |

---

## 七、总结

```
Q：LyraPawn.h 第 38~42 行这三个函数在干嘛？
A：它们是 ALyraPawn 对"队伍接口"的三件实现：

  • GetGenericTeamId() const
      → 读：直接 return MyTeamID（我是哪队）

  • SetGenericTeamId(NewTeamID)
      → 写：把我换队，但先过两道安检：
          ① 没被 Controller 附身  ② 在服务器端(HasAuthority)
          都通过才改 + 快照旧值 + ConditionalBroadcastTeamChanged 广播

  • GetOnTeamIndexChangedDelegate()
      → 通知：返回 &OnTeamChangedDelegate（把队伍变化通知器交出去）
      （GetTeamChangedDelegateChecked 拿它做 check 后再用）

三者合起来 = 查询/改变/订阅 队伍的一套完整服务。
```

**一句话**：这三个 `virtual ... override` 函数是 LyraPawn 对"队伍接口"的标准答卷——`Get` 负责读、`Set` 负责写（还自带"没被控制 + 服务器权威"两道安检）、`GetDelegate` 负责把变化通知器交出去。看懂这一组，就看懂了 Lyra 队伍系统的对外入口。

---

## 八、下一步

- [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 接口整体定义
- [29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解](./29_DetermineNewTeamAfterPossessionEnds与ConditionalBroadcastTeamChanged详解.md) — 队伍变化广播
- [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — 委托怎么用
- [10_接口继承接口详解](./10_接口继承接口详解.md) — 接口如何实现
