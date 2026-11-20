# 06 — Enhanced Input 详解（增强输入系统）

> **定位**：UE5 的**现代输入系统**，替代旧的 Input Axis/Action。Lyra 用的就是它。
>
> **一句话**：Enhanced Input 用**数据驱动**的方式，把"**按键**"（W键/手柄摇杆）映射到"**游戏动作**"（向前走/转身）。核心是 **Input Action（动作）+ Input Mapping Context（映射上下文）**。
>
> **文件**：`Engine/Plugins/EnhancedInput/`（插件）

---

## 一、为什么要换 Enhanced Input？（旧 vs 新）

### 旧输入（Input Axis/Action）
```cpp
// 旧方式：直接绑按键到函数，写死在代码里
InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
InputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
```
**问题**：按键映射写死在代码里，改键位麻烦，手柄/键鼠切换麻烦。

### 新输入（Enhanced Input）
```
用数据资产定义"动作"和"映射"，代码只管"动作"不管"按键"
  InputAction（动作）= 抽象的动作（如"向前走"）
  InputMappingContext（映射）= 哪个键触发这个动作（W 键/摇杆）
```

**核心优势**：**代码和按键解耦**。改键位、加手柄支持，都改数据资产，不用改代码。

---

## 二、核心概念（必须先懂）

### ① Input Action（输入动作）—— 抽象的动作

**InputAction = 一个"动作"**，比如"移动"、"跳跃"、"射击"。它不关心具体按键。

```
InputAction：IA_Move（移动）
InputAction：IA_Jump（跳跃）
InputAction：IA_Fire（射击）
```

### ② Input Mapping Context（输入映射上下文）—— 按键 → 动作

**IMC = 一套"哪个键触发哪个动作"的映射**。

```
InputMappingContext：IMC_Default（默认映射）
  ├─ W/A/S/D 键 → IA_Move（移动）
  ├─ 空格键    → IA_Jump（跳跃）
  ├─ 鼠标左键  → IA_Fire（射击）
  └─ 手柄摇杆  → IA_Move（移动）
```

**关键**：**同一个动作（IA_Move）可以映射多个按键**（W 键、摇杆、方向键都能触发移动）。

---

## 三、完整流程：怎么用（4 步）

### 第 1 步：创建 Input Action（资产）

```
Content Browser → 右键 → Input → Input Action
→ 命名 IA_Move，类型选 Axis2D（二维：前后+左右）
```

### 第 2 步：创建 Input Mapping Context（资产）

```
Content Browser → 右键 → Input → Input Mapping Context
→ 命名 IMC_Default
→ 在里面加映射：W/S/A/D → IA_Move
```

### 第 3 步：代码里绑定（EnhancedInputComponent）

```cpp
UCLASS()
class AMyCharacter : public ACharacter {
    GENERATED_BODY()
public:
    // 引用输入资产（在蓝图/编辑器里指定）
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UInputAction> MoveAction;

protected:
    // 绑定输入（EnhancedInputComponent）
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
};
```

```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    // 用 EnhancedInputComponent 绑定（不是旧的 InputComponent）
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
        // 绑定"移动"动作到处理函数
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
    }
}

void AMyCharacter::Move(const FInputActionValue& Value) {
    // 从动作值里取出输入向量（W=前进，S=后退，A=左，D=右）
    FVector2D Input = Value.Get<FVector2D>();
    AddMovementInput(FVector::ForwardVector, Input.Y);   // 前后
    AddMovementInput(FVector::RightVector, Input.X);     // 左右
}
```

### 第 4 步：把映射上下文加到 PlayerController

```cpp
// 在 PlayerController 里添加映射上下文（让输入生效）
UCLASS()
class AMyPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    virtual void BeginPlay() override {
        Super::BeginPlay();
        // 添加默认映射上下文（0 优先级）
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
};
```

---

## 四、核心代码对照（记这几个）

| 东西 | 作用 |
|------|------|
| `UInputAction` | 动作资产（移动/跳跃/射击） |
| `UInputMappingContext` | 映射资产（哪个键 → 哪个动作） |
| `UEnhancedInputComponent` | 绑定输入的组件 |
| `UEnhancedInputLocalPlayerSubsystem` | 管理映射上下文（添加/移除） |
| `FInputActionValue` | 动作的值（Get<FVector2D>() 等） |
| `ETriggerEvent` | 触发时机（Triggered/Pressed/Started） |

---

## 五、具体场景：跳跃 + 移动 + 射击

```cpp
// 三个动作资产：IA_Jump / IA_Move / IA_Fire
// 映射：空格→IA_Jump，WASD→IA_Move，鼠标左键→IA_Fire

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
        EIC->BindAction(MoveAction,  ETriggerEvent::Triggered, this, &AMyCharacter::Move);
        EIC->BindAction(JumpAction,  ETriggerEvent::Started,   this, &ACharacter::Jump);
        EIC->BindAction(FireAction,  ETriggerEvent::Started,   this, &AMyCharacter::Fire);
    }
}
```

**触发时机（ETriggerEvent）**：
- `Triggered`：持续触发（移动，按住一直动）
- `Started`：按下瞬间（跳跃/射击，只触发一次）
- `Completed`：松开瞬间
- `Canceled`：取消

---

## 六、Enhanced Input 的优势（为什么 Lyra 用它）

| 优势 | 说明 |
|------|------|
| **代码解耦** | 代码只管动作，不管按键 |
| **改键位** | 改数据资产，不改代码 |
| **多设备** | 键鼠/手柄/触屏共用一套动作 |
| **组合键** | 支持组合（如"同时按 左+攻击"） |
| **Lyra 用** | Lyra 用 Tag 进一步解耦输入 |

**Lyra 的扩展**：Lyra 在 Enhanced Input 基础上，用 **GameplayTag** 映射输入——输入动作打 Tag，技能用 Tag 匹配，输入和技能完全解耦。

---

## 七、常见陷阱

**① 忘了加映射上下文（输入不生效）**
```cpp
// ❌ 绑定了动作但没 AddMappingContext，按键没反应
// ✅ 要在 PlayerController 里 AddMappingContext
```

**② 用旧的 InputComponent 绑定新动作**
```cpp
// ❌ InputComponent->BindAction(...) 是旧的
// ✅ 要用 Cast<UEnhancedInputComponent>
```

**③ 忘了处理 Triggered 和 Started 的区别**
```cpp
// 移动要 Triggered（持续），跳跃要 Started（一次）
// 用错时机，行为不对
```

---

## 八、总结速查

```
Enhanced Input 核心：
  InputAction（动作）：移动/跳跃/射击（不关心按键）
  InputMappingContext（映射）：W键→移动，空格→跳跃
  代码只处理"动作"，不处理"按键"

使用流程：
  1. 创建 InputAction 资产
  2. 创建 InputMappingContext 资产（按键→动作）
  3. 代码用 UEnhancedInputComponent 绑定动作
  4. PlayerController 里 AddMappingContext

触发时机：
  Triggered（持续）/ Started（按下）/ Completed（松开）

优势：代码解耦，改键位改数据，多设备共用
```

**一句话**：Enhanced Input 用**数据驱动**把"**按键**"映射到"**动作**"。核心是 `InputAction`（动作）+ `InputMappingContext`（映射）。**代码只管动作不管按键，改键位改数据资产**。Lyra 用它，并进一步用 Tag 解耦输入和技能。

---

## 九、下一步

理解了 Enhanced Input，下一步可以看 **Lyra 怎么用它**（LyraInputConfig 用 Tag 映射输入到技能），或深入输入和 GAS 技能的联动。
