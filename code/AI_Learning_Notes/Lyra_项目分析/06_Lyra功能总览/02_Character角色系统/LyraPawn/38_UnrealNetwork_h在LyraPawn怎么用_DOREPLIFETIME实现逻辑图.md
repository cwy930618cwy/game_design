# 38 — `Net/UnrealNetwork.h` 在 LyraPawn.cpp 里怎么用？实现逻辑图解

> **定位**：`LyraPawn.cpp` 第 7 行：
>
> ```cpp
> #include "Net/UnrealNetwork.h"
> ```
>
> 它引入的是 **UE 网络复制（Replication）的宏**——这个文件里最核心的就是 `DOREPLIFETIME` 系列宏。这篇讲清：`LyraPawn.cpp` 里**哪一行在用**、这个宏**展开后干了什么**、完整实现逻辑用图看懂。
>
> **衔接**：LyraPawn 目录第 35/36 篇讲过 `GetLifetimeReplicatedProps` 与 `DOREPLIFETIME`。这篇是在 LyraPawn.cpp **这个具体文件**上，把 include→宏→展开→效果串成完整图。

---

## 〇、30 秒先给答案

| 问题 | 答案 |
|---|---|
| 在哪用？ | `LyraPawn.cpp` 第 24 行：`DOREPLIFETIME(ThisClass, MyTeamID);` |
| 这个宏干嘛？ | 把 `MyTeamID` 登记进"复制花名册"（告诉引擎：这个属性要同步） |
| include 干嘛？ | 引入 `DOREPLIFETIME` 宏及复制系统类型 |
| 最终效果 | 服务器上 `MyTeamID` 变了 → 自动同步给所有客户端 |

---

## 〇点五、先听个故事：网络复制 = 官方广播站发通知

> 网络复制的本质，**就是一个"官方广播站"在向所有"分站"播报"什么变了"**。把整个机制套进一个电台故事里，所有名词一次对上：

### 场景设定

想象 Lyra 对战有一个**官方广播站（服务器）**，以及每个玩家电脑里的**分站（客户端）**。

- 广播站墙上有块**公告栏**，写着每位选手的**队伍颜色**（`MyTeamID`）。
- 分站们**没有自己的公告栏**，它们只靠听广播来知道"谁是什么队"。
- 广播站不会把整面墙都念一遍——**只有哪条变了，才广播哪条。**

### 分三步讲

**第一步：开播前，广播站要申报"我播哪些内容"（= 定义复制列表）**

Lyra 的每个角色在"开播前"都要交一份申报表（`GetLifetimeReplicatedProps`），上面写明"我这角色有哪些信息要对外广播"。

申报表上写这一行（=`DOREPLIFETIME`）：

```
我要对外广播：MyTeamID（队伍颜色）
```

**第二步：游戏进行中，广播站盯着申报表，谁变播谁（= 引擎复制系统）**

某刻角色被红队接管了（`PossessedBy` 改了 `MyTeamID`），广播站一看申报表："哦，`MyTeamID` 在广播清单里，而且它变了！" → 立刻**把这条播出去**（打包发给所有客户端）。

**第三步：分站收到广播，按约定做反应（= OnRep 回调）**

每个分站门口贴着一张"收到某广播该干嘛"的纸条（`.h` 里 `ReplicatedUsing = OnRep_MyTeamID`）。分站听到"`MyTeamID` 变了" → 按纸条行动：更新本地记录 + 敲锣喊一声"队伍变啦！"（`OnRep_MyTeamID` → `ConditionalBroadcastTeamChanged`）。

### 一图看懂"广播站模式"

```
     官方广播站（服务器）                     分站们（客户端们）
 ┌───────────────────────────┐   无线电    ┌──────────────────────┐
 │ 公告栏：MyTeamID=蓝队      │ ──────────► │ 听到"变成红队了"      │
 │                           │             │    │                │
 │ 申报表(GetLifetimeRepl.)： │             │    ▼                │
 │  "我要广播: MyTeamID"      │             │ 按门口纸条(OnRep)行动 │
 │  = DOREPLIFETIME 登记      │             │  更新本地 MyTeamID   │
 │                           │             │  喊"队伍变了！"       │
 │ MyTeamID 变了(红队)        │             │  → 广播给其他系统     │
 │ → 查申报表：在清单里        │             └──────────────────────┘
 │ → 打包播报出去             │
 └───────────────────────────┘
        │ 只有变了才播（省流量）
        │ 谁在申报表里才播（DOREPLIFETIME 决定）
```

### 故事 ↔ 代码名词对照表

