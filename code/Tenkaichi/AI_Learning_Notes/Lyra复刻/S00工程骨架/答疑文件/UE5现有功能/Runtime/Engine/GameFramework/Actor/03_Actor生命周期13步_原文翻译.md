# Actor 生命周期 13 步 —— 官方原文翻译 + 说人话

> 依据：`Engine\Classes\GameFramework\Actor.h` 第 **156~193 行**（38 行官方类注释，原文实查）
> 对应：`00_Actor.h分区域讲解.md` 区域 5（说「最值钱的是区域 5 那 38 行注释」指的就是这段）
> 定位：**逐句翻译原文 + 每条补人话解释**。这是引擎注释，只读不改。

---

## 一、这段注释在说什么

开头三句是 Actor 的定义，然后列出了 **13 个按顺序调用的虚函数**——这就是一个 Actor 从「被创建」到「开始游戏」的完整流程。UE 官方把顺序写在这里，是为了让所有写 Actor 的人知道「我重写哪个函数、在哪个时机做哪件事」。

---

## 二、开头三句定义（原文翻译）

| 原文 | 中文 |
|---|---|
| Actor is the base class for an Object that can be placed or spawned in a level. | Actor 是「能被摆进关卡、或能被动态生成出来的对象」的基类。 |
| Actors may contain a collection of ActorComponents, which can be used to control how actors move, how they are rendered, etc. | Actor 可以包含一组 ActorComponent，用它们控制 Actor 怎么移动、怎么渲染等等。 |
| The other main function of an Actor is the replication of properties and function calls across the network during play. | Actor 的另一个主要职责，是在游戏过程中把属性和函数调用复制到网络另一端。 |

🔑 **三句话三个要点**：① 能放进关卡 ② 挂组件干活 ③ 负责联网同步。

---

## 三、13 步生命周期（原文翻译 + 人话）

> 原注释说 "Actor initialization has multiple steps, here's the order of important virtual functions that get called"
> —— Actor 初始化分多步，下面是重要虚函数被调用的顺序。

### 第 1 步 `UObject::PostLoad`

**原文翻译**：对于静态摆在关卡里的 Actor，正常的 `UObject::PostLoad` 会在编辑器和游戏里都被调用；**新生成的 Actor 不走这一步**。

**说人话**：关卡里预先摆好的东西（拖进场景的那种）加载时走；运行时 `SpawnActor` 出来的不走。它是 UObject 层的老流程，不是 Actor 独有的。

### 第 2 步 `UActorComponent::OnComponentCreated`

**原文翻译**：在编辑器或游戏里生成 Actor 时，所有**原生组件**（C++ 里声明的）会调用；**蓝图创建的组件**则是在组件构造时调用；**从关卡加载的组件不走这一步**。

**说人话**：只有「新造出来的组件」才走，关卡里存好的组件不走。区分三种来源：原生 C++ / 蓝图 / 关卡加载。

### 第 3 步 `AActor::PreRegisterAllComponents`

**原文翻译**：对于静态 Actor，以及**带原生根组件**的生成 Actor，现在调用；对于**没有原生根组件的蓝图 Actor**，这些注册函数会晚一点、在构造阶段调用。

**说人话**：注册组件之前的准备动作。大多数 Actor 到这都会走，但纯蓝图搭的 Actor 会延后。

### 第 4 步 `UActorComponent::RegisterComponent`

**原文翻译**：编辑器和运行时所有组件都会注册，这一步**创建它们的物理 / 视觉表现**。这些调用可能分散到多帧，但一定在 `PreRegisterAllComponents` 之后。如果之前调过 `UnregisterComponent` 把组件从世界移除，之后也可能再次调用。

**说人话**：组件真正「生效」的时刻——碰撞体开始有碰撞、网格开始可见，都在这。注意它可能跨帧，别假设同一帧全部完成。

### 第 5 步 `AActor::PostRegisterAllComponents`

**原文翻译**：编辑器和游戏里所有 Actor 都会调用，是**在所有情况下都会调用的最后一个函数**。

**说人话**：注册阶段的收尾。原注释特意强调「all cases」「last」——它是注册流程的铁定终点。

### 第 6 步 `AActor::PostActorCreated`

