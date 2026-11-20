# ModularGameplayActors 具体怎么用？—— 看源码的真实例子

> 回答：光说"模块化、可扩展"太泛了。本文用**源码里的真实类**，把整条链路串起来看。
> 源码依据：`Tenkaichi\Plugins\ModularGameplayActors\` + `LyraStarterGame\Source\LyraGame\`（Lyra 真实继承关系）。

---

## 一、先看全局：谁是"插槽"，谁是"插上去的卡"

`ModularGameplayActors` 这个插件里**没有一行游戏逻辑**，它只定义了 7 个"空壳基类"。真正的主角是两拨人：

| 角色 | 是谁 | 例子（Lyra 真实类） |
|------|------|---------------------|
| **插槽（Actor 基类）** | `AModular*` 系列，只做"登记+广播" | `ALyraCharacter`、`ALyraPlayerState`、`ALyraGameMode`… |
| **插上去的卡（组件）** | `UGameFrameworkComponent` 系列子类 | `ULyraHealthComponent`、`ULyraWeaponStateComponent`、`ULyraExperienceManagerComponent`… |

**关键**：`AModular*` 基类存在的唯一目的，就是让那些"卡"（组件）能被**动态插到它身上**，而且 Actor 自己不用知道卡是谁、有几张。

---

## 二、真实例子 1：Lyra 的角色类，继承的是 `AModularCharacter`

看 `LyraGame\Character\LyraCharacter.h` 第 98 行：

```cpp
class ALyraCharacter : public AModularCharacter,        // ← 模块化基类（不是引擎原生的 ACharacter）
                       public IAbilitySystemInterface,
                       public IGameplayCueInterface,
                       public IGameplayTagAssetInterface,
                       public ILyraTeamAgentInterface
```

Lyra 的五个核心类，**继承的全是 `AModular*`，没有一个直接继承引擎原生类**：

| Lyra 类 | 继承的基类 | 源文件 |
|---------|-----------|--------|
| `ALyraCharacter` | `AModularCharacter` | `Character\LyraCharacter.h:98` |
| `ALyraPawn` | `AModularPawn` | `Character\LyraPawn.h:20` |
| `ALyraGameMode` | `AModularGameModeBase` | `GameModes\LyraGameMode.h:37` |
| `ALyraGameState` | `AModularGameStateBase` | `GameModes\LyraGameState.h:27` |
| `ALyraPlayerState` | `AModularPlayerState` | `Player\LyraPlayerState.h:51` |

> 这就是 `AModular*` 的"地基"意义：Lyra 上层所有关键类都长在它上面。

---

## 三、真实例子 2：`AModular*` 到底做了什么（三个动作）

看 `ModularPlayerState.cpp`（所有 `AModular*` 都是同一套模板，只是组件类型不同）：

```cpp
void AModularPlayerState::PreInitializeComponents()
{
    Super::PreInitializeComponents();
    // 动作①：登记 —— 告诉"组件管理器"我是个可扩展的目标
    UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AModularPlayerState::BeginPlay()
{
    // 动作②：广播 —— 大喊"我 Ready 了，谁有组件要挂赶紧来"
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);
    Super::BeginPlay();
}

