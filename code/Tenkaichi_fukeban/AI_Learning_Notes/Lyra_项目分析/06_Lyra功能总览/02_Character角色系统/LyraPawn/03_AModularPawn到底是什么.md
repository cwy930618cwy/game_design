# AModularPawn 到底是什么？（以及为什么不直接继承 APawn / AActor）

> **定位**：彻底讲清 `AModularPawn` 这个类——它从哪来、给了什么能力、和 `APawn`/`AActor`/`UObject` 的关系，以及 Lyra 为什么偏偏选它当基类。
>
> **关联**：
> - [Modular Gameplay 框架在 Lyra 中的应用](../../07_ModularGameplay框架在Lyra中的应用.md)
> - [教学经验记录](./01_Character角色系统教学经验记录.md)
>
> **一句话**：`AModularPawn` = **`APawn` + "运行时用数据( DataAsset )动态挂组件"的能力**。它是个空壳，自己不干活，只负责"让别的组件能往上挂"。

---

## 一、先看继承链：每往上一层就多一项能力

```
UObject                 ← 万物之祖：只是内存里的对象，有反射/GC，没有游戏世界属性
  └─ AActor             ← 加：能放进关卡、有 Transform(位置/旋转)、有生命周期(Tick/BeginPlay)
       └─ APawn         ← 加：能被 Controller "附身控制"（玩家/AI 能操控）
            └─ AModularPawn  ← 加：能用 DataAsset 在运行时动态挂载/替换组件（Modular 框架）
```

| 层级 | 新增能力 | 缺了会怎样 |
|------|---------|-----------|
| `UObject` | 仅是内存对象 | 进不了关卡、没位置、看不见 |
| `AActor` | 放进世界 + 位置 + 生命周期 | 不能被玩家控制 |
| `APawn` | 能被 Controller 附身(Possess) | 加功能只能靠继承或写死在类里 |
| `AModularPawn` | **运行时用数据挂组件** | ——（Lyra 要的就是这个） |

> **关键**：`AModularPawn` 不是凭空冒出来的，它就是 `APawn` 的"增强版"——**继承了 APawn 的全部能力，额外加了模块化拼装能力**。

---

## 二、回答核心疑问：为什么不继承 APawn / AActor？

### 场景回顾

Lyra 的需求是："我要一个能被玩家控制、且能灵活拼装功能的角色基类。"

### 三个候选方案对比

```cpp
// 方案 A：继承 AActor
class ALyraPawn : public AActor { ... }
//   → 还得自己实现 Possess（控制）、移动等一大堆 Pawn 本来的能力。重复造轮子。

// 方案 B：继承 APawn
class ALyraPawn : public APawn { ... }
//   → 有了控制能力，但想"挂组件"得自己手写一套 Modular 框架（状态机、DataAsset 装配…）。

// 方案 C：继承 AModularPawn ✅ Lyra 的选择
class ALyraPawn : public AModularPawn { ... }
//   → 控制能力(APawn 自带) + 挂组件能力(Modular 自带)，开箱即用。
```

| 方案 | 能得到什么 | 要自己补什么 |
|------|-----------|-------------|
| 继承 `AActor` | 只有"进关卡+位置" | 自己实现控制、移动、模块化……（累死） |
| 继承 `APawn` | 控制能力 | 自己实现整套模块化框架 |
| 继承 `AModularPawn` | **控制 + 模块化全都有** | 几乎不用补 |

**结论**：不是 Lyra 偏爱 `AModularPawn`，而是它**正好是"能被控制的 Pawn"里最省事的那个基类**——把模块化的脏活累活都封装好了。

> 类比：你要开一家"能换装的人偶店"。
> - 继承 `AActor` = 从一团棉花开始做（连骨架都没有）
> - 继承 `APawn` = 买了个能动的玩偶，但要自己改装"可换装关节"
> - 继承 `AModularPawn` = 直接买了**带可换装关节的标准骨架**，上手就拼衣服（组件）

---

## 三、`AModularPawn` 到底给了什么（Modular 框架的核心）

