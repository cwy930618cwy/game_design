# `HasAuthority()` 是什么？谁引进来的？—— 网络权威（服务器权限）判断

> **定位**：回答两个问题——① **"`HasAuthority()` 是什么？"** ② **"它是谁引进来的、从哪来的？"**。以 `LyraPawn.cpp` 第 74 行为切入点，讲透 UE 的"网络权威"概念。
>
> **关联**：
> - [30_LyraTeamAgentInterface三个实现函数详解](./30_LyraTeamAgentInterface三个实现函数详解_Set_Get_GetDelegate.md) — 它出现在 SetGenericTeamId 里
> - [20_LyraPawn.h所有类型的出身_传递依赖全解析](./20_LyraPawn.h所有类型的出身_传递依赖全解析.md) — 类型/方法的传递依赖追溯
> - [03_AModularPawn到底是什么](./03_AModularPawn到底是什么.md) — 继承链（最终通向 AActor）
>
> **一句话**：`HasAuthority()` 是 **`AActor` 自带的一个网络判断函数**——问一句"我现在是不是服务器（有没有拍板权）？"。它不是 Lyra 发明的，而是顺着继承链 `ALyraPawn → AModularPawn → APawn → AActor` 从引擎最顶层的 `AActor` 那里继承来的。

---

## 一、先看它在代码里的位置

```cpp
// LyraPawn.cpp 第 70~89 行，SetGenericTeamId 里
void ALyraPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (GetController() == nullptr)
    {
        if (HasAuthority())          // ← 就是它：只有服务器才能改队伍
        {
            // ... 真正改队伍 ...
        }
        else
        {
            // 客户端想改？报错拒绝
        }
    }
}
```

**它的作用**：在改队伍之前问一句——"**我有没有资格改？**" 只有服务器（有权威）才放行。

---

## 二、`HasAuthority()` 是什么？

### 官方定义（真实源码 `Actor.h` 第 1926~1928 行）

```cpp
/** Returns whether this actor has network authority */
UFUNCTION(BlueprintCallable, Category="Networking")
bool HasAuthority() const;
```

注释就一句话：**"返回这个 Actor 是否拥有网络权威（network authority）。"**

### 人话翻译

> **`HasAuthority()` = "我现在是不是服务器？我有没有最终拍板权？"**
> - 返回 `true`  → 我是服务器（或单机），我说了算。
> - 返回 `false` → 我是客户端，我只能看、不能私自改。

### 为什么游戏里要分"有没有权威"？

因为 UE 是**网络化引擎**，一个游戏可能跑在很多台机器上（一台服务器 + 无数客户端）。像"队伍归属""血量""分数"这种关键数据，**不能让每个客户端随便乱改**，否则就会打架、作弊。所以规则是：

| 角色 | HasAuthority() | 能改关键数据吗 |
|------|---------------|---------------|
| **服务器（Server）** | `true` | ✅ 能，它说了算 |
| **客户端（Client）** | `false` | ❌ 不能，改了也不作数 |
| **单机（Standalone）** | `true` | ✅ 能（单机自己就是权威） |

> **类比**：
> - 服务器 = 公司总部，有权改合同、改薪资。
> - 客户端 = 分公司前台，只能看、能提交申请，但不能直接改正式文件。
> - `HasAuthority()` 就是问："你这儿是不是总部？"

---

## 三、重点：它是"谁引进来的"？—— 继承链追溯

你问"谁引进来的"，这是关键。**`HasAuthority()` 不是 LyraPawn 自己写的，也不是 include 进来的宏，而是通过继承从 `AActor` 那里得来的。**

### 追溯链条

```
ALyraPawn
   ↓ 继承
AModularPawn
   ↓ 继承
APawn
   ↓ 继承
AActor              ← 引擎最顶层的 Actor 基类
   ↓ 这里定义了
bool HasAuthority() const;   （Actor.h 第 1928 行）
```

**所以 `LyraPawn` 能直接调用 `HasAuthority()`，是因为它一路继承了 `AActor` 的所有公有成员函数**——`HasAuthority()` 就是其中之一。

### 这和你第 20 篇学的"传递依赖"是同一道理