void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 动作③：注销
    UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
    Super::EndPlay(EndPlayReason);
}
```

三个动作都是**调 `UGameFrameworkComponentManager`（框架组件管理器）**，这个管理器才是真正的"插槽总线"。

---

## 四、真实例子 3：遍历"插上去的卡"（这是最实际的用法）

`AModular*` 不只负责登记，它还会在**生命周期钩子里，把身上所有"模块化组件"都揪出来，挨个通知一遍**。

看 `ModularPlayerController.cpp`：

```cpp
void AModularPlayerController::ReceivedPlayer()
{
    // 先广播 Ready 事件（让组件能被挂上来）
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
        this, UGameFrameworkComponentManager::NAME_GameActorReady);

    Super::ReceivedPlayer();

    // 然后：把身上所有 UControllerComponent 揪出来，挨个调 ReceivedPlayer()
    TArray<UControllerComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UControllerComponent* Component : ModularComponents)
    {
        Component->ReceivedPlayer();
    }
}
```

**这就是"扩展"的实际含义**：`ALyraPlayerController` 继承 `AModularPlayerController`，而它身上挂的 `ULyraWeaponStateComponent`、`ULyraQuickBarComponent` 这些"卡"，**不需要 PlayerController 写死调用**，基类的 `ReceivedPlayer()` 会自动把它们挨个通知到。

再举一个更有代表性的例子——`ModularGameState.cpp` 里的 `HandleMatchHasStarted()`：

```cpp
void AModularGameState::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    // 比赛开始事件，自动转发给身上所有 UGameStateComponent
    TArray<UGameStateComponent*> ModularComponents;
    GetComponents(ModularComponents);
    for (UGameStateComponent* Component : ModularComponents)
    {
        Component->HandleMatchHasStarted();
    }
}
```

Lyra 里 `ULyraExperienceManagerComponent`（继承 `UGameStateComponent`）就是这样被自动通知"比赛开始"的——它自己没被 GameState 显式调用。

---

## 五、真实例子 4：那张"卡"长什么样（`ULyraHealthComponent`）

`AModular*` 是"插槽"，那"卡"就是继承 `UGameFrameworkComponent` 的组件。看 `LyraHealthComponent.h`：

```cpp
UCLASS(MinimalAPI, Blueprintable, Meta=(BlueprintSpawnableComponent))
class ULyraHealthComponent : public UGameFrameworkComponent
{
    ...
    void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);
    float GetHealth() const;
    ...
};
```

`ULyraHealthComponent` 就是一个普通的 Actor 组件，但它继承的是 `UGameFrameworkComponent`（而不是引擎原生 `UActorComponent`），所以它能被 `GameFrameworkComponentManager` 识别、能被"动态挂到" `ALyraCharacter` 身上。

Lyra 里这样的"卡"有一大堆：

| 组件类 | 继承的模块化基类 | 挂到谁身上 |
|--------|------------------|-----------|
| `ULyraHealthComponent` | `UGameFrameworkComponent` | `ALyraCharacter` |
| `ULyraWeaponStateComponent` | `UControllerComponent` | `ALyraPlayerController` |
| `ULyraQuickBarComponent` | `UControllerComponent` | `ALyraPlayerController` |
| `ULyraExperienceManagerComponent` | `UGameStateComponent` | `ALyraGameState` |
| `ULyraBotCreationComponent` | `UGameStateComponent` | `ALyraGameState` |
| `ULyraTeamCreationComponent` | `UGameStateComponent` | `ALyraGameState` |

---

## 六、整条链路串起来（一句话版）

```
Game Feature 插件（动态提供能力）
        │  监听 NAME_GameActorReady 事件
        ▼
UGameFrameworkComponentManager（总线）
        │  把组件挂到 Actor 上
        ▼
AModular* 基类（插槽）
        │  在生命周期钩子里 GetComponents() 遍历
        ▼
UGameFrameworkComponent 子类（卡）被逐个通知
```

**所以 ModularGameplayActors 的"具体用法"就是**：

1. 你的 `Character`/`PlayerState`/`GameMode` 继承 `AModular*` 基类（而不是引擎原生类）。
2. 你想加的"能力"写成继承 `UGameFrameworkComponent` 的组件。
3. 这些组件由 Game Feature 插件在运行时**动态挂**到 Actor 上（靠监听 `NAME_GameActorReady`）。
4. `AModular*` 基类会在 `ReceivedPlayer`/`HandleMatchHasStarted` 等钩子里，**自动遍历并通知**这些组件。

**好处**：Actor 本身永远不需要写 `if (有武器系统) 通知武器系统; if (有快捷栏) 通知快捷栏;`——挂多少张卡都行，基类自动搞定。

---

## 七、为什么 Lyra 要绕这么大一圈？

一句话：**为了"玩法可插拔"**。

- 射击玩法：往 `ALyraCharacter` 上挂 `ULyraHealthComponent` + 武器组件。
- 大逃杀玩法：可能挂的是另一套组件（缩圈、毒圈…）。
- 这些组件放在**不同的 Game Feature 插件**里，激活哪个玩法，就加载哪个插件、挂哪套组件。

`AModularCharacter` 这个"空壳基类"就是让这一切成为可能的**万能插槽**。