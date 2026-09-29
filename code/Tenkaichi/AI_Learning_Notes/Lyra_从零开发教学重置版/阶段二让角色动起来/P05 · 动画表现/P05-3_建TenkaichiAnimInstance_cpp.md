# P05 步 2 — 建 `TenkaichiAnimInstance`（.cpp 实现）

> **本 md**：只教 `.cpp`（实现）。`.h` 已在 `P05-2_建TenkaichiAnimInstance_h.md` 教过。
>
> **本步目标**：实现两个函数——① 构造函数（转交父类）；② `NativeUpdateAnimation`（每帧算出水平速度，赋给 `Speed`）。

---

## 一、先讲为什么（本文件要干两件事）

`.h` 里声明了两个东西，`.cpp` 要把它们实现出来：

| `.h` 声明 | `.cpp` 实现 | 干什么 |
|-----------|-------------|--------|
| 构造函数 | 转交父类 `Super(ObjectInitializer)` | 本身什么都不做 |
| `NativeUpdateAnimation` | 算速度赋给 `Speed` | 每帧给蓝图送数据 |

---

## 二、减法说明（对比 Lyra 的 `.cpp`）

Lyra 的 `NativeUpdateAnimation`（`LyraAnimInstance.cpp` 第 51-64 行）长这样：

```cpp
const ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwningActor());   // 依赖 Lyra 角色类
...
ULyraCharacterMovementComponent* CharMoveComp = CastChecked<...>(...);     // 依赖 Lyra 移动组件
const FLyraCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
GroundDistance = GroundInfo.GroundDistance;                                 // 离地距离
```

它依赖两个**我们没建**的 Lyra 自定义类：`ALyraCharacter` 和 `ULyraCharacterMovementComponent`。

**我们的减法版本**：不 Cast 到任何具体角色类，直接用引擎 `AActor` 基类自带的 `GetVelocity()`：

```cpp
const AActor* OwningActor = GetOwningActor();
Speed = OwningActor ? OwningActor->GetVelocity().Size2D() : 0.0f;
```

> **为什么能这么省**：`GetVelocity()` 是 `AActor` 基类的方法（任何角色都有），`Size2D()` 是 `FVector` 的方法（算水平长度）。两者都是引擎原生 API，不需要 Lyra 的自定义组件。达到的目的和 Lyra 一样——"每帧送一个数字给蓝图"，只是数字从"离地距离"换成"速度"。

---

## 三、代码（`.cpp` 完整内容）

文件路径：`Source/Tenkaichi/Animation/TenkaichiAnimInstance.cpp`

```cpp
#include "TenkaichiAnimInstance.h"

#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAnimInstance)

// 构造函数：把构造转交给父类 UAnimInstance，本身什么都不做（对应 LyraAnimInstance.cpp 第 15-18 行）
UTenkaichiAnimInstance::UTenkaichiAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 引擎每帧调用一次：算出角色的水平移动速度，赋给 Speed（对应 LyraAnimInstance.cpp 第 51-64 行的思路）
void UTenkaichiAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 拿到这个动画实例所属的 Actor（也就是我们的角色）
	const AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		// 还没挂到任何角色上，速度算 0（静止）
		Speed = 0.0f;
		return;
	}

	// 核心：GetVelocity() 返回角色的速度向量（3D），Size2D() 只取水平（X/Y）长度，
	// 忽略 Z 轴的跳跃/下落，这样"走路/跑步"的判定不受跳跃干扰。
	Speed = OwningActor->GetVelocity().Size2D();
}
```

---

## 四、逐行讲（回扣"为什么"）

### 1. include 与生成头

```cpp
#include "TenkaichiAnimInstance.h"          // 自己的 .h（类声明）

#include "GameFramework/Actor.h"             // 为了 AActor 的 GetVelocity()

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAnimInstance)
```

- `UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAnimInstance)`：UE5 的现代写法，**替代旧的 `#include "TenkaichiAnimInstance.generated.h"`**（那个放 `.h` 里）。它告诉编译器"这是本文件对应的 UHT 生成代码入口"。Lyra 的 `.cpp` 第 12 行也是这么写的。
- `GameFramework/Actor.h`：`GetVelocity()` 声明在 `AActor` 里，需要这个头。Lyra 那边因为 `Cast<ALyraCharacter>` 而间接 include 了，我们直接 include 更干净。