**原文翻译**：在编辑器或游戏里创建 Actor 时，紧接在构造（construction）之前调用；**从关卡加载的组件不走这一步**。

**说人话**：构造前的最后一口气，马上要进构造脚本了。

### 第 7 步 `AActor::UserConstructionScript`

**原文翻译**：实现了构造脚本（Construction Script）的蓝图会调用它。

**说人话**：蓝图层面的「摆放时就执行」的脚本， Actor 拖进场景、改个参数都会重跑。

### 第 8 步 `AActor::OnConstruction`

**原文翻译**：在 `ExecuteConstruction` 结束时调用，该函数会调用蓝图构造脚本。它在**所有蓝图创建的组件都完全创建并注册完之后**调用。游戏里只对生成的 Actor 调用；编辑器里改蓝图时可能重跑。

**说人话**：构造阶段总收尾。这时候组件全齐了，所以很多「要根据组件算东西」的逻辑放这里比放构造函数安全。

### 第 9 步 `AActor::PreInitializeComponents`

**原文翻译**：在 Actor 的组件调用 `InitializeComponent` 之前调用；**只在游戏过程和某些编辑器预览窗口里调用**。

**说人话**：初始化前的最后准备。注意「only during gameplay」——纯编辑器状态不走。

### 第 10 步 `UActorComponent::Activate`

**原文翻译**：只有当组件设置了 `bAutoActivate` 才会被调用；如果之后手动激活组件，也会再调用。

**说人话**：组件「开机」。默认自动开（bAutoActivate），也可以后期手动开。

### 第 11 步 `UActorComponent::InitializeComponent`

**原文翻译**：只有当组件设置了 `bWantsInitializeComponent` 才会被调用；每次游戏过程只发生**一次**。

**说人话**：组件初始化，一次性。别在这里假设能反复触发。

### 第 12 步 `AActor::PostInitializeComponents`

**原文翻译**：在 Actor 的组件全部初始化完之后调用；**只在游戏过程和某些编辑器预览里调用**。

**说人话**：所有组件就绪后的 Actor 级回调。这里拿组件最安全。

### 第 13 步 `AActor::BeginPlay`

**原文翻译**：关卡开始 Tick 时调用，**只在真正的游戏过程中**。它通常紧跟在 `PostInitializeComponents` 之后，但网络 Actor 或子 Actor 可能被延迟。

**说人话**：开场！游戏正式开始跑。为什么「通常紧跟第 12 步但可能延迟」？因为网络 Actor 要等服务器确认、子 Actor 要等父级先就位。

---

## 四、13 步一图流

```
静态摆在关卡里的 Actor：
  PostLoad(1) → 注册组件(2,3,4,5) → 构造(6,7,8) → 初始化(9,10,11,12) → BeginPlay(13)

运行时 SpawnActor 出来的 Actor：
  （无 PostLoad）→ 注册组件(2,3,4,5) → 构造(6,7,8) → 初始化(9,10,11,12) → BeginPlay(13)
```

区别只有一处：**第一条多一个 `PostLoad`**。

---

## 五、几条要记住的

1. **构造函数里组件还没注册**（第 4 步才注册）—— 想在构造期拿组件、算尺寸，放 `OnConstruction`（第 8 步）或 `PostInitializeComponents`（第 12 步）。
2. **`BeginPlay` 未必紧跟 `PostInitializeComponents`**（第 13 步原注释明说会延迟），网络 Actor 和子 Actor 尤其如此。
3. **第 3/4/6/9/12/13 步都标注了「只在游戏过程 / 某些编辑器预览」**——不要在纯编辑器逻辑里假设它们一定发生。
4. **第 5 步 `PostRegisterAllComponents` 是「所有情况都调用」的注册终点**，写组件注册后的统一收尾逻辑可以放这。
5. **Actor 的三大职责**（开头三句）：能放进关卡、挂组件、网络复制——后面 4700 行全在实现这三件事。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h` 第 156~193 行（原注释逐句实查）
- 官方链接（原注释里的 `@see`）：
  - `https://docs.unrealengine.com/Programming/UnrealArchitecture/Actors`
  - `https://docs.unrealengine.com/Programming/UnrealArchitecture/Actors/ActorLifecycle`
