# ModularGameplayActors 逐类详解 —— 每个基类的源码

> 定位：本文件是 `ModularGameplayActors` 插件的**逐类精读**。
> 前提：这个插件是**从 Lyra 一比一复制来的现成代码**，阶段二的任务是**读懂它**，不是自己重写（类名里没有 `Lyra` 前缀，铁律 24 前缀替换不适用）。
> 源码依据：`Tenkaichi\Plugins\ModularGameplayActors\Source\ModularGameplayActors\`

---

## 一、全景：这个插件一共有几个文件？

```
ModularGameplayActors/
├── ModularGameplayActors.uplugin          # 插件描述（依赖 ModularGameplay）
└── Source/ModularGameplayActors/
    ├── ModularGameplayActors.Build.cs     # 模块依赖
    ├── Public/                            # 7 个基类头文件
    │   ├── ModularCharacter.h
    │   ├── ModularPawn.h
    │   ├── ModularPlayerController.h
    │   ├── ModularPlayerState.h
    │   ├── ModularGameMode.h              # 含两个类
    │   ├── ModularGameState.h             # 含两个类
    │   └── ModularAIController.h
    └── Private/
        ├── ModularGameplayActorsModule.cpp  # 模块入口（空）
        ├── ModularCharacter.cpp
        ├── ModularPawn.cpp
        ├── ModularPlayerController.cpp
        ├── ModularPlayerState.cpp
        ├── ModularGameMode.cpp
        ├── ModularGameState.cpp
        └── ModularAIController.cpp
```

共 9 个基类（7 个文件里 `GameMode.h` 和 `GameState.h` 各含 2 个类）。

---

## 二、它们共同的"套路"（先记住这个，后面就都懂了）

这 9 个基类**没有一个有业务逻辑**，全都遵循同一个模板。它们的职责可以概括成一句话：

> **继承引擎原生类，然后在生命周期钩子里，调用 `UGameFrameworkComponentManager`（框架组件管理器）来"登记自己 / 广播 Ready / 注销自己"，并在合适的钩子里遍历身上挂着的"模块化组件"，挨个通知它们。**

每个基类具体做几件事，取决于它继承的原生类有哪些生命周期钩子。但核心就两条线：

1. **让组件能被挂上来**（登记 + 广播 `NAME_GameActorReady`）
2. **把事件转发给已挂上的组件**（`GetComponents()` 遍历 + 逐个调用）

---

## 三、逐个类精读

### 3.1 AModularCharacter（模块化角色）

**头文件** `ModularCharacter.h`：

```cpp
/** Minimal class that supports extension by game feature plugins */
UCLASS(MinimalAPI, Blueprintable)
class AModularCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UE_API virtual void PreInitializeComponents() override;
    UE_API virtual void BeginPlay() override;
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
```

**实现** `ModularCharacter.cpp`：

```cpp
void AModularCharacter::PreInitializeComponents()
{
    Super::PreInitializeComponents();
    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);  // 登记
}

void AModularCharacter::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);          // 广播 Ready
    Super::BeginPlay();
}

void AModularCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this); // 注销
    Super::EndPlay(EndPlayReason);
}
```

**解读**：只做「登记 → 广播 Ready → 注销」三件事，是最简的基类模板。它没有额外的"转发"逻辑（因为 `ACharacter` 没有 `ReceivedPlayer`、`HandleMatchHasStarted` 这类需要转发的钩子）。

---

### 3.2 AModularPawn（模块化 Pawn）

和 `AModularCharacter` **完全一样**（三件事：登记/广播/注销），只是继承 `APawn`。不再重复贴。

---

### 3.3 AModularAIController（模块化 AI 控制器）

也和 `AModularCharacter` 一样（三件事），继承 `AAIController`。

---

### 3.4 AModularPlayerController（模块化玩家控制器）—— 开始有"转发"了

**头文件** `ModularPlayerController.h`：

```cpp
class AModularPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UE_API virtual void PreInitializeComponents() override;
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // APlayerController 的钩子
    UE_API virtual void ReceivedPlayer() override;
    UE_API virtual void PlayerTick(float DeltaTime) override;
};
```

**实现** `ModularPlayerController.cpp`（关键，注意多出来的"转发"）：

```cpp
void AModularPlayerController::PreInitializeComponents()
{
    Super::PreInitializeComponents();
    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);  // 登记
}

