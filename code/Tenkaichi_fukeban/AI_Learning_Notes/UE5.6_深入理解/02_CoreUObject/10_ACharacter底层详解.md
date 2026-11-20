# 10 — ACharacter 底层详解（真实源码实拍）

> **定位**：ACharacter 是"**人形可移动角色**"——你写玩家、NPC、敌人天天用的类。它内置了移动、碰撞、模型三件套。
>
> **一句话**：`ACharacter` = `APawn`（能被控制）+ **内置移动组件 + 胶囊碰撞 + 骨骼模型**。开箱即用的"能走能跳的人形角色"。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/GameFramework/Character.h`（59 KB）

---

## 一、ACharacter 的真实声明（从源码抄来）

```cpp
// Character.h 第 240 行（真实源码）
UCLASS(config=Game, BlueprintType, meta=(ShortTooltip="A character is a type of Pawn that includes the ability to walk around."), MinimalAPI)
class ACharacter : public APawn
{
    GENERATED_BODY()
public:
    ...
};
```

**官方注释**（Character.h 第 240 行）：
> "A character is a type of Pawn that includes the ability to walk around."
> **ACharacter 是一种 Pawn，包含"到处走动"的能力。**

**继承链**：`ACharacter` → `APawn` → `AActor` → `UObject`

---

## 二、ACharacter 内置的"三件套"（核心！真实源码）

ACharacter 的**最大价值**：它自动创建了三个关键组件。看真实源码：

```cpp
// Character.h 第 253-263 行（真实源码）

/** ① 主骨骼网格（显示角色模型 + 动画） */
UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<USkeletalMeshComponent> Mesh;

/** ② 移动组件（走路/跑步/跳跃/下落） */
UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UCharacterMovementComponent> CharacterMovement;

/** ③ 胶囊碰撞（碰撞检测 + 移动物理） */
UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UCapsuleComponent> CapsuleComponent;
```

**这就是为什么 ACharacter "开箱即用"**——它已经给你准备好了：

```
ACharacter 自动创建：
├── USkeletalMeshComponent（Mesh）       → 显示角色模型 + 动画
├── UCharacterMovementComponent（CharacterMovement）→ 移动
└── UCapsuleComponent（CapsuleComponent）→ 碰撞
```

**用 APawn 你得自己挂这些，用 ACharacter 它全给你配好了。**

---

## 三、三个组件的用途（配具体场景）

### ① Mesh（USkeletalMeshComponent）—— 模型 + 动画

```cpp
// 设置角色的骨骼模型
GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Hero.Hero")));

// 播放动画
GetMesh()->PlayAnimation(IdleAnim, true);   // 播放待机动画
```

### ② CharacterMovement（UCharacterMovementComponent）—— 移动

```cpp
// 控制移动（蓝图/Axis 绑定）
AddMovementInput(FVector::ForwardVector, 1.f);   // 向前走

// 跳跃
Jump();

// 移动设置
GetCharacterMovement()->MaxWalkSpeed = 600.f;    // 走路速度
GetCharacterMovement()->JumpZVelocity = 420.f;   // 跳跃高度
```

### ③ CapsuleComponent（UCapsuleComponent）—— 碰撞

```cpp
// 胶囊碰撞（决定角色的物理体积）
GetCapsuleComponent()->SetCapsuleRadius(34.f);   // 半径
GetCapsuleComponent()->SetCapsuleHalfHeight(88.f); // 半高
```

---

## 四、完整：一个能走能跳的角色

```cpp
UCLASS()
class AMyCharacter : public ACharacter {
    GENERATED_BODY()
public:
    AMyCharacter() {
        // ACharacter 已自动配好：Mesh + CharacterMovement + Capsule

        // 你只需要设置属性和挂额外组件
        // 例：设走路速度
        GetCharacterMovement()->MaxWalkSpeed = 500.f;
    }

    // 绑定按键（SetupPlayerInputComponent）
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override {
        Super::SetupPlayerInputComponent(Input);
        Input->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
        Input->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    }

    void MoveForward(float Val) {
        AddMovementInput(FVector::ForwardVector, Val);   // 前后移动
    }
};
```

---

## 五、ACharacter vs APawn（区别）

| | APawn | ACharacter |
|---|---|---|
| 能被控制 | ✅ | ✅ |
| 内置移动 | ❌（要自己加） | ✅（CharacterMovement） |
| 内置碰撞 | ❌ | ✅（Capsule） |
| 内置模型 | ❌ | ✅（Mesh） |
| 适用 | 飞机/车辆等自定义 | **人形角色**（玩家/敌人/NPC） |

**一句话**：ACharacter = APawn + 人形三件套（移动+碰撞+模型）。**写人形角色用 ACharacter，写非人形（载具）用 APawn。**

---

## 六、常见陷阱

**① 用 ACharacter 但不设骨骼模型 → 角色看不见**
```cpp
// ❌ 忘了设 Mesh，角色是个隐形胶囊
// ✅ 设骨骼模型 + 动画蓝图
GetMesh()->SetSkeletalMesh(...);
GetMesh()->SetAnimInstanceClass(...);
```

**② 改移动属性忘用 GetCharacterMovement()**
```cpp
// ❌ 直接访问不存在的成员
// ✅ 用 GetCharacterMovement()
GetCharacterMovement()->MaxWalkSpeed = 600.f;
```

**③ 重写 Jump 却忘了 Super**
```cpp
// ❌ 覆盖了 ACharacter 的跳跃逻辑
virtual void Jump() override { /* 空 */ }
// ✅ 要调 Super
virtual void Jump() override { Super::Jump(); /* 额外逻辑 */ }
```

---

## 七、总结速查

```
ACharacter = APawn + 人形三件套
├── Mesh（USkeletalMeshComponent）→ 模型 + 动画
├── CharacterMovement（UCharacterMovementComponent）→ 移动/跳跃
└── CapsuleComponent（UCapsuleComponent）→ 碰撞

常用：
  GetMesh() → 设模型/动画
  GetCharacterMovement() → 移动设置（速度/跳跃）
  GetCapsuleComponent() → 碰撞设置
  AddMovementInput() → 移动
  Jump() → 跳跃
  SetupPlayerInputComponent() → 绑定按键

适用：玩家角色、NPC、敌人（人形）
```

**一句话**：ACharacter 是"**开箱即用的人形角色**"，自动配好 `Mesh`（模型）、`CharacterMovement`（移动）、`CapsuleComponent`（碰撞）。写玩家/敌人/NPC 都用它，用 `GetCharacterMovement()` 控移动、`GetMesh()` 设模型、`SetupPlayerInputComponent()` 绑按键。

---

## 八、下一步

理解了 ACharacter，下一个自然的是 **AController 家族**（PlayerController 处理输入、AIController 控制 AI）——它们和 ACharacter 配合，构成完整的"大脑控制身体"。要继续吗？
