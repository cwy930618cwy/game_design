# ALyraPawn —— LyraPawn.cpp 实现详解（附真实源码+注释）

> **定位**：逐行拆解 `LyraPawn.cpp`。`.h` 负责声明，`.cpp` 才是**真正干活的实现**。
>
> **关联**：[01_LyraPawn.h 详解](./01_LyraPawn.h详解.md)
>
> **一句话**：这个 cpp 的核心就一件事——**管理"队伍 ID"在网络环境下的同步与变化广播**。代码量小，但浓缩了 UE 网络复制 + 委托监听的经典套路。

---

## 一、整体逻辑地图

```
构造函数 ALyraPawn()            → 空的，啥都没做
GetLifetimeReplicatedProps()    → 注册"哪些变量要网络复制"（MyTeamID）
PreInitializeComponents()       → 调 Super（早期准备）
EndPlay()                       → 调 Super（退出清理）
PossessedBy()                   → ★被控制：跟着 Controller 走队伍 + 绑定监听
UnPossessed()                   → ★失去控制：断开监听 + 恢复无队伍
SetGenericTeamId()              → 设置队伍（仅服务器/未控制时允许）
GetGenericTeamId()              → 返回队伍 ID
OnControllerChangedTeam()       → Controller 队伍变了的回调
OnRep_MyTeamID()                → 客户端收到复制数据的回调
```

带 ★ 的是重点，其余都是模板化的标准写法。

---

## 二、逐段源码 + 注释

### ① 包含头文件

```cpp
#include "LyraPawn.h"

#include "GameFramework/Controller.h"   // AController 基类
#include "LyraLogChannels.h"            // Lyra 的日志分类（LogLyraTeams）
#include "Net/UnrealNetwork.h"          // ← 网络复制必需（DOREPLIFETIME）
#include "UObject/ScriptInterface.h"    // 接口指针操作

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPawn)  // UE5 新写法，生成内联相关
```

**要点**：`Net/UnrealNetwork.h` 是网络复制的关键头文件，没有它 `DOREPLIFETIME` 宏用不了。

---

### ② 构造函数（空的）

```cpp
ALyraPawn::ALyraPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)   // 调用父类 AModularPawn 的构造
{
    // 空！逻辑全交给组件和引擎默认值
}
```

**为什么空？** Lyra 的设计哲学：**构造函数尽量简单**，复杂初始化交给组件 + `PossessedBy` 等运行时钩子。这样更安全（避免初始化顺序问题）。

---

### ③ 网络复制注册（重要）

```cpp
void ALyraPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);  // 先让父类注册它的

    DOREPLIFETIME(ThisClass, MyTeamID);   // ← 告诉引擎：MyTeamID 这个变量要网络复制
}
```

**这是理解整个类的关键**：
- `GetLifetimeReplicatedProps` 是 UE 的"复制清单"——引擎问"你有哪些变量要同步给客户端？"，这里回答"**MyTeamID**"。
- `DOREPLIFETIME(类, 变量)` 是最常用的复制宏：服务器改了 `MyTeamID`，引擎自动推给所有客户端。
- 对应 `.h` 里的 `ReplicatedUsing = OnRep_MyTeamID`——客户端收到后触发那个回调。

> 记忆：**服务器改值 → 引擎自动同步 → 客户端触发 OnRep。** 这套机制叫"属性复制（Property Replication）"。

---

### ④ PreInitializeComponents / EndPlay（都是调 Super）

```cpp
void ALyraPawn::PreInitializeComponents()
{
    Super::PreInitializeComponents();   // Modular 框架在这里做组件早期准备
}

void ALyraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);      // 退出时清理
}
```

**要点**：这两个只是"占位 + 转发"。Lyra 重写它们是为了**保证 Modular 框架的生命周期钩子被正确调用**——即使现在没额外逻辑，也为将来扩展留口子。

---

### ⑤ PossessedBy —— 被控制时（★核心）