`AModularPawn` 本身代码极少（引擎源码里它就是个空壳），真正的能力来自它配套的 **ModularGameplay 框架**。这套框架给你三样东西：

### ① 用 DataAsset 决定"挂哪些组件"

传统写法（硬编码）：

```cpp
// 构造函数里写死每个角色都有哪些组件
ALyraPawn::ALyraPawn() {
    HealthComp = CreateDefaultSubobject<UHealthComponent>(...);
    CameraComp = CreateDefaultSubobject<UCameraComponent>(...);
}
```

Modular 写法（数据驱动）：

```cpp
// 角色类保持干净，组件由一份 DataAsset 配置决定
// 同一套空壳 Pawn，配不同 DataAsset → 变成完全不同的单位
UPROPERTY(EditDefaultsOnly)
TSoftObjectPtr<ULyraPawnData> PawnData;   // 一份"配方表"
```

`ULyraPawnData` 这份"配方表"里列着：这个角色要挂哪些组件、给什么技能、用什么相机……**改配置就能变出不同角色，不用改代码**。

### ② 运行时动态组装

- 组件可以在 **Spawn 时**根据 DataAsset 动态 `NewObject` + 注册上去。
- 甚至能在游戏过程中**替换/增删组件**（比如装备切换）。

### ③ InitState 初始化状态机

- 因为组件是异步、乱序到达的（尤其网络下），Modular 框架提供了 **InitState 四阶段**（Spawned → DataAvailable → DataInitialized → GameplayReady）来保证"所有零件到齐后再统一初始化"。
- 详见 [InitState 初始化状态机详解](../../08_InitState初始化状态机详解.md)。

---

## 四、一张图看懂 AModularPawn 的角色

```
┌─────────────────────────────────────────────┐
│              AModularPawn（空壳骨架）          │
│   提供：APawn 的控制能力 + Modular 的拼装能力   │
└───────────────────┬─────────────────────────┘
                    │ 运行时按 DataAsset 组装
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   ┌─────────┐ ┌─────────┐ ┌─────────┐
   │ 血量组件 │ │ 相机组件 │ │ 技能组件 │  ……
   └─────────┘ └─────────┘ └─────────┘
        ▲           ▲           ▲
        └───────────┴───────────┘
         一份 ULyraPawnData（配方表）决定挂哪些
```

**核心理念**：**组合优于继承**。与其做一个"什么都会的超级角色类"，不如做一个"空壳骨架 + 一堆可插拔组件"。

---

## 五、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`AModularPawn` 很复杂" | 它本身是空壳，复杂的是配套框架的思想 |
| "它替代了 APawn" | 它是 APawn 的子类，**包含** APawn 全部能力 |
| "继承 UObject 也能挂组件" | ❌ 连游戏实体都不是，无从谈起 |
| "DataAsset 是必须的" | 理论上也能纯代码挂组件，但 DataAsset 才是 Modular 的精髓 |
| "只能在构造时挂组件" | 运行时也能动态挂/换 |

---

## 六、源码实现：它到底怎么实现的（真实源码）

> `AModularPawn` 的源码**短到惊人**——因为它就是个空壳，真正的魔法藏在幕后管家手里。

### ① 头文件 `ModularPawn.h`（完整）

```cpp
UCLASS(MinimalAPI, Blueprintable)
class AModularPawn : public APawn     // ← 就继承 APawn，没别的
{
    GENERATED_BODY()
public:
    virtual void PreInitializeComponents() override;   // 只重写了这 3 个
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
```

**整个类就声明了 3 个函数，一个成员变量都没有**——它自己啥都不存。

### ② 实现文件 `ModularPawn.cpp`（完整）

```cpp
void AModularPawn::PreInitializeComponents()
{
    Super::PreInitializeComponents();
    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
    //                       ↑ 把自己注册给"组件管家"
}

void AModularPawn::BeginPlay()
{
    // 发一个"我准备好了"的事件
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);
    Super::BeginPlay();
}

void AModularPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
    //                       ↑ 退出时把自己从管家那里注销
    Super::EndPlay(EndPlayReason);
}
```

