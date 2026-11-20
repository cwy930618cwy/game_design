# 29 — 构造函数 `ULyraPawnExtensionComponent::ULyraPawnExtensionComponent` 一行行拆

> **定位**：`LyraPawnExtensionComponent.cpp` 第 21~32 行：
>
> ```cpp
> ULyraPawnExtensionComponent::ULyraPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
> 	: Super(ObjectInitializer)                       // ① 调父类构造
> {
> 	PrimaryComponentTick.bStartWithTickEnabled = false;  // ② 默认不启动 Tick
> 	PrimaryComponentTick.bCanEverTick = false;           // ③ 永远不需要 Tick
>
> 	SetIsReplicatedByDefault(true);                      // ④ 默认网络复制
>
> 	PawnData = nullptr;                                  // ⑤ 成员先置空
> 	AbilitySystemComponent = nullptr;                    // ⑥ 成员先置空
> }
> ```
>
> 这是**每个组件出生时最先跑的一段代码**（对象一创建就执行）。这篇逐行讲：每行干嘛、为什么这么设、不设会怎样。
>
> **衔接**：LyraPawn/15~16 篇（FObjectInitializer）、第 22 篇（成员该不该暴露）、第 18 篇（PawnData）。构造函数的这几行和它们全有关。

---

## 〇、30 秒先给答案

| 行 | 干什么 | 大白话 |
|---|---|---|
| `: Super(ObjectInitializer)` | 调父类（UPawnComponent…）构造函数 | 先让"爹"初始化好 |
| `bStartWithTickEnabled = false` | 出生时**不自动**开始 Tick | 别默认每帧叫我 |
| `bCanEverTick = false` | **永远禁止** Tick | 我这辈子都不用每帧干活 |
| `SetIsReplicatedByDefault(true)` | 组件**默认参与网络复制** | 天生就要同步到客户端 |
| `PawnData = nullptr` | 配方指针置空 | 先声明"还没拿到配方" |
| `AbilitySystemComponent = nullptr` | ASC 缓存指针置空 | 先声明"还没挂 ASC" |

---

## 一、先看整体：构造函数什么时候跑？谁调它？

UE 里对象不是用普通 `new` 造的，而是引擎用 **`FObjectInitializer`** 创建（LyraPawn/15~16 篇详解过）。构造函数就是"**对象出生的第一口气**"——在这个时刻：
- 组件还没挂到 Pawn 上；
- 成员刚分配好内存，值还没定；
- 后续所有初始化（OnRegister / BeginPlay）都在这之后才发生。

> **注意**：构造函数只负责"**出生默认值**"，不负责"运行时逻辑"。真正的干活（注册状态机、推进初始化）在 `OnRegister`/`BeginPlay`（第 23 篇讲过）。构造 = 设置"出厂设置"。

---

## 二、逐行拆

### 行① `: Super(ObjectInitializer)` —— 先让父类初始化

```cpp
	: Super(ObjectInitializer)
```

- 这是**初始化列表**：在进入函数体前，先调用**父类**（`UPawnComponent` → `UGameFrameworkComponent` → `UActorComponent` → …）的构造函数。
- 为什么要传 `ObjectInitializer`？因为 UE 对象的创建上下文必须一路传给父类，父类才能正确完成自己的初始化。
- **如果漏了**：编译器也会自动调父类默认构造，但显式传 `ObjectInitializer` 是 UE 标准写法（保证创建上下文一致）。

> **类比**：你（子类）出生前，得先让你爸（父类）把家业（基类成员）安顿好——`Super()` 就是"先让长辈走完流程"。

### 行②③ `bStartWithTickEnabled = false` + `bCanEverTick = false` —— 关闭 Tick

```cpp
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
```

`PrimaryComponentTick` 是 `UActorComponent` 自带的 **Tick 调度器**。两个开关：

| 开关 | 管什么 |
|---|---|
| `bCanEverTick` | 这个组件**到底能不能**每帧 Tick？设 false = **彻底不能** |
| `bStartWithTickEnabled` | 出生时**要不要自动开始** Tick？（前提是能 Tick） |

**为什么总指挥两个都关？** 看它的职责（第 01 篇）：它是个"事件驱动"的协调者——**PawnData 到了、Controller 变了、别的组件状态变了，它才动一下**（这些全是事件回调，不是每帧轮询）。它**根本不需要每帧干活**。