```cpp
void ALyraPawn::PossessedBy(AController* NewController)
{
    const FGenericTeamId OldTeamID = MyTeamID;   // 记住旧队伍（用于广播）

    Super::PossessedBy(NewController);           // 先调父类

    // 如果这个 Controller 也支持队伍接口，就把 Pawn 的队伍同步成 Controller 的队伍
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(NewController))
    {
        MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();   // ① 同步队伍 ID

        // ② 监听 Controller 未来的队伍变化（Controller 换队，Pawn 也跟着换）
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
            .AddDynamic(this, &ThisClass::OnControllerChangedTeam);
    }

    // ③ 广播"队伍变了"（从 OldTeamID → MyTeamID）
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

**三步走，务必理解**：
1. **同步队伍**：Pawn 的队伍 ID = Controller 的队伍 ID（棋子跟着棋手走）。
2. **绑定监听**：`AddDynamic` 把 `OnControllerChangedTeam` 挂到 Controller 的"队伍变化委托"上——以后 Controller 换队，Pawn 自动跟着换。
3. **广播变化**：通知所有监听者"我的队伍变了"（比如 UI 更新队友标识）。

> `Cast<ILyraTeamAgentInterface>(NewController)` 是安全类型转换——如果 Controller 不支持队伍接口，就不处理，保证健壮性。

---

### ⑥ UnPossessed —— 失去控制时（★核心）

```cpp
void ALyraPawn::UnPossessed()
{
    AController* const OldController = GetController();   // 拿到即将离开的 Controller

    // ① 断开监听（不然 Controller 换队时，已无人控制的 Pawn 还跟着变就错了）
    const FGenericTeamId OldTeamID = MyTeamID;
    if (ILyraTeamAgentInterface* ControllerAsTeamProvider = Cast<ILyraTeamAgentInterface>(OldController))
    {
        ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);  // 移除本对象的所有绑定
    }

    Super::UnPossessed();   // 调父类（此时 GetController() 已变 nullptr）

    // ② 决定失去控制后队伍该变成什么（默认 NoTeam，子类可改）
    MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);

    // ③ 广播变化
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

**和 PossessedBy 正好对称**：
1. **断开监听**：`RemoveAll(this)` 移除之前绑定的回调，防止"幽灵跟随"。
2. **重置队伍**：调用钩子函数 `DetermineNewTeamAfterPossessionEnds`（默认返回无队伍）。
3. **广播变化**：通知队伍变了。

> ⚠️ 注意顺序：先断监听、再调 Super、再改队伍。顺序乱了可能导致广播数据不对。

---

### ⑦ SetGenericTeamId —— 手动设置队伍（有严格限制）

```cpp
void ALyraPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (GetController() == nullptr)   // 情况 A：当前没有被控制
    {
        if (HasAuthority())           // A1：在服务器上 → 允许设置
        {
            const FGenericTeamId OldTeamID = MyTeamID;
            MyTeamID = NewTeamID;
            ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
        }
        else                          // A2：在客户端 → 报错拒绝（只有服务器能改）
        {
            UE_LOG(LogLyraTeams, Error, TEXT("You can't set the team ID on a pawn (%s) except on the authority"), *GetPathNameSafe(this));
        }
    }
    else                              // 情况 B：正被控制着 → 报错拒绝（队伍由 Controller 驱动）
    {
        UE_LOG(LogLyraTeams, Error, TEXT("You can't set the team ID on a possessed pawn (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
    }
}
```

**两个"不能改"的情况**（设计很严谨）：
| 情况 | 能否设置 | 原因 |
|------|---------|------|
| 未控制 + 服务器 | ✅ 可以 | 服务器才有权威 |
| 未控制 + 客户端 | ❌ 报错 | 客户端没权威，改了也不同步 |
| 正被控制 | ❌ 报错 | 队伍由 Controller 说了算，别越权 |

> 这体现了网络游戏的**权威模型（Authority）**：只有服务器能修改关键数据，避免客户端作弊。

---

### ⑧ GetGenericTeamId / GetOnTeamIndexChangedDelegate（简单 getter）