void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this); // 注销
    Super::EndPlay(EndPlayReason);
}

void AModularPlayerController::ReceivedPlayer()
{
    // PlayerController 必须先被分配玩家才能干活，所以 Ready 事件在这里广播（而不是 BeginPlay）
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);

    Super::ReceivedPlayer();

    // 转发：把"收到玩家"事件，转发给身上所有 UControllerComponent
    TArray<UControllerComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UControllerComponent* Component : ModularComponents)
    {
        Component->ReceivedPlayer();
    }
}

void AModularPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    // 转发：每帧 Tick 也转发给所有 UControllerComponent
    TArray<UControllerComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UControllerComponent* Component : ModularComponents)
    {
        Component->PlayerTick(DeltaTime);
    }
}
```

**解读**：
- 因为 `APlayerController` 有 `ReceivedPlayer` 和 `PlayerTick` 两个特殊钩子，所以这里除了"登记/广播/注销"，还多了**"转发"**——把这两个事件转发给身上挂的 `UControllerComponent`。
- 注意它**没有 `BeginPlay`**，Ready 事件改在 `ReceivedPlayer` 里广播（因为玩家控制器要先等玩家进来才有意义）。

---

### 3.5 AModularPlayerState（模块化玩家状态）—— 有 Reset 和 CopyProperties 转发

**头文件** `ModularPlayerState.h`：

```cpp
class AModularPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    UE_API virtual void PreInitializeComponents() override;
    UE_API virtual void BeginPlay() override;
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UE_API virtual void Reset() override;

protected:
    UE_API virtual void CopyProperties(APlayerState* PlayerState);
};
```

**实现** `ModularPlayerState.cpp`：

```cpp
void AModularPlayerState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerState::BeginPlay()
{
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);
    Super::BeginPlay();
}

void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
    Super::EndPlay(EndPlayReason);
}

void AModularPlayerState::Reset()
{
    Super::Reset();

    // 转发：把 Reset 转发给所有 UPlayerStateComponent
    TArray<UPlayerStateComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UPlayerStateComponent* Component : ModularComponents)
    {
        Component->Reset();
    }
}

void AModularPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);

    // 转发：把 CopyProperties 转发给所有 UPlayerStateComponent（玩家重生/无缝传送时复制状态用）
    TInlineComponentArray<UPlayerStateComponent*> PlayerStateComponents;
    GetComponents(PlayerStateComponents);
    for (UPlayerStateComponent* SourcePSComp : PlayerStateComponents)
    {
        if (UPlayerStateComponent* TargetComp = Cast<UPlayerStateComponent>(...))
        {
            SourcePSComp->CopyProperties(TargetComp);
        }
    }
}
```

**解读**：比 Character 多了 `Reset` 和 `CopyProperties` 两个转发。原因：`APlayerState` 有"重置"（玩家死亡重开）和"复制属性"（重生/传送）两个特殊生命周期，需要同步转发给身上的 `UPlayerStateComponent`。

---

### 3.6 AModularGameModeBase / AModularGameMode（模块化游戏模式）

**头文件** `ModularGameMode.h`：

```cpp
/** Pair this with a ModularGameStateBase */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameModeBase : public AGameModeBase
{
    GENERATED_BODY()
public:
    UE_API AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/** Pair this with a ModularGameState */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    UE_API AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
```

**实现** `ModularGameMode.cpp`：

```cpp
AModularGameModeBase::AModularGameModeBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GameStateClass = AModularGameStateBase::StaticClass();    // 用模块化 GameState
    PlayerControllerClass = AModularPlayerController::StaticClass(); // 用模块化 PC
    PlayerStateClass = AModularPlayerState::StaticClass();    // 用模块化 PS
    DefaultPawnClass = AModularPawn::StaticClass();           // 用模块化 Pawn
}