> **教学场景（关 Tick 的意义）**：想象你的角色每帧都要检查"该不该初始化"，那是浪费。总指挥只在"有事发生"时才被叫醒（SetPawnData/HandleControllerChanged/OnActorInitStateChanged），**平时睡着、有事才醒**。关 Tick = 告诉引擎"别每帧叫我，我省电"。
>
> **不关会怎样**？性能浪费——引擎每帧都会考虑要不要调它的 Tick。对成百上千个角色 × 每个都挂着这种组件，就是白烧 CPU。

### 行④ `SetIsReplicatedByDefault(true)` —— 默认网络复制

```cpp
	SetIsReplicatedByDefault(true);
```

- `SetIsReplicatedByDefault` 是 `UActorComponent` 的方法：**设置"这个组件要不要默认参与网络复制"**。
- 设为 `true` = 只要 Owner（Pawn）会复制，这个组件就跟着复制到客户端。

**为什么必须开？** 因为总指挥身上有要同步的数据——**`PawnData`（`.h` L99：`ReplicatedUsing = OnRep_PawnData`）**！服务器把"用了哪份配方"定好后，客户端得收到同一份才能正确初始化（第 01 篇：客户端靠 `OnRep_PawnData` 触发初始化）。

> **不设会怎样？** 组件的 `PawnData` 复制可能不生效——客户端拿不到配方，初始化链永远推不动（卡在"等 PawnData"）。

### 行⑤⑥ `PawnData = nullptr` / `AbilitySystemComponent = nullptr` —— 成员先置空

```cpp
	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
```

把两个核心成员（`.h` L97~103）先清成空指针：

| 成员 | 为什么出生时要置空 |
|---|---|
| `PawnData` | **"我还没拿到配方"**——等 GameMode 之后 `SetPawnData` 才给 |
| `AbilitySystemComponent` | **"我还没挂上 ASC"**——等之后 `InitializeAbilitySystem` 才挂 |

**为什么必须显式置空？** 新分配的对象内存**不是自动清零的**（C++ 不保证）。如果不置空，`PawnData` 里存的是**随机垃圾值**——后面 `if (!PawnData)` 判断会得到错误结果（可能以为有配方、也可能崩溃）。

> **注意**：`PawnData` 用 `UPROPERTY(TObjectPtr)`，UE 对 UPROPERTY 指针的"初始值"其实是 null（构造后），但 Lyra 仍显式写 `= nullptr`——**一是明确意图、二是防某些路径**。这是 UE 源码里常见的防御习惯。

---

## 三、和后续生命周期的关系（构造不是终点）

构造函数只管"出生默认值"，真正的初始化在后面：

```
构造函数（本篇）：设默认值 → 关Tick、开复制、置空成员
    │
    ▼（组件被加到 Pawn 上、注册）
OnRegister（第 23 篇）：注册进状态系统（RegisterInitStateFeature）
    │
    ▼（Pawn BeginPlay）
BeginPlay：报告 Spawned、订阅所有组件状态变化
    │
    ▼（后续各种事件）
SetPawnData / InitializeAbilitySystem / HandleXXX：真正的干活
```

> **记忆**：构造函数像**新员工入职填表**（勾掉不需要的权限、声明还没领到的东西）；`OnRegister`/`BeginPlay` 像**开始上班**；各种 Handle 是**具体工作**。先填表，后上班。

---

## 四、总结一句话

> **构造函数是组件"出生的第一口气"，只管设出厂默认值**：① 先 `Super(ObjectInitializer)` 让父类初始化；②③ 关掉 Tick（`bCanEverTick=false` + `bStartWithTickEnabled=false`），因为总指挥是事件驱动的协调者、不需要每帧轮询，省性能；④ `SetIsReplicatedByDefault(true)` 让组件默认参与网络复制，保证 `PawnData` 能同步给客户端；⑤⑥ 把 `PawnData` 和 `AbilitySystemComponent` 置空——**先声明"还没拿到配方、还没挂 ASC"，等之后 SetPawnData / InitializeAbilitySystem 再填**。真正的初始化逻辑不在这，而在 OnRegister/BeginPlay 及各 Handle 里。

---

## 五、下一步

- 对照 `.h` L97~103 的两个成员声明，理解"为什么它们需要复制/缓存"。
- 深挖 `UActorComponent` 的 `PrimaryComponentTick` 完整结构（还有 TickGroup、TickInterval 等可以配）。
- 看 `SetIsReplicatedByDefault` 与 `GetLifetimeReplicatedProps`/`DOREPLIFETIME` 的关系（第 01 篇的 PawnData 复制）。
