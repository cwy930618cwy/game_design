# 06 — APawn 底层详解（真实源码实拍）

> **定位**：APawn 是"**能被控制**的 Actor"——玩家控制的角色、AI 控制的敌人都是 Pawn。它在 AActor 基础上，加上了"**被 Possess（附身/控制）**"的能力。
>
> **一句话**：`APawn` = `AActor` + **能被 Controller 控制**。它是"玩家的身体"或"AI 的身体"，由控制器（PlayerController/AIController）操控。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/GameFramework/Pawn.h`（30 KB）

---

## 一、APawn 的真实声明（从源码抄来）

```cpp
// Pawn.h 第 41 行（真实源码）
UCLASS(config=Game, BlueprintType, Blueprintable, ...)
class APawn : public AActor, public INavAgentInterface
{
    GENERATED_BODY()
    ...
};
```

**关键点**：
1. `APawn : public AActor` —— **继承 AActor**（所以有组件/生命周期/网络）
2. `, public INavAgentInterface` —— 还实现了**导航接口**（AI 寻路用）

**官方注释**（Pawn.h 第 36 行）：
> "Pawn is the base class of all actors that can be possessed by players or AI. They are the physical representations of players and creatures in a level."
> **Pawn 是所有能被玩家或 AI 附身控制的 Actor 基类。它们是玩家和生物在关卡中的物理代表。**

---

## 二、APawn 的核心概念：Possess（附身/控制）

这是 Pawn 和普通 Actor 最大的区别。**Possess = 控制器"附身"到一个 Pawn 上控制它。**

```
PlayerController（玩家的控制器）
  └─ Possess（附身）→ APawn（玩家的身体/角色）

AIController（AI 的控制器）
  └─ Possess（附身）→ APawn（AI 控制的敌人）
```

**真实源码（Possess 相关函数）**：

```cpp
// Pawn.h 真实源码
/** 当这个 Pawn 被控制时调用（服务器） */
ENGINE_API virtual void PossessedBy(AController* NewController);

/** 当控制器不再控制我们时调用 */
ENGINE_API virtual void UnPossessed();

/** 返回当前控制这个 Pawn 的 Controller */
UPROPERTY(replicatedUsing=OnRep_Controller)
TObjectPtr<AController> Controller;
```

---

## 三、APawn 的真实成员（从源码看它有什么）

### ① Controller（谁在控制我）—— 最核心

```cpp
// Pawn.h 真实源码
/** Controller currently possessing this Actor */
UPROPERTY(replicatedUsing=OnRep_Controller)
TObjectPtr<AController> Controller;
```

**Pawn 知道"谁在控制我"**。通过 `GetController()` 获取当前控制器。

### ② PlayerState（玩家的状态）

```cpp
// Pawn.h 真实源码
/** If Pawn is possessed by a player, points to its Player State. */
UPROPERTY(replicatedUsing=OnRep_PlayerState)
TObjectPtr<APlayerState> PlayerState;
```

**Pawn 被玩家控制时，关联一个 PlayerState**（记录分数、名字等）。

### ③ 自动被控制设置

```cpp
// Pawn.h 真实源码
/** 关卡开始/生成时，自动让哪个 PlayerController 控制我 */
UPROPERTY(EditAnywhere, Category=Pawn)
TEnumAsByte<EAutoReceiveInput::Type> AutoPossessPlayer;

/** 何时让 AI Controller 控制我 */
UPROPERTY(EditAnywhere, Category=Pawn)
EAutoPossessAI AutoPossessAI;

/** AI 控制器的类 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Pawn)
TSubclassOf<AController> AIControllerClass;
```

### ④ 输入相关

```cpp
// Pawn.h 真实源码
/** 被 PlayerController 控制时，创建输入组件绑定按键 */
ENGINE_API virtual UInputComponent* CreatePlayerInputComponent();

/** 设置玩家的按键绑定（你重写这个绑定 WASD 等） */
virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
```

---

## 四、关键函数（你写游戏会用到）

| 函数 | 作用 |
|------|------|
| `PossessedBy(Controller)` | 被控制时调用（服务器） |
| `UnPossessed()` | 失去控制时调用 |
| `GetController()` | 获取当前控制器 |
| `IsPlayerControlled()` | 是不是玩家控制 |
| `SetupPlayerInputComponent()` | **重写它绑定按键**（WASD 等） |
| `SpawnDefaultController()` | 生成默认控制器并控制 |

---

## 五、具体场景：一个能被玩家控制的 Pawn

```cpp
UCLASS()
class APlayerPawn : public APawn {
    GENERATED_BODY()
public:
    APlayerPawn() {
        // 挂一个碰撞和模型（Pawn 也要组件）
        RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        RootComponent = RootComp;
    }

    // 被玩家控制时，绑定按键（WASD 移动）
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override {
        Super::SetupPlayerInputComponent(Input);
        // 真实场景：绑定 W 键向前移动
        Input->BindAxis("MoveForward", this, &APlayerPawn::MoveForward);
    }

    void MoveForward(float Val) {
        // 前后移动
        AddActorLocalOffset(FVector(Val * 100.f, 0.f, 0.f));
    }
};
```

---

## 六、Pawn 和它的"邻居"们的关系（必背）

```
AActor（能放场景）
  └─ APawn（能被控制）
       └─ ACharacter（人形，内置移动组件）
            └─ 你的玩家角色 / NPC / 敌人

谁控制 Pawn：
  APlayerController（玩家控制器）→ 控制玩家 Pawn
  AIController（AI 控制器）       → 控制 AI Pawn
```

**三者分工**：
| 类 | 作用 |
|------|------|
| `APawn` | **身体**（能被控制的角色） |
| `AController` | **大脑**（决定做什么：玩家输入 / AI） |
| `ACharacter` | 升级版 Pawn（内置人形移动） |

---

## 七、APawn 底层总结

```
AActor（能放场景 + 组件 + 生命周期）
  └─ APawn（能被 Controller 控制）
       ├─ 知道"谁在控制我"（Controller 成员）
       ├─ 玩家控制时关联 PlayerState
       ├─ 支持自动被玩家/AI 控制
       └─ 能绑定输入（SetupPlayerInputComponent）

核心概念：Possess（附身）
  PlayerController / AIController ──Possess──→ APawn

你重写：SetupPlayerInputComponent（绑定按键）
```

**一句话**：`APawn` 在 `AActor` 基础上，加了"**能被控制**"的能力。它是玩家的身体或 AI 的身体，由 `Controller`（PlayerController/AIController）附身操控。核心概念是 **Possess（附身）**，你通过重写 `SetupPlayerInputComponent` 绑定玩家输入。

---

## 八、下一步

理解了 Pawn，下一步自然是 **ACharacter**（人形角色，内置移动）和 **Controller/GameMode**（游戏框架）。这是你实际写角色时天天用的。