```cpp
FGenericTeamId ALyraPawn::GetGenericTeamId() const
{
    return MyTeamID;   // 直接返回
}

FOnLyraTeamIndexChangedDelegate* ALyraPawn::GetOnTeamIndexChangedDelegate()
{
    return &OnTeamChangedDelegate;   // 返回委托的地址，让别人能 AddDynamic 监听
}
```

**要点**：`GetOnTeamIndexChangedDelegate` 返回的是**委托成员变量的地址**，外部拿到后就能注册自己的回调。

---

### ⑨ OnControllerChangedTeam —— Controller 换队的回调

```cpp
void ALyraPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
    const FGenericTeamId MyOldTeamID = MyTeamID;
    MyTeamID = IntegerToGenericTeamId(NewTeam);   // 把 int 转成 FGenericTeamId
    ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}
```

**触发时机**：Controller 自己的队伍变了（比如玩家换了阵营）→ 因为 PossessedBy 里绑定了监听，这里被回调 → Pawn 跟着换队并广播。

> 注意参数是 `int32`（委托签名如此），用 `IntegerToGenericTeamId` 转换。

---

### ⑩ OnRep_MyTeamID —— 客户端收到复制的回调

```cpp
void ALyraPawn::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
    // 客户端收到服务器的新队伍 ID 后，广播"队伍变了"
    // 让本地的 UI/特效等系统也能响应（比如队友高亮刷新）
    ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}
```

**这是网络复制闭环的最后一步**：
- 服务器改 `MyTeamID` → 引擎同步到客户端 → 客户端自动调这个函数 → 广播变化 → 本地 UI 更新。

> `OldTeamID` 参数是引擎自动传的"变化前的旧值"（复制机制会保存旧值给你对比）。

---

## 三、数据流全景图（网络视角）

```
【服务器】
  Controller 换队
      │
      ▼
  OnControllerChangedTeam()  ← 委托回调
      │  改 MyTeamID
      ▼
  引擎检测到复制变量变化 ──────► 自动同步到所有客户端
      │
      ▼
【客户端】
  OnRep_MyTeamID()  ← 引擎自动调用
      │
      ▼
  ConditionalBroadcastTeamChanged() → 本地 UI/特效响应
```

---

## 四、常见误区

| 误区 | 正确理解 |
|------|---------|
| "构造函数初始化了队伍" | ❌ 空的，队伍靠 PossessedBy 或复制设置 |
| "客户端也能 SetGenericTeamId" | ❌ 只有服务器+未控制时才行 |
| "OnRep 是手动调的" | ❌ 网络复制引擎自动触发 |
| "UnPossessed 不用断监听" | ❌ 必须 RemoveAll，否则"幽灵跟随" |
| "GetLifetimeReplicatedProps 可有可无" | ❌ 不注册就不会同步给客户端 |

---

## 五、总结速查

```
LyraPawn.cpp 核心 = 队伍 ID 的网络同步管理

关键函数：
  PossessedBy     → 跟 Controller 走队伍 + 绑定监听
  UnPossessed     → 断监听 + 恢复无队伍
  SetGenericTeamId→ 仅服务器/未控制时可改（权威模型）
  GetLifetimeReplicatedProps → 注册 MyTeamID 要复制
  OnControllerChangedTeam    → Controller 换队回调
  OnRep_MyTeamID             → 客户端收复制回调

设计精髓：
  ① 权威模型（只有服务器改关键数据）
  ② 委托监听（Controller 换队 → Pawn 跟随）
  ③ 网络复制闭环（服务器改 → 客户端 OnRep → 广播）
```

**一句话**：这个 cpp 虽小，却浓缩了 UE 网络游戏的三大套路——**属性复制（DOREPLIFETIME）、委托监听（AddDynamic）、权威模型（仅服务器可改）**。搞懂它，就理解了 Lyra 里"队伍"这种共享状态是怎么在网络中流转的。

---

## 六、下一步

- 回看 [01_LyraPawn.h 详解](./01_LyraPawn.h详解.md) 对照声明与实现
- 看 [02_PawnExtensionComponent](../02_PawnExtensionComponent.md) —— 更复杂的组件如何协调初始化
- 看 LyraCharacter.cpp —— 同样的模式如何扩展到更复杂的角色
