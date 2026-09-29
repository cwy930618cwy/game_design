# PossessedBy / UnPossessed 详解 —— 角色"被控制/失去控制"时到底在干嘛

> **定位**：逐行拆解 `LyraPawn.cpp` 里最核心的两个函数——`PossessedBy`（被控制器附身）和 `UnPossessed`（失去控制）。它们是整个"队伍系统"运转的开关。
>
> **关联**：
> - [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 队伍 ID 从哪来、委托怎么用
> - [17_生命周期函数PreInitialize与EndPlay详解](./17_生命周期函数PreInitialize与EndPlay详解.md) — 同样是"重写+调Super"套路
> - [19_virtual_override_Super三者区别详解](./19_virtual_override_Super三者区别详解.md) — 为什么要写 `Super::`
>
> **一句话**：**被控制（PossessedBy）= 跟着 Controller 走队伍 + 监听它以后的变化；失去控制（UnPossessed）= 断开监听 + 重置成无队伍。** 这两个函数就是队伍归属的"插拔电源"。

---

## 一、先搞懂背景：什么是 Possess（附身/控制）？

UE 里有个核心概念叫 **Possession（控制）**：

```
AController（控制器：玩家或 AI）  ──附身──►  APawn（角色身体）
        │                                          │
        │  "我接管这个身体"                          │  "我被控制了"
        └──────────────────────────────────────────┘
```

- **`PossessedBy(Controller)`**：一个 Controller "附身"这个 Pawn —— 比如玩家按下开始键，他的 PlayerController 接管了某个角色。
- **`UnPossessed()`**：Controller 放弃这个 Pawn —— 比如角色死亡、玩家退出、切换角色。

> **类比**：Possession 就像"灵魂（Controller）钻进一具身体（Pawn）"。`PossessedBy` 是灵魂入体，`UnPossessed` 是灵魂出窍。

**Lyra 在这两个时机做的事，核心就是："这具身体现在属于哪个队伍？"**

---

## 二、`PossessedBy`：灵魂入体 → 跟着 Controller 走队伍

### 真实源码（逐行注释）

```cpp
void ALyraPawn::PossessedBy(AController* NewController)
{
    // ① 先记住"旧的队伍 ID"（万一后面要广播"从什么变成什么"）
    const FGenericTeamId OldTeamID = MyTeamID;

    // ② 调用父亲 AModularPawn 的版本（Modular 框架的准备工作）
    Super::PossessedBy(NewController);

    // ③ 如果这个 Controller 也实现了"队伍接口"（能提供队伍信息）
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
    {
        // ④ 把 Pawn 的队伍 ID 同步成 Controller 的队伍 ID
        MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();

        // ⑤ 监听这个 Controller 以后队伍变了的通知（关键！）
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
            .AddDynamic(this, &ThisClass::OnControllerChangedTeam);
    }

    // ⑥ 广播"队伍变化了"（从 OldTeamID 变成现在的 MyTeamID）
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

### 一步步在干嘛？

| 步骤 | 代码 | 作用（大白话） |
|------|------|--------------|
| ① | `OldTeamID = MyTeamID` | 拍照留念：记住入体前的队伍，方便后面说"我从 X 变成了 Y" |
| ② | `Super::PossessedBy()` | 让父亲 `AModularPawn` 干它该干的（Modular 框架准备） |
| ③ | `Cast<ILyraTeamAgentInterface>(NewController)` | **类型转换试探**：这个 Controller 有没有"队伍能力"？（见下方重点） |
| ④ | `MyTeamID = ...->GetGenericTeamId()` | **跟队伍**：我的队伍 = 我老板（Controller）的队伍 |
| ⑤ | `...AddDynamic(this, &...::OnControllerChangedTeam)` | **绑监听**：老板队伍以后变了，记得通知我一声 |
| ⑥ | `ConditionalBroadcastTeamChanged(...)` | **广播**：告诉所有监听者"这家伙换队伍了" |

### 🔑 重点理解第 ③ 步的 `Cast`

```cpp
if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
```

- `Cast<>` 是 UE 的**安全类型转换**：尝试把 `AController*` 转成 `ILyraTeamAgentInterface*`。
- 如果这个 Controller **实现了队伍接口**（比如 Lyra 的玩家控制器），转换成功，返回有效指针 → 进 if 干活。
- 如果这个 Controller **没有队伍能力**（比如某个普通 AI 控制器），转换返回 `nullptr`（空指针，等于 false）→ 跳过，啥也不做。
- **好处**：LyraPawn 不强求每个 Controller 都有队伍——有就跟着走，没有也无所谓，不崩溃。

> **记忆**：`Cast` 成功 = "这个老板能提供队伍信息"；失败 = "这老板不管队伍，随它去"。

### 🔑 重点理解第 ⑤ 步的委托绑定

```cpp
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .AddDynamic(this, &ThisClass::OnControllerChangedTeam);
```

- `GetTeamChangedDelegateChecked()` 拿到一个"队伍变化时会响铃的装置"（委托）。
- `AddDynamic(this, &ThisClass::OnControllerChangedTeam)` = **"把这个铃铛接到我的 `OnControllerChangedTeam` 函数上"**。
- 以后 Controller 的队伍一变，就会自动调用 `OnControllerChangedTeam`，让 Pawn 跟着更新。
- 这是 UE 的**动态委托**（`AddDynamic`），可以序列化的那种，比标准 C++ 的回调更灵活。

---

## 三、`UnPossessed`：灵魂出窍 → 断开监听 + 重置队伍

### 真实源码（逐行注释）

```cpp
void ALyraPawn::UnPossessed()
{
    // ① 记住"是谁在控制我"（出窍前要找到那个老板）
    AController* const OldController = GetController();

    // ② 同样先拍下旧队伍的快照
    const FGenericTeamId OldTeamID = MyTeamID;

    // ③ 如果之前的老板有队伍能力……
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
    {
        // ④ ……就把监听拆掉（RemoveAll：移除所有挂在我身上的监听）
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
    }

    // ⑤ 调用父亲 AModularPawn 的版本
    Super::UnPossessed();

    // ⑥ 决定"失去控制后我的队伍该变成啥"（默认变无队伍）
    MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);

    // ⑦ 广播队伍变化
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

### 一步步在干嘛？

| 步骤 | 代码 | 作用（大白话） |
|------|------|--------------|
| ① | `OldController = GetController()` | 抓住当前老板（出窍前先知道是谁在控制我） |
| ② | `OldTeamID = MyTeamID` | 拍快照 |
| ③④ | `Cast` + `RemoveAll(this)` | **拆铃铛**：老板走了，把他那套队伍变化的监听全卸掉 |
| ⑤ | `Super::UnPossessed()` | 让父亲干它该干的 |
| ⑥ | `MyTeamID = DetermineNewTeamAfterPossessionEnds(...)` | **决定新队伍**：默认变"无队伍"，子类可改策略 |
| ⑦ | `ConditionalBroadcastTeamChanged(...)` | 广播"这家伙又换队伍了" |

### 🔑 注意 `RemoveAll(this)` 和 `AddDynamic` 是对称的

- `PossessedBy` 里 `AddDynamic`（接线），`UnPossessed` 里 `RemoveAll`（拆线）。
- **必须对称！** 否则灵魂都走了，铃铛还接着——以后 Controller 队伍一变，会去通知一个已经没人控制的 Pawn，逻辑就乱了。

### 🔑 第 ⑥ 步的钩子函数（hook）

```cpp
MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
```

- `DetermineNewTeamAfterPossessionEnds` 是个 **virtual 钩子**（在 .h 第 46 行定义，默认返回 `NoTeam`）。
- 意思是："失去控制后，队伍怎么办？" —— **默认变无队伍**，但子类可以重写：
  - 想保留原队伍？返回 `OldTeamID`
  - 想变中立阵营？返回某个中立 ID
- 这就是 Lyra 的**可扩展设计**：基类给默认行为，子类按需定制。

---

## 四、两个函数的对称之美

把它们放一起看，会发现**完美的对称结构**：

```
┌─ PossessedBy（入体）──────────────┐   ┌─ UnPossessed（出窍）──────────────┐
│  ① 记旧队伍快照                    │   │  ① 抓住当前 Controller             │
│  ② Super::PossessedBy()           │   │  ② 记旧队伍快照                    │
│  ③ Cast 试探 Controller 有无队伍   │   │  ③ Cast 试探                       │
│  ④ MyTeamID = 老板的队伍           │   │  ④ RemoveAll（拆监听）← 对称！     │
│  ⑤ AddDynamic（绑监听）←          │   │  ⑤ Super::UnPossessed()           │
│  ⑥ Super（注：顺序与上面略不同）   │   │  ⑥ MyTeamID = 钩子决定新队伍       │
│  ⑦ 广播变化                        │   │  ⑦ 广播变化                        │
└───────────────────────────────────┘   └───────────────────────────────────┘
        关键词：跟队伍 + 绑监听                  关键词：拆监听 + 重置队伍
```

> ⚠️ 小细节：`PossessedBy` 里 `Super::` 在**前面**调，`UnPossessed` 里 `Super::` 在**中间**调——顺序不同是因为两件事的前置依赖不一样。但**"先保存旧状态 → 调 Super → 干自己的事 → 广播"** 这个大骨架是一致的。

---

## 五、一张图看懂整个队伍绑定流程

```
        【玩家按下开始键】
               │
               ▼
   PlayerController 附身 LyraPawn
   （引擎自动调用 PossessedBy）
               │
               ├─ Cast：这 Controller 有队伍能力吗？
               │         │
               │    有 ──┴── 无 → 跳过，保持原样
               │         ▼
               │   MyTeamID = Controller 的队伍   ← 跟老板走
               │         │
               │         ▼
               │   AddDynamic：监听老板队伍变化    ← 绑铃铛
               │         │
               ▼         ▼
         广播"队伍变了"（队友血条变色等 UI 更新）
               │
      ═════════╪═════════ 一段时间后 ═════════╪═════════
               │                            │
        【玩家死亡 / 退出 / 切角色】          │
               ▼                            │
   引擎调用 UnPossessed                     │
               │                            │
               ├─ RemoveAll：拆掉监听        ← 拆铃铛（对称）
               │                            │
               ├─ MyTeamID = 钩子决定        ← 默认变无队伍
               │                            │
               ▼                            │
         广播"队伍变了"                      │
                                            │
        【此时这具身体已无队伍归属】◄──────────┘
```

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "PossessedBy 里直接写死队伍 ID" | ❌ 是从 Controller 那里**读**来的，不是写死的 |
| "没写 Cast 也能直接调 GetGenericTeamId" | ❌ 必须先 Cast 确认有队伍能力，否则普通 Controller 会出问题 |
| "UnPossessed 不用拆监听也没事" | ❌ 必须 `RemoveAll`，否则野监听会导致后续逻辑错乱 |
| "失去控制后队伍自动清空" | ⚠️ 不完全对——是 `DetermineNewTeamAfterPossessionEnds` 决定的，默认无队伍，子类可改 |
| "这两个函数是 Lyra 自己瞎写的" | ❌ 是重写 `APawn` 的虚函数（`override`），引擎本来就有这俩时机 |

---

## 七、总结

```
PossessedBy（被控制 / 灵魂入体）：
  1. 跟老板（Controller）走队伍：MyTeamID = 老板的队伍
  2. 绑监听：老板队伍变了，通知我（AddDynamic）
  3. 广播变化

UnPossessed（失去控制 / 灵魂出窍）：
  1. 拆监听：RemoveAll（与上面对称，必须拆！）
  2. 重置队伍：钩子决定，默认变无队伍
  3. 广播变化

核心设计思想：
  - Cast 安全试探 → 不强求每个 Controller 都有队伍，优雅容错
  - Add/Remove 对称 → 有借有还，监听生命周期清晰
  - 钩子函数 → 基类给默认，子类可定制（DetermineNewTeamAfterPossessionEnds）
  - 广播机制 → 队伍一变，全局 UI（队友识别）自动更新
```

**一句话**：这两个函数就是队伍系统的"电源插头"——**插上（PossessedBy）就通电（跟队伍+监听），拔掉（UnPossessed）就断电（拆监听+重置）**。整个队友/敌人识别，都靠这一插一拔维持。

---

## 八、下一步

- [05_LyraTeamAgentInterface队伍接口详解](./05_LyraTeamAgentInterface队伍接口详解.md) — 队伍接口的完整机制
- [19_virtual_override_Super三者区别详解](./19_virtual_override_Super三者区别详解.md) — 为什么要写 `Super::`
- [02_LyraPawn.cpp详解](./02_LyraPawn.cpp详解.md) — 回到 cpp 看其余函数（如 OnControllerChangedTeam / OnRep_MyTeamID）