AModularGameMode::AModularGameMode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    GameStateClass = AModularGameState::StaticClass();
    PlayerControllerClass = AModularPlayerController::StaticClass();
    PlayerStateClass = AModularPlayerState::StaticClass();
    DefaultPawnClass = AModularPawn::StaticClass();
}
```

**解读**：这两个类**不登记、不广播、不转发**，它们唯一干的事是——**在构造函数里，把 GameMode 默认会创建的那几个类（GameState/PlayerController/PlayerState/Pawn）都指定成 `AModular*` 版本**。

> 这是最容易被忽略但最关键的一环：有了它，当游戏用 `AModularGameModeBase` 时，它创建出来的 GameState、PlayerController、PlayerState、Pawn **自动都是模块化版本**，整条"模块化"链路就串起来了。

---

### 3.7 AModularGameStateBase / AModularGameState（模块化游戏状态）

**头文件** `ModularGameState.h`（GameState 版本多一个 `HandleMatchHasStarted`）：

```cpp
class AModularGameStateBase : public AGameStateBase
{
    ...
    UE_API virtual void PreInitializeComponents() override;
    UE_API virtual void BeginPlay() override;
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

class AModularGameState : public AGameState
{
    ...
    UE_API virtual void PreInitializeComponents() override;
    UE_API virtual void BeginPlay() override;
    UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
    UE_API virtual void HandleMatchHasStarted() override;  // ← 多出来的
};
```

**实现** `ModularGameState.cpp`（`AModularGameState` 多一个转发）：

```cpp
// ...（Base 版本就是三件事：登记/广播/注销，略）

void AModularGameState::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    // 转发：把"比赛开始"转发给所有 UGameStateComponent
    TArray<UGameStateComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UGameStateComponent* Component : ModularComponents)
    {
        Component->HandleMatchHasStarted();
    }
}
```

**解读**：`AGameState` 有 `HandleMatchHasStarted`（比赛开始）钩子，所以转发给身上的 `UGameStateComponent`。Lyra 里 `ULyraExperienceManagerComponent`（继承 `UGameStateComponent`）就是靠这个自动感知"比赛开始"的。

---

## 四、一张表总结（9 个基类干了什么）

| 基类 | 继承 | 登记/广播/注销 | 额外"转发"的钩子 |
|------|------|---------------|------------------|
| `AModularCharacter` | `ACharacter` | ✅ | 无 |
| `AModularPawn` | `APawn` | ✅ | 无 |
| `AModularAIController` | `AAIController` | ✅ | 无 |
| `AModularPlayerController` | `APlayerController` | ✅（Ready 在 `ReceivedPlayer`） | `ReceivedPlayer`、`PlayerTick` |
| `AModularPlayerState` | `APlayerState` | ✅ | `Reset`、`CopyProperties` |
| `AModularGameModeBase` | `AGameModeBase` | ❌（只指定默认类） | 无 |
| `AModularGameMode` | `AGameMode` | ❌（只指定默认类） | 无 |
| `AModularGameStateBase` | `AGameStateBase` | ✅ | 无 |
| `AModularGameState` | `AGameState` | ✅ | `HandleMatchHasStarted` |

---

## 五、小结（一句话）

这 9 个基类就是**9 个"带插槽的原生类"**。它们自己不干活，只做两件事：

1. **让自己能被挂组件**（登记 + 广播 Ready）
2. **把生命周期事件转发给挂上来的组件**（`GetComponents` 遍历 + 逐个调用）

而 `AModularGameMode` 还额外负责"指定默认类都用模块化版本"，把整条链路闭环。

> 真正"挂什么组件、组件干什么"的逻辑，全在更上层的 Lyra 代码和 Game Feature 插件里（那是 L4/L5 的事）。阶段二先把这个"万能插槽"读懂即可。

---

## 六、下一步

本文是"逐类精读"总览。接下来可以按需要深入：

1. **`UGameFrameworkComponentManager` 总线本身**（它是 `ModularGameplay` 引擎插件提供的，负责真正的"登记/挂载"逻辑）——这是理解整个机制的核心，建议重点讲。
2. 某个具体组件的完整实现（如 `ULyraHealthComponent` 怎么用 `InitializeWithAbilitySystem` 初始化）。

你告诉我下一步想深入哪个，我继续。