- 第 20 篇讲的是"类型/字段的出身"（比如 `FGenericTeamId` 从哪 include 来）。
- 这里讲的是"**方法的出身**"——`HasAuthority()` 不是靠 include 引入的，而是**靠继承**获得的。

> **记忆**：
> - **include** 引入的是"能用某个类型/宏"。
> - **继承** 引入的是"能用父类的成员函数/字段"。
> - `HasAuthority()` 属于后者——**继承自 `AActor`**。

---

## 四、为什么这里要用 `HasAuthority()` 做检查？

回到 `SetGenericTeamId` 的场景：**改队伍是网络敏感操作**。

```cpp
if (HasAuthority())     // 只有服务器能改
{
    MyTeamID = NewTeamID;                      // 改
    ConditionalBroadcastTeamChanged(...);      // 广播给所有人
}
else
{
    // 客户端如果跑到这里，说明出问题了 → 报错
    UE_LOG(..., "except on the authority");
}
```

**逻辑**：
- 队伍数据会被**网络复制（Replicated）**同步到所有客户端。
- 如果允许每个客户端都改自己的队伍，就会出现"我觉得我在红队、你觉得你在蓝队"的混乱。
- 所以规定：**只有服务器（有权威）才能改，改完再同步给所有人**。

> 这也呼应了第 30 篇讲的"`SetGenericTeamId` 两道安检"——`HasAuthority()` 就是第二道安检。

---

## 五、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`HasAuthority()` 是 Lyra 自己写的函数" | ❌ 它是 `AActor` 的内置函数，继承来的 |
| "客户端也能调，只是结果不同" | ⚠️ 能调，但返回 `false`；通常用来做"只有服务器执行"的分支保护 |
| "单机游戏里 `HasAuthority()` 返回 false" | ❌ 单机（Standalone）也返回 `true`（自己就是权威） |
| "它是宏 / 是 include 进来的" | ❌ 既不是宏，也不是 include；是继承来的成员函数 |

---

## 六、总结

```
Q1：HasAuthority() 是什么？
A1：AActor 的网络权威判断函数。
    true  = 我是服务器/单机，有拍板权
    false = 我是客户端，不能私自改关键数据

Q2：谁引进来的？从哪来？
A2：不是 include、不是宏，而是【继承】来的。
    链条：ALyraPawn → AModularPawn → APawn → AActor
    定义在引擎 Actor.h 第 1928 行。

为什么用它：
    改队伍是网络敏感操作，只允许服务器(有权威)修改再同步，
    防止客户端乱改导致数据混乱。
```

**一句话**：`HasAuthority()` 是引擎 `AActor` 提供的网络权限查询——"我是不是服务器、有没有拍板权"。LyraPawn 之所以能用它，是因为**一路继承到了 `AActor`**（不是 include、不是宏）。在 `SetGenericTeamId` 里它充当"第二道安检"：只有服务器才能改队伍，保证网络数据一致。

---

## 七、它内部到底怎么判断的？（实现原理）

前面讲了"它是谁引进来的"（继承），现在讲"它自己内部怎么算出 true/false"。

### 真实实现（`Actor.h` 第 4911~4914 行，内联函数）

```cpp
FORCEINLINE_DEBUGGABLE bool AActor::HasAuthority() const
{
    return (GetLocalRole() == ROLE_Authority);   // ← 就这么一句
}
```

**它做的事极其简单**：拿"我的本地角色（LocalRole）"跟 `ROLE_Authority` 比一下，相等就返回 `true`。

### 它依赖的两个东西

**① `GetLocalRole()` —— 也是内联的（`Actor.h` 第 726 行）：**

```cpp
/** Returns how much control the local machine has over this actor. */
ENetRole GetLocalRole() const { return Role; }   // ← 直接返回成员变量 Role
```

它就是返回 Actor 身上那个叫 `Role` 的成员变量。

**② `Role` —— 一个记录"网络身份"的枚举变量**

每个 Actor 身上都存着一个 `Role`（类型是 `ENetRole` 枚举），它在网络生成时被赋值（`ExchangeNetRoles()`，见 `Actor.cpp` 第 4251 行）。取值有三种：

