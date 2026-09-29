# P05 步 2 — 建 `TenkaichiAnimInstance`（.h 骨架）

> **本 md**：只教 `.h`（类声明）。`.cpp`（实现）在下一个 md 单独教。
>
> **本步目标**：建一个 C++ 动画实例类 `UTenkaichiAnimInstance`，继承引擎的 `UAnimInstance`，它的职责只有一个——**每帧把角色的"移动速度"算好，暴露成蓝图能读的变量**。

---

## 一、先讲为什么（设计决策）

回顾总概念里的分工：C++ 的 `AnimInstance` = **报数员**，只送数据，不切动画。

所以这个 `.h` 要回答三个问题：

| 问题 | 答案 | 对应 Lyra |
|------|------|-----------|
| 继承谁？ | 引擎自带的 `UAnimInstance` | `ULyraAnimInstance : public UAnimInstance` |
| 每帧干什么？ | 重写 `NativeUpdateAnimation`，算速度 | Lyra 里算 `GroundDistance` |
| 数据怎么送出去？ | 一个 `BlueprintReadOnly` 变量，蓝图读它 | Lyra 的 `GroundDistance` 变量 |

---

## 二、减法说明（铁律 16：只做减法，说清删了什么）

对照 Lyra 的 `ULyraAnimInstance`（46 行），我们**删掉了 3 样东西**：

| Lyra 有的 | 我们删了吗 | 为什么删 |
|-----------|-----------|----------|
| `FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap` | ✅ 删 | 它把 GAS 的 GameplayTag 自动映射成蓝图变量，P05 用不到，依赖 GAS 太重 |
| `InitializeWithAbilitySystem(ASC)` | ✅ 删 | 是上面那个 Tag 映射的初始化，跟着一起删 |
| `IsDataValid(...)`（编辑器校验） | ✅ 删 | 纯编辑器辅助，教学阶段用不到 |
| `GroundDistance`（离地距离） | ✅ 换成 `Speed` | `GroundDistance` 依赖 Lyra 自定义的 `LyraCharacterMovementComponent`（我们没建），改用引擎自带 `GetVelocity()` 就能算的**速度** |

> **一句话**：删掉一切依赖"Lyra 自定义组件 / GAS"的东西，只保留"每帧送一个数字给蓝图"这个核心范式。数字从"离地距离"换成"速度"，因为待机/走/跑只需要速度。

---

## 三、代码（`.h` 完整内容）

文件路径：`Source/Tenkaichi/Animation/TenkaichiAnimInstance.h`

```cpp
#pragma once

#include "Animation/AnimInstance.h"
#include "TenkaichiAnimInstance.generated.h"

/**
 * UTenkaichiAnimInstance
 *
 *	本工程的动画实例基类。
 *	职责：每帧把角色的移动速度算好，暴露给动画蓝图（作为"送数据"的报数员）。
 *	动画的切换（待机/走/跑）不在这个类里做，而在动画蓝图的状态机里做。
 */
UCLASS(Config = Game)
class UTenkaichiAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UTenkaichiAnimInstance(const FObjectInitializer& ObjectInitializer);

protected:

	// 引擎每帧调用一次，在这里算"速度"送进蓝图（对应 LyraAnimInstance.cpp 的 NativeUpdateAnimation）
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	// 这块"黑板"：动画蓝图读这个变量来决定走/跑/待机。
	// BlueprintReadOnly = 蓝图只能读、不能写（数据由 C++ 每帧更新）。
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float Speed = 0.0f;
};
```

---

## 四、逐行讲（回扣"为什么"）

### 1. 头文件与生成头

```cpp
#include "Animation/AnimInstance.h"          // 引擎的动画实例基类
#include "TenkaichiAnimInstance.generated.h" // UHT 生成的头，必须放最后
```

- `AnimInstance.h` 是引擎自带的，路径 `Animation/AnimInstance.h`，和 Lyra 一模一样（LyraAnimInstance.h 第 5 行也是这个 include）。
- `.generated.h` 是 UHT（Unreal Header Tool）根据 `UCLASS`/`UPROPERTY` 自动生成的，**必须最后 include**，且文件名要和本文件同名。

### 2. UCLASS 宏

```cpp
UCLASS(Config = Game)
class UTenkaichiAnimInstance : public UAnimInstance
```

- `Config = Game`：对应 Lyra 的写法（LyraAnimInstance.h 第 17 行 `UCLASS(Config = Game)`）。表示这个类可以在 `DefaultGame.ini` 里配置默认值。我们一比一保留。
- 类名 `UTenkaichiAnimInstance`：前缀 `U`（UObject 派生类规范），换 Lyra 的 `Lyra` 前缀为 `Tenkaichi`。

### 3. 构造函数

```cpp
UTenkaichiAnimInstance(const FObjectInitializer& ObjectInitializer);
```

- 带 `FObjectInitializer` 的构造函数，是 `UAnimInstance` 派生的标准写法（Lyra 第 24 行也是这个签名）。实现放 `.cpp`，下一步教。

### 4. 核心：重写 `NativeUpdateAnimation`

```cpp
virtual void NativeUpdateAnimation(float DeltaSeconds) override;
```

- 这是 `UAnimInstance` 提供的虚函数，**引擎每帧自动调用一次**。
- `DeltaSeconds` = 上一帧到这一帧的时间差（秒），单位秒。
- 我们在这里写"算速度"的逻辑。对应 Lyra 的第 35 行（Lyra 重写它来算 `GroundDistance`）。
- `override` 关键字 = 明确告诉编译器"我是在重写父类的虚函数"，写错签名会直接报错，防止手滑。

### 5. 数据出口：`Speed` 变量

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
float Speed = 0.0f;
```

- `BlueprintReadOnly`：**蓝图只能读、不能写**。这是关键——速度由 C++ 每帧算好，蓝图只负责"读"它来决定放哪个动画，不能反过来改它。
- `Category = "Character State Data"`：在编辑器详情面板里归类显示，对应 Lyra 第 45 行的分类名。
- `float Speed = 0.0f`：默认 0（静止），类型 float（速度是小数，比如 350.5 cm/s）。

---

## 五、核对清单（铁律 11：`.h` 声明 ↔ `.cpp` 实现成对）

| `.h` 里声明了什么 | `.cpp` 里要做什么（下一步） |
|-------------------|------------------------------|
| 构造函数 `UTenkaichiAnimInstance(const FObjectInitializer&)` | 写 `: Super(ObjectInitializer) {}` |
| `NativeUpdateAnimation(float)` | 拿角色 `GetVelocity().Size2D()` 算出速度，赋给 `Speed` |

---

## 六、下一步

这份 `.h` 你确认后，回我「**懂了/下一步**」，我教 `.cpp`（`TenkaichiAnimInstance.cpp`），讲清楚：
- 构造函数怎么初始化父类；
- `NativeUpdateAnimation` 里怎么用引擎自带的 `GetVelocity()` 算出水平速度；
- 为什么用 `Size2D()`（忽略 Z 轴的跳跃）而不是 `Size()`。