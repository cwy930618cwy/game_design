# 11 — AController 家族详解（大脑体系）

> **定位**：理解 UE 的"**大脑**"——Controller 不自己存在，它**控制一个 Pawn**。玩家的大脑是 PlayerController，AI 的大脑是 AIController。
>
> **一句话**：`AController` = "**大脑**"，它 `Possess`（附身）到一个 `APawn`（身体）上控制它。玩家控制器（PlayerController）处理输入，AI 控制器（AIController）跑 AI 逻辑。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/GameFramework/Controller.h`

---

## 一、AController 的真实声明（从源码抄来）

```cpp
// Controller.h 第 39 行（真实源码）
UCLASS(abstract, notplaceable, NotBlueprintable, ...)
class AController : public AActor, public INavAgentInterface
{
    GENERATED_BODY()
private:
    /** Pawn currently being controlled by this controller */
    UPROPERTY(replicatedUsing=OnRep_Pawn)
    TObjectPtr<APawn> Pawn;   // ← 核心：它控制哪个 Pawn
public:
    ...
};
```

**官方注释**（Controller.h 第 24-27 行）：
> "PlayerControllers are used by human players to control pawns, while AIControllers implement the artificial intelligence for the pawns they control. Controllers take control of a pawn using their Possess() method..."
> **PlayerController 让人控制 Pawn，AIController 实现 AI。Controller 用 Possess() 控制 Pawn。**

**关键**：`AController` 继承 `AActor`（所以它也是个 Actor，能放场景，但不渲染），核心成员是 `Pawn`（它控制谁）。

---

## 二、核心概念：Possess（附身）—— 大脑接管身体

这是 Controller 的**灵魂**。看真实源码：

```cpp
// Controller.h 第 281 行（真实源码）
/** 让这个控制器控制一个 Pawn */
UFUNCTION(...)
virtual void Possess(APawn* InPawn);

/** 释放控制的 Pawn */
virtual void UnPossess();

/** 返回当前控制的 Pawn */
FORCEINLINE TObjectPtr<APawn> GetPawn() const { return Pawn; }
```

**流程**：
```
Controller.Possess(Pawn)   → Controller 开始控制这个 Pawn
Controller.GetPawn()       → 拿到它控制的 Pawn
Controller.UnPossess()     → 释放控制
```

**具体场景：游戏开始时，玩家控制器附身到玩家角色**

```cpp
// GameMode 里配置（通常自动发生）
void AMyGameMode::BeginPlay() {
    Super::BeginPlay();
    // 找到玩家控制器和默认角色
    APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (PC && PC->GetPawn()) {
        // PC 已经自动 Possess 了玩家 Pawn
    }
}
```

---

## 三、家族成员（Controller 的子类）

```
AController（大脑基类）
├── APlayerController（玩家大脑）
└── AAIController（AI 大脑）
```

| | APlayerController | AAIController |
|---|---|---|
| 控制谁 | 玩家 Pawn | AI Pawn |
| 干什么 | 处理玩家输入、视角 | 跑 AI 逻辑（行为树） |
| 谁来 | 真人玩家 | AI 程序 |

---

## 四、APlayerController —— 玩家大脑（处理输入）

玩家控制器**接收玩家的键盘/鼠标输入**，指挥 Pawn 行动。

```cpp
UCLASS()
class AMyPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    virtual void SetupInputComponent() override {
        Super::SetupInputComponent();
        // 绑定按键（玩家输入）
        InputComponent->BindAxis("MoveForward", this, &AMyPlayerController::MoveForward);
    }

    void MoveForward(float Val) {
        // 指挥控制的 Pawn 移动
        if (APawn* P = GetPawn()) {
            P->AddMovementInput(FVector::ForwardVector, Val);
        }
    }
};
```

**注意区别**：
- **APlayerController** 用 `SetupInputComponent()` 绑输入
- **APawn/ACharacter** 用 `SetupPlayerInputComponent()` 绑输入（如果你在 Pawn 里绑，也能行）

---

## 五、AAIController —— AI 大脑（控制 AI）

AI 控制器实现 AI 逻辑，控制 AI Pawn。

```cpp
UCLASS()
class AMonsterAIController : public AAIController {
    GENERATED_BODY()
public:
    virtual void OnPossess(APawn* InPawn) override {
        Super::OnPossess(InPawn);
        // 被控制时，启动 AI 逻辑（比如跑行为树）
        RunBehaviorTree(BTAsset);
    }
};
```

**具体场景：让 AI 敌人巡逻**

```cpp
// 在 GameMode 里生成敌人并让它被 AI 控制
AAIController* AIC = GetWorld()->SpawnActor<AMonsterAIController>(...);
AIC->Possess(EnemyPawn);   // AI 大脑附身到敌人身上
```

---

## 六、完整协作：GameMode + Controller + Pawn（一局游戏）

```
AGameMode（规则）
  ├─ 生成 APlayerController（玩家大脑）
  │    └─ Possess → ACharacter（玩家身体）
  │         └─ 玩家按 WASD，PlayerController 指挥移动
  ├─ 生成 AAIController（AI 大脑）
  │    └─ Possess → AEnemy（敌人身体）
  │         └─ AI 跑行为树，控制敌人
  └─ 判定胜负
```

**关键流程**：
```
GameMode 启动 → 生成 Controller → Controller.Possess(Pawn) → 控制
```

---

## 七、常用函数

| 函数 | 作用 |
|------|------|
| `Possess(Pawn)` | 控制一个 Pawn |
| `UnPossess()` | 释放 Pawn |
| `GetPawn()` | 拿当前控制的 Pawn |
| `OnPossess()` | 被控制时重写（初始化） |
| `GetPlayerController()` | 拿玩家控制器 |

```cpp
// 获取玩家控制器
APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
// 拿它控制的 Pawn
APawn* MyPawn = PC->GetPawn();
```

---

## 八、常见陷阱

**① 在 Controller 里访问 Pawn 忘了 GetPawn()**
```cpp
// ❌ 直接访问不存在的成员
// ✅ 用 GetPawn()
if (APawn* P = GetPawn()) { P->AddMovementInput(...); }
```

**② 混淆 SetupInputComponent（Controller）和 SetupPlayerInputComponent（Pawn）**
```cpp
// Controller 里用 SetupInputComponent
// Pawn 里用 SetupPlayerInputComponent
// 别搞混
```

**③ 忘了 Possess 就控制不了**
```cpp
// ❌ 生成 Controller 没 Possess，控制不了 Pawn
SpawnActor<AMonsterAIController>(...);   // 没 Possess
// ✅
AIC->Possess(EnemyPawn);
```

---

## 九、总结速查

```
AController = 大脑（控制 Pawn）
├── APlayerController（玩家大脑：处理输入）
└── AAIController（AI 大脑：行为树）

核心：Possess(Pawn) 附身控制，GetPawn() 拿当前控制
流程：GameMode → 生成 Controller → Possess(Pawn) → 控制

常用：
  Possess() / UnPossess() / GetPawn() / OnPossess()
```

**一句话**：Controller 是"**大脑**"，用 `Possess()` 附身到 `APawn`（身体）上控制它。玩家用 `APlayerController`（处理输入），AI 用 `AAIController`（跑行为树）。**大脑（Controller）控制身体（Pawn），GameMode 负责连接它们。**

---

## 十、下一步

理解了 Controller 家族，"大脑控制身体"的游戏框架就完整了。接下来可以进入其他模块（输入系统、动画、AI 行为树、UI），或先沉淀已学内容。