| 故事里 | 代码里 |
|---|---|
| 广播站 | 服务器（Authority） |
| 申报表 | `GetLifetimeReplicatedProps` |
| "我要广播 MyTeamID" 这一行 | `DOREPLIFETIME(ThisClass, MyTeamID)` |
| 盯着申报表、谁变播谁 | 引擎复制系统（每次 Net Update） |
| 分站门口"收到该干嘛"的纸条 | `.h` 的 `UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)` |
| 分站听到广播后敲锣 | `OnRep_MyTeamID` 回调 |
| 只播变的（不整墙念） | 复制系统的属性级增量同步 |

> **一句话故事版**：`DOREPLIFETIME` = **广播站在申报表上写"我要对外广播队伍颜色"这一行**；`.h` 的 `ReplicatedUsing` = **分站门口贴的"听到广播怎么反应"的纸条**；服务器每次发现有角色队伍变了，就按申报表把这条播出去，分站听到后按纸条敲锣。**申报表决定"播什么"，纸条决定"收到后做啥"，广播站负责"谁变播谁"。**

---

## 一、LyraPawn.cpp 里到底哪一行在用？（先找到使用点）

整个文件里，`UnrealNetwork.h` 服务的目标只有**一处**——第 20~25 行的 `GetLifetimeReplicatedProps`：

```cpp
void ALyraPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyTeamID);   // ← 第 24 行：就这一句在用！
}
```

而它要复制的属性 `MyTeamID`，声明在 `.h` 第 57~58 行：

```cpp
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)     // ← 网络同步 + 到达时回调 OnRep_MyTeamID
	FGenericTeamId MyTeamID;
```

> **完整拼图**：`.h` 里 `MyTeamID` 标了 `ReplicatedUsing`（"我要同步，同步到了调 OnRep_MyTeamID"）+ `.cpp` 里 `DOREPLIFETIME` 把它登记进复制列表。**两处配合，属性才会真的被网络复制。**

---

## 二、`DOREPLIFETIME` 宏展开后到底干了什么？（实现逻辑）

去引擎源码看真实定义（`UE_5.6/Engine/.../Net/UnrealNetwork.h` L261）：

```cpp
#define DOREPLIFETIME(c,v) DOREPLIFETIME_WITH_PARAMS(c,v,FDoRepLifetimeParams())
```

它转调 `DOREPLIFETIME_WITH_PARAMS`（L252~259）：

```cpp
#define DOREPLIFETIME_WITH_PARAMS(c,v,params) \
{ \
	static_assert(ValidateReplicatedClassInheritance<c, ThisClass>(), ...); \
	FProperty* ReplicatedProperty = GetReplicatedProperty(StaticClass(), c::StaticClass(), GET_MEMBER_NAME_CHECKED(c,v)); \
	RegisterReplicatedLifetimeProperty(ReplicatedProperty, OutLifetimeProps, FixupParams<decltype(c::v)>(params)); \
}
```

**把 `DOREPLIFETIME(ThisClass, MyTeamID)` 手动画展开**，实际变成约 4 步：

```cpp
{
	// ① 编译期检查：MyTeamID 确实是 ThisClass(ALyraPawn) 能访问的复制属性
	static_assert(ValidateReplicatedClassInheritance<ThisClass, ThisClass>(), "...");

	// ② 通过反射找到 MyTeamID 这个属性的"元数据描述"（FProperty*）
	FProperty* ReplicatedProperty =
		GetReplicatedProperty(StaticClass(), ThisClass::StaticClass(), GET_MEMBER_NAME_CHECKED(ThisClass, MyTeamID));

	// ③ 用默认参数（无额外条件）构造一个"复制参数"
	//    并把它 + 找到的属性一起 登记进 OutLifetimeProps（复制花名册）
	RegisterReplicatedLifetimeProperty(ReplicatedProperty, OutLifetimeProps, FDoRepLifetimeParams());
}
```

### 三步各自的意义（大白话）

| 步 | 干的事 | 类比 |
|---|---|---|
| ① static_assert | **编译期验证**：这属性确实存在、可访问 | 报名前先查"这人真有资格" |
| ② GetReplicatedProperty | **按名字找到属性的反射描述**（FProperty） | 查花名册找到"这个人"的档案 |
| ③ RegisterReplicatedLifetimeProperty | 把属性 + 同步参数 **追加到 OutLifetimeProps** | 正式把名字写进"要同步的名单" |

**最终结果**：`OutLifetimeProps` 里多了一条 `MyTeamID` 的登记。之后引擎的复制系统在服务器端每次 net update 时，检查这份名单 → 发现 `MyTeamID` 变了 → 打包发给客户端。

---

## 三、完整实现逻辑图（一张图看懂）