| `Role` 的值 | 含义 | HasAuthority() 结果 |
|---|---|---|
| `ROLE_Authority` | 我是权威（服务器/单机） | ✅ `true` |
| `ROLE_AutonomousProxy` | 客户端自己控制的（如玩家自己的 Pawn） | ❌ `false` |
| `ROLE_SimulatedProxy` | 客户端模拟的（别人的 Pawn） | ❌ `false` |

### 完整调用链（一眼看懂）

```
HasAuthority()                          // 你调的
   ↓ 内联展开（Actor.h 第 4911 行）
GetLocalRole() == ROLE_Authority
   ↓ 内联展开（Actor.h 第 726 行）
Role == ROLE_Authority                  // 最终就是比这个成员变量
   ↑
Role 在 Actor 网络生成时由 ExchangeNetRoles() 赋值（Actor.cpp 第 4251 行）
```

> **一句话**：`HasAuthority()` 没有任何复杂计算，就是读一个早就被网络系统打好的"身份标记" `Role`，看它是不是 `ROLE_Authority`。

---

## 八、那 `Role` 这个标记什么时候被改？（它的"一生"）

既然 `HasAuthority()` 全靠读 `Role`，那关键就是：**`Role` 到底什么时候、被谁赋值？**

结论：**`Role` 几乎只在"出生/网络生成"那一刻设定，之后基本不再改。** 一共三个时机：

### ① 构造函数里 —— 默认值（最早的一次）

`Actor.cpp` 第 282 行：

```cpp
SetRole(ROLE_Authority);   // ← 每个 Actor 出生时，Role 默认都是 ROLE_Authority
RemoteRole = ROLE_None;
```

**所有 Actor 一出生 `Role` 都是 `ROLE_Authority`（权威）**。这是起点。

### ② 网络生成时 —— `ExchangeNetRoles()`（最关键、定型的一次）

`Actor.cpp` 第 4251 行调用 → 第 4647 行实现：

```cpp
void AActor::ExchangeNetRoles(bool bRemoteOwned)
{
    if (!bExchangedRoles)                 // 只执行一次，防重复
    {
        if (bRemoteOwned)                 // 如果这个 Actor 是"远程拥有的"（属于某客户端）
        {
            Exchange( Role, RemoteRole ); // ← 把 Role 和 RemoteRole 对调！
        }
        bExchangedRoles = true;
    }
}
```

**这就是 `Role` 值"定型"的时刻**：
- 服务器生成的、属于自己的 Actor → 保持 `ROLE_Authority`。
- 客户端拥有的 Actor（`bRemoteOwned=true`）→ `Role` 和 `RemoteRole` 对调，客户端那个变成 `ROLE_AutonomousProxy`。

### ③ 极少见的情况 —— `SetRole()` / `SwapRoles()`

- `SetRole(ENetRole)`（第 6870 行）：直接写 `Role = InRole;`，引擎内部很少用。
- `SwapRoles()`（第 4661 行）：再次对调 Role 和 RemoteRole，用于特殊场景（如某些 Possess 切换）。

> **一句话**：`Role` 在**构造时默认 = Authority**，在**网络生成（`ExchangeNetRoles`）时根据"我是服务器还是客户端、是不是远程拥有"定型**，之后基本不变。

### 时间线图：`Role` 的一生

```
                          【Role 成员变量的变化时间线】

 ┌─────────────┐                                    ┌──────────────────┐
 │  ① 构造出生  │                                    │  HasAuthority()  │
 │             │                                    │   随时读取 Role   │
 │ SetRole(    │                                    │   判断是否权威    │
 │  Authority) │                                    └────────▲─────────┘
 │             │                                             │
 │ Role =      │        一直保持不变（除非极少数 SwapRoles）   │ 读
 │ Authority   │◄────────────────────────────────────────────┘
 └──────┬──────┘
        │
        │  如果是联网游戏，进入网络生成流程
        ▼
 ┌─────────────────────────────────────────┐
 │  ② ExchangeNetRoles(bRemoteOwned)        │  ← 第 4251 行调用
 │     （第 4647 行，只执行一次）            │
 │                                          │
 │   bRemoteOwned == false ?                │
 │   ┌──────────────────────────────────┐   │
 │   │ 是服务器自己的 Actor              │   │
 │   │ → Role 保持 ROLE_Authority        │   │  → HasAuthority = true ✅
 │   └──────────────────────────────────┘   │
 │                                          │
 │   bRemoteOwned == true ?                 │
 │   ┌──────────────────────────────────┐   │
 │   │ 是客户端拥有的 Actor              │   │
 │   │ Exchange(Role, RemoteRole)        │   │  → Role 变成 AutonomousProxy
 │   │ → Role = AutonomousProxy          │   │  → HasAuthority = false ❌
 │   └──────────────────────────────────┘   │
 └─────────────────────────────────────────┘
        │
        ▼
 ┌─────────────────────────────────────────┐
 │  ③ 之后：Role 基本不再改变               │
 │     HasAuthority() 读到的永远是定型的值  │
 └─────────────────────────────────────────┘
```