### ③ 关键：魔法全在 `UGameFrameworkComponentManager`

`AModularPawn` 自己什么都不干，它的三个函数全是**调用同一个幕后管家** `UGameFrameworkComponentManager`：

```
AModularPawn（空壳）
    │  只做 3 件事：注册 / 发事件 / 注销
    ▼
UGameFrameworkComponentManager（真正的组件管家，单例）
    │  负责：管理所有 Modular 组件的注册、装配、生命周期
    ▼
你的各种组件（血量/相机/技能…）
```

| 时机 | 调用 | 作用 |
|------|------|------|
| `PreInitializeComponents` | `AddGameFrameworkComponentReceiver(this)` | **报到**："我出生了，把我登记在册，该给我装组件了" |
| `BeginPlay` | `Send...Event(NAME_GameActorReady)` | **广播**："我零件到齐了，大家可以开始初始化了"（触发 InitState 推进） |
| `EndPlay` | `RemoveGameFrameworkComponentReceiver(this)` | **销号**："我要没了，把挂在我身上的组件清理掉" |

### ④ 整体机制图

```
┌──────────────────────────────────────────────┐
│  AModularPawn（空壳，就 3 个函数）              │
│   PreInitializeComponents → 报到（注册）       │
│   BeginPlay               → 广播（就绪）       │
│   EndPlay                 → 销号（清理）       │
└───────────────────┬──────────────────────────┘
                    │ 全靠这三个调用
                    ▼
┌──────────────────────────────────────────────┐
│  UGameFrameworkComponentManager（组件管家）     │
│   - 维护一份"谁需要装组件"的登记表              │
│   - 收到"报到"→ 按 DataAsset 给这个 Pawn 装组件 │
│   - 收到"就绪"→ 通知各组件初始化（InitState）   │
│   - 收到"销号"→ 拆组件、清现场                  │
└──────────────────────────────────────────────┘
```

### ⑤ "能挂组件"到底是怎么实现的

不是 `AModularPawn` 有魔法，而是它**在合适的时机（出生/就绪/死亡）向管家报告**，由管家完成真正的组装：

1. **出生时**（PreInitializeComponents）：登记 → 管家知道"有个新 Pawn 要装组件"
2. **就绪时**（BeginPlay）：广播 → 管家推动 InitState，让组件们有序初始化
3. **死亡时**（EndPlay）：注销 → 管家回收组件

> **类比**：`AModularPawn` 是酒店前台的一个"入住登记铃"——按一下（报到），后台（管家）就知道该安排房间、送行李、办手续了。前台铃本身不会订房，但它触发了整个流程。

---

## 七、总结速查

```
AModularPawn = APawn + "用 DataAsset 运行时挂组件"的能力

继承链：UObject → AActor → APawn → AModularPawn
        (对象)   (进关卡)  (能控制)  (能拼装)

为什么不继承更底层的：
  ├─ AActor：还得自己实现控制、移动（重复造轮子）
  └─ APawn ：得自己实现整套模块化框架
  └─ AModularPawn：控制 + 模块化，开箱即用 ✅

Modular 框架给的三样东西：
  ① DataAsset 决定挂哪些组件（数据驱动）
  ② 运行时动态组装
  ③ InitState 初始化状态机（保证乱序安全）

核心理念：组合优于继承
```

**一句话**：`AModularPawn` 就是 **"能被玩家控制的 Pawn" 里最省事的基类**——它把"用数据动态拼装组件"这套 Modular 框架打包好送给你。Lyra 选它，是因为做"灵活可拼装的角色"时，它能让你**少写一大堆重复代码**，直接把精力放在业务上。

---

## 八、下一步

- [Modular Gameplay 框架在 Lyra 中的应用](../../07_ModularGameplay框架在Lyra中的应用.md) — 看 Lyra 具体怎么用这套框架
- [InitState 初始化状态机详解](../../08_InitState初始化状态机详解.md) — 搞懂组装时序
- [LyraPawn 源码详解](./LyraPawn/) — 看真实代码如何落地