```
LyraPawn.cpp 里让 MyTeamID 网络复制的完整链路
─────────────────────────────────────────────────────────────

【声明侧 .h】                 【登记侧 .cpp】              【引擎复制系统】
UPROPERTY(ReplicatedUsing     DOREPLIFETIME(ThisClass,
  = OnRep_MyTeamID)             MyTeamID);
FGenericTeamId MyTeamID;      │
       │                      │ 宏展开后：
       │                      ▼
       │            GetLifetimeReplicatedProps(OutLifetimeProps) 被引擎调用
       │              ├─ ① static_assert 检查属性合法
       │              ├─ ② 反射找到 MyTeamID 的 FProperty
       │              └─ ③ RegisterReplicatedLifetimeProperty(...)
       │                    → OutLifetimeProps 追加一条 MyTeamID
       │                                │
       ▼                                ▼
     "这个属性要同步"        服务器每次 Net Update 扫名单
                                   │
                                   │ MyTeamID 变了？（如 PossessedBy 改了队伍）
                                   ▼
                          打包 MyTeamID 发送给客户端
                                   │
                                   ▼
                    客户端收到 → 更新 MyTeamID
                                   │
                                   ▼ 触发 ReplicatedUsing 指定的回调
                           OnRep_MyTeamID(OldTeamID)   ← LyraPawn.cpp L108
                                   │
                                   ▼
                    广播队伍变化：ConditionalBroadcastTeamChanged(this, ...)
```

> **读图要点**：
> - `.h` 的 `ReplicatedUsing` = "到达时叫醒哪个回调"；
> - `.cpp` 的 `DOREPLIFETIME` = "把它登记进同步名单"；
> - 引擎复制系统 = 真正的搬运工（服务器打包→发→客户端收→触发 OnRep）。

---

## 四、为什么必须 include `Net/UnrealNetwork.h`？

`DOREPLIFETIME` 宏展开里用到的类型/函数全在引擎这个头文件里：

| 用到的东西 | 来自 |
|---|---|
| `DOREPLIFETIME` / `DOREPLIFETIME_WITH_PARAMS` 宏 | `Net/UnrealNetwork.h` |
| `FLifetimeProperty` | 该头文件引入（第 28 篇讲过它只前向声明够签名，真正用靠这里） |
| `RegisterReplicatedLifetimeProperty` | 该头文件引入 |
| `FDoRepLifetimeParams` | 该头文件引入 |

> **不 include 会怎样？** `DOREPLIFETIME` 是宏——不 include 它根本不存在，第 24 行直接编译错误（`DOREPLIFETIME` 未定义）。

---

## 五、`ReplicatedUsing` vs `DOREPLIFETIME` 的关系（常见困惑）

很多人搞不清这两个：**.h 上标的 `ReplicatedUsing` 和 .cpp 里的 `DOREPLIFETIME` 是不是重复了？**

| | `.h` 的 `ReplicatedUsing = OnRep_X` | `.cpp` 的 `DOREPLIFETIME` |
|---|---|---|
| 管什么 | **到达后调谁**（回调函数名） | **要不要同步**（登记进名单） |
| 能省吗 | 不省——没有它，属性同步到了但没回调 | 不省——没有它，属性根本不进同步名单 |
| 类比 | "收到信后通知谁" | "这封信值不值得寄" |

**两个都要写**：`DOREPLIFETIME` 决定"同步"，`ReplicatedUsing` 决定"同步到了之后做啥"。Lyra 里 `MyTeamID` 两者都配齐了（`.h` L57 + `.cpp` L24）。

---

## 六、总结一句话

> **`Net/UnrealNetwork.h` 在 LyraPawn.cpp 只服务一处——第 24 行 `DOREPLIFETIME(ThisClass, MyTeamID)`**：这个宏展开后（① 编译期验证 → ② 反射找到属性 → ③ 登记进 `OutLifetimeProps`），把 `MyTeamID` 写进"网络同步名单"。之后服务器上队伍一变，引擎复制系统自动把新值发给客户端，客户端更新后触发 `.h` 里 `ReplicatedUsing` 指定的 `OnRep_MyTeamID` 回调去广播队伍变化。**include 它 = 拿到 `DOREPLIFETIME` 宏及复制系统的类型，否则第 24 行直接编译失败。**

---

## 七、下一步

- 对照 LyraPawn/35、36 篇，把 `GetLifetimeReplicatedProps` 的完整机制补全。
- 去引擎看 `RegisterReplicatedLifetimeProperty` 的实现，理解它如何把属性真正挂到 `OutLifetimeProps`。
- 试读 `OnRep_MyTeamID`（LyraPawn.cpp L108~111），理解"复制到达后回调→广播队伍"这最后一步。