### 2. 构造函数

```cpp
UTenkaichiAnimInstance::UTenkaichiAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
```

- `: Super(ObjectInitializer)`：把 `ObjectInitializer` 传给父类 `UAnimInstance` 的构造函数。这是 UObject 派生的标准姿势。
- 函数体空：我们不需要在构造时初始化任何东西（`Speed` 已在 `.h` 里给默认值 `0.0f`）。

### 3. `NativeUpdateAnimation` 开头

```cpp
Super::NativeUpdateAnimation(DeltaSeconds);
```

- **先调父类**，让引擎把基础的动画更新跑完（比如算骨骼、混合等），再补我们自己的逻辑。这是 UE 虚函数重写的惯例——`Super::` 优先。

### 4. 拿 Actor + 判空

```cpp
const AActor* OwningActor = GetOwningActor();
if (!OwningActor)
{
    Speed = 0.0f;
    return;
}
```

- `GetOwningActor()`：`UAnimInstance` 自带的方法，返回这个动画实例挂在哪个 Actor 身上（动画实例通常挂在角色的骨骼网格组件上）。
- **判空很重要**：动画实例在初始化早期可能还没挂到角色上，此时 `GetOwningActor()` 返回空指针，直接访问会崩溃。所以先判空，空就 `Speed = 0` 并提前返回。

### 5. 核心：算水平速度

```cpp
Speed = OwningActor->GetVelocity().Size2D();
```

拆开看：
- `OwningActor->GetVelocity()`：返回 `FVector`（三维速度向量，单位 cm/s）。角色往前走时，这个向量的 X/Y 有值，Z 是 0；跳起来时 Z 会突然变大。
- `.Size2D()`：`FVector` 的方法，**只算 X/Y 两个轴的向量长度**，忽略 Z。也就是说"跳起来"不会让这个数字暴涨，走路/跑步的判定更稳。
- 结果赋给 `Speed`（`.h` 里那个 `BlueprintReadOnly` 变量），动画蓝图就能读到它。

---

## 五、为什么用 `Size2D()` 而不是 `Size()`（一个常见坑）

| 方法 | 算的是什么 | 问题 |
|------|-----------|------|
| `Size()` | X/Y/Z 三轴总长度 | 角色跳起来时 Z 变大，速度数字会突然暴涨，动画蓝图会误判成"跑得飞快" |
| `Size2D()` | 只 X/Y 水平长度 | 跳跃不影响，只反映"前后左右"的真实移动速度 |

> 走路/跑步只看水平移动，所以用 `Size2D()`。这是从"能跑"到"跑得对"的关键细节。

---

## 六、核对清单（铁律 11：`.h` ↔ `.cpp` 成对）

| `.h` 声明 | `.cpp` 实现 | 状态 |
|-----------|-------------|------|
| `UTenkaichiAnimInstance(const FObjectInitializer&)` | ✅ 已写（转交 Super） | 成对 |
| `virtual void NativeUpdateAnimation(float)` | ✅ 已写（算 Speed） | 成对 |
| `float Speed`（UPROPERTY） | ✅ 在 `NativeUpdateAnimation` 里赋值 | 成对 |

---

## 七、下一步

`.cpp` 写完，编译应该能过（这是纯 C++ 类，不涉及蓝图资产）。

但**光有 C++ 类，角色还不会播动画**——因为还差两样：
1. 角色的骨骼网格组件要**指定用这个 AnimInstance 类**（这一步在步 3 的编辑器里做）；
2. 要有一个**动画蓝图**（步 3），它继承 `UTenkaichiAnimInstance`，里面搭状态机 + Blend Space 读 `Speed` 来切动画。

所以下一步 = **步 3：动画蓝图搭状态机 + Blend Space**（主要是动编辑器/蓝图，不是写 C++）。

确认 `.cpp` 没问题后，回「**懂了/下一步**」，我教步 3。