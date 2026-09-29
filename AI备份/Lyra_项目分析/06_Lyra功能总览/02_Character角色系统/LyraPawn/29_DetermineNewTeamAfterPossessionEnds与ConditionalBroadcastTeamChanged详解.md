# UnPossessed 里的两个关键方法：`DetermineNewTeamAfterPossessionEnds` 与 `ConditionalBroadcastTeamChanged`

> **定位**：回答一个具体困惑——"**退出控制（UnPossessed）时，为什么还要调 `ConditionalBroadcastTeamChanged` 广播队伍变化？**"。把这两个方法的真实源码、作用、以及它们和 Possessed 的对称关系彻底讲清。
>
> **关联**：
> - [21_PossessedBy与UnPossessed详解_队伍绑定与解绑](./21_PossessedBy与UnPossessed详解_队伍绑定与解绑.md) — 附身/脱离的生命周期
> - [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — Broadcast 通知监听者
> - [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 队伍接口整体
>
> **一句话**：`UnPossessed` 里调 `ConditionalBroadcastTeamChanged` 不是多余，而是**必然**——因为退出控制会让 Pawn 的队伍从"有"变"无"，`DetermineNewTeamAfterPossessionEnds` 算出新队伍后，只要变了就得广播。它和 Possessed 是**对称的一对**：进来跟队伍走、出去恢复无队伍，两次变化都通过同一个方法通知大家。

---

## 一、先看核心困惑

看 `ALyraPawn::UnPossessed()`（`LyraPawn.cpp` 第 52~68 行）：

```cpp
void ALyraPawn::UnPossessed()
{
    AController* const OldController = GetController();

    // Stop listening for changes from the old controller
    const FGenericTeamId OldTeamID = MyTeamID;
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
    {
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
    }

    Super::UnPossessed();

    // Determine what the new team ID should be afterwards
    MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);      // ← 队伍变了！
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);     // ← 所以广播这个变化
}
```

**困惑点**：明明在"退出控制"，为什么最后还要广播队伍变化？

**答案**：因为 **"退出控制"这件事本身就会导致队伍改变**。

- Pawn 被 Controller 附身时，队伍通常**跟着 Controller 走**（见 Possessed 里 `MyTeamID = ControllerAsTeamProvider->GetGenericTeamId()`）。
- 当 Controller 离开（UnPossessed），Pawn **不能再跟那个 Controller 的队伍了**，于是队伍要变回"无队伍"（`NoTeam`）。
- **队伍从"有"变成"无"，这是一个变化** → 必须广播出去，让所有监听者（UI、计分板等）知道"这家伙没队伍了"。

> **一句话**：`ConditionalBroadcastTeamChanged` 不是"退出控制时顺便喊一嗓子"，而是 **"队伍变了，所以要通知大家"**。它和 Possessed 里那次调用是同一套机制——**只要队伍变了，就广播**。

---

## 二、方法一：`DetermineNewTeamAfterPossessionEnds`（决定新队伍）

真实源码（`LyraPawn.h` 第 45~50 行）：

```cpp
// Called to determine what happens to the team ID when possession ends
virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
{
    // This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards...
    return FGenericTeamId::NoTeam;   // ← 默认返回"无队伍"
}
```

### 它在干嘛

> **Controller 离开后，这个 Pawn 应该属于哪个队伍？默认答案是 `NoTeam`（谁都不属于）。**

### 三个关键点

1. **它是 `virtual` 函数** —— 子类可以覆盖，定制自己的规则。
2. **默认实现**：返回 `FGenericTeamId::NoTeam`（恢复成无队伍）。
3. **可扩展**：某个特殊单位如果想"即使 Controller 走了我还留在原队"，子类重写它返回 `OldTeamID` 即可。

> **类比**：员工离职（Controller 离开），公司默认把他的工位清空（`NoTeam`）。但如果是特殊合同工，子公司可以自定义"他离职后工位保留"（返回 `OldTeamID`）。

---

## 三、方法二：`ConditionalBroadcastTeamChanged`（有条件地广播变化）

真实源码（`LyraTeamAgentInterface.cpp` 第 15~25 行）：

```cpp
void ILyraTeamAgentInterface::ConditionalBroadcastTeamChanged(
    TScriptInterface<ILyraTeamAgentInterface> This,
    FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
    if (OldTeamID != NewTeamID)   // ← 关键：只有"真的变了"才广播
    {
        const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID);
        const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

        UObject* ThisObj = This.GetObject();
        UE_LOG(LogLyraTeams, Verbose, TEXT("[%s] %s assigned team %d"),
               *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

        This.GetInterface()->GetTeamChangedDelegateChecked()
            .Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);   // ← 触发所有监听者
    }
}
```

### 它在干嘛

> **如果队伍真的变了（旧 ≠ 新），就广播通知所有监听者；没变就不吭声。**

### 三个关键点

1. **`if (OldTeamID != NewTeamID)`** —— 这就是 "Conditional"（有条件）的含义。
2. **为什么要判一下？** 防止"队伍没变也瞎广播"，浪费性能 + 避免监听者做无用功。
3. **真正干活的是 `.Broadcast(...)`** —— 它遍历所有用 `AddDynamic` 登记的监听者，逐个回调（还记得第 28 篇讲的"联系人名单"吗？这里就是把名单上的人挨个叫来）。

> **类比**：只有当门牌号真的换了，才去通知快递站；要是没换，就别打扰人家。

---

## 四、把整个 UnPossessed 串起来看

```cpp
void ALyraPawn::UnPossessed()
{
    AController* const OldController = GetController();          // ① 记住原来的 Controller

    const FGenericTeamId OldTeamID = MyTeamID;                   // ② 记住现在的队伍（旧值）
    if (auto* Ctrl = Cast<ILyraTeamAgentInterface>(OldController))
    {
        Ctrl->GetTeamChangedDelegateChecked().RemoveAll(this);   // ③ 取消监听：不再听旧 Controller
    }

    Super::UnPossessed();                                        // ④ 让父类完成标准退出流程

    MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);   // ⑤ 决定新队伍（默认 NoTeam）
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);  // ⑥ 因为变了，广播通知
}
```

**逻辑链**：③ 断开监听 → ⑤ 队伍重置为 NoTeam → ⑥ 检测到变化，广播。

---

## 五、和 Possessed 的对称关系（最重要的全局视角）

这两个方法之所以"看起来重复"，是因为 Lyra 的队伍机制是**对称的两端**：

| | Possessed（被附身） | UnPossessed（脱离） |
|---|---|---|
| 队伍来源 | 跟上 Controller 的队伍 | 重置为 NoTeam |
| 监听委托 | `AddDynamic`（开始听） | `RemoveAll`（停止听） |
| 广播变化 | ✅ `ConditionalBroadcastTeamChanged` | ✅ `ConditionalBroadcastTeamChanged` |

```
Possessed：  无队伍 ──跟上Controller──► 有队伍  → 广播"我加入某队了"
UnPossessed：有队伍 ──断开Controller──► 无队伍  → 广播"我退出队伍了"
                          ↑
              两次都是"队伍变了"，都用同一个方法通知大家
```

> **记忆**：**进来跟队伍走、出去恢复无队伍，两次变化都通过同一个 `ConditionalBroadcastTeamChanged` 通知。** 这不是代码啰嗦，而是"变化即广播"这一统一原则的体现。

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "UnPossessed 调广播是多余的" | ❌ 退出控制会导致队伍从有变无，这是真变化，必须广播 |
| "`Conditional` 就是'一定会广播'" | ❌ 加了 `if (旧≠新)` 判断，没变就不广播 |
| "`DetermineNewTeam...` 永远返回 NoTeam" | ⚠️ 默认是，但它是 virtual，子类可覆盖定制规则 |
| "Broadcast 只通知一个人" | ❌ 它是 multicast，通知所有 AddDynamic 登记的监听者 |

---

## 七、总结

```
Q：UnPossessed 为什么还调 ConditionalBroadcastTeamChanged？
A：因为退出控制会让 Pawn 队伍从"有"变"无"，这是真变化，必须通知大家。

两个方法：
  • DetermineNewTeamAfterPossessionEnds(OldTeamID)
      → 决定 Controller 离开后 Pawn 的新队伍，默认返回 NoTeam（virtual，可覆盖）
  • ConditionalBroadcastTeamChanged(this, Old, New)
      → 只有 Old ≠ New 才广播；内部调 Broadcast 通知所有监听者

对称关系：
  Possessed：跟上 Controller 队伍 → 广播
  UnPossessed：重置为 NoTeam → 广播
  两头对称，都是"队伍一变就通知"
```

**一句话**：`UnPossessed` 里调 `ConditionalBroadcastTeamChanged` 是**必然**而非多余——退出控制让队伍从"有"变"无"，`DetermineNewTeamAfterPossessionEnds` 算出新队伍后，只要变了就由 `ConditionalBroadcastTeamChanged`（带"变了才发"的判断）广播给所有人。它与 Possessed 里的调用**对称**，共同构成"变化即广播"的统一机制。

---

## 八、下一步

- [21_PossessedBy与UnPossessed详解_队伍绑定与解绑](./21_PossessedBy与UnPossessed详解_队伍绑定与解绑.md) — 附身/脱离的生命周期
- [24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic](./24_委托绑定详解_GetTeamChangedDelegateChecked与AddDynamic.md) — Broadcast 如何通知监听者
- [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 队伍接口整体