### 一张表看懂"谁、什么时候、改成什么"

| 时机 | 触发函数 | 改成什么 | 发生几次 |
|------|---------|---------|---------|
| **构造出生** | `SetRole(ROLE_Authority)`（第 282 行） | 一律 `ROLE_Authority` | 1 次（出生即设） |
| **网络生成** | `ExchangeNetRoles()`（第 4647 行） | 服务器保持 Authority；客户端的换成 AutonomousProxy | 1 次（`bExchangedRoles` 防重） |
| **特殊切换** | `SwapRoles()` / `SetRole()`（第 4661/6870 行） | 对调或指定 | 极少 |

> **记忆**：`Role` 像"出生证明上的国籍"——出生（构造）先填个默认，联网后按"你是不是远程拥有"做一次正式登记（`ExchangeNetRoles`），之后就一辈子不变了。`HasAuthority()` 只是随时瞄一眼这张证。

---

## 九、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`HasAuthority()` 是 Lyra 自己写的函数" | ❌ 它是 `AActor` 的内置函数，继承来的 |
| "客户端也能调，只是结果不同" | ⚠️ 能调，但返回 `false`；通常用来做"只有服务器执行"的分支保护 |
| "单机游戏里 `HasAuthority()` 返回 false" | ❌ 单机（Standalone）也返回 `true`（自己就是权威） |
| "它是宏 / 是 include 进来的" | ❌ 既不是宏，也不是 include；是继承来的成员函数 |
| "HasAuthority 内部很复杂" | ❌ 就一句 `return GetLocalRole() == ROLE_Authority;`，读一个身份标记 |

---

## 十、总结

```
Q1：HasAuthority() 是什么？
A1：AActor 的网络权威判断函数。
    true  = 我是服务器/单机，有拍板权
    false = 我是客户端，不能私自改关键数据

Q2：谁引进来的？从哪来？
A2：不是 include、不是宏，而是【继承】来的。
    链条：ALyraPawn → AModularPawn → APawn → AActor
    声明在引擎 Actor.h 第 1928 行。

Q3：内部怎么判断的？
A3：就一句内联 —— return (GetLocalRole() == ROLE_Authority);
    GetLocalRole() 返回成员变量 Role，
    Role 在网络生成时由 ExchangeNetRoles() 打好，
    是 ROLE_Authority 就 true，否则 false。

为什么用它：
    改队伍是网络敏感操作，只允许服务器(有权威)修改再同步，
    防止客户端乱改导致数据混乱。
```

**一句话**：`HasAuthority()` 是引擎 `AActor` 提供的网络权限查询——"我是不是服务器、有没有拍板权"。LyraPawn 之所以能用它，是因为**一路继承到了 `AActor`**（不是 include、不是宏）。它的内部实现只有一句 `return (GetLocalRole() == ROLE_Authority);`——读的是 Actor 身上早已被网络系统打好的 `Role` 身份标记。在 `SetGenericTeamId` 里它充当"第二道安检"：只有服务器才能改队伍，保证网络数据一致。

---

## 十一、下一步

- [30_LyraTeamAgentInterface三个实现函数详解](./30_LyraTeamAgentInterface三个实现函数详解_Set_Get_GetDelegate.md) — 它所在的 SetGenericTeamId
- [20_LyraPawn.h所有类型的出身_传递依赖全解析](./20_LyraPawn.h所有类型的出身_传递依赖全解析.md) — 追溯方法/类型的出身
- [03_AModularPawn到底是什么](./03_AModularPawn到底是什么.md) — 继承链
