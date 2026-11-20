# P04-3c — 角色类写输入实现（.cpp）

> **本步定位**：P04 第 3 步的 `.cpp` 部分。上一步（P04-3b）在 `.h` 里加了声明，这一步在 `.cpp` 里写实现——重写 `SetupPlayerInputComponent` + 写两个回调的移动逻辑。
>
> **⚠️ 写代码边界**：你工程 `Source\` 里的代码**由你自己写**，我只讲解、给对照。下面的代码是「教你怎么写」，请你照着一比一写进 `.cpp`。
>
> **⚠️ 本步是 P04 的「兑现时刻」**：写完角色就能动（移动 + 转视角）。

---

## 一、这一步要解决什么问题（先讲为什么）

`.h` 里声明了三个函数，现在要补它们的「身体」：

1. `SetupPlayerInputComponent` —— 怎么写「把输入绑上」的逻辑。
2. `Input_Move` —— 怎么写「让角色前后左右移动」。
3. `Input_LookMouse` —— 怎么写「让镜头左右上下转」。

先回答一个**最关键、也最容易踩坑**的问题：

### 为什么不能照抄 Lyra 的 `GetPawn<APawn>()` / `GetController<APlayerController>()`？

Lyra 的 `Input_Move` 写在 `ULyraHeroComponent`（**组件**）里，所以它用：

```cpp
APawn* Pawn = GetPawn<APawn>();                       // ← 组件的方法：找自己挂在哪个 Pawn 上
AController* Controller = Pawn ? Pawn->GetController() : nullptr;
```

但我们的路线 B 把回调**搬进了角色类**，角色类**自己就是 Pawn**，没有 `GetPawn<APawn>()` 这个方法（那是 `UActorComponent` 的模板方法）。

所以搬到角色类后要改成：

```cpp
APawn* Pawn = this;                                   // ← 角色类自己就是 Pawn，直接用 this
AController* Controller = GetController();            // ← GetController() 是 AActor 的方法
```

> 这就是「减法重构」里最容易错的一处：**组件写法 → 角色类写法，要改「怎么拿自己」**。逻辑（移动的数学）一比一照抄，但「拿 Pawn / 拿控制器」的姿势要换成角色类自己的。

---

## 二、Lyra 真实源码（先看它怎么写）

### 1. 绑定逻辑（Lyra 在 `InitializePlayerInput` 里，我们精简进 `SetupPlayerInputComponent`）

Lyra 原文（`LyraHeroComponent.cpp` 第 274-289 行，已做减法抽出核心）：

```cpp
ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(PlayerInputComponent);
if (ensureMsgf(LyraIC, TEXT("...")))
{
	LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Move,       ETriggerEvent::Triggered, this, &ThisClass::Input_Move,       /*bLogIfNotFound=*/ false);
	LyraIC->BindNativeAction(InputConfig, LyraGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse,   /*bLogIfNotFound=*/ false);
}
```

### 2. 移动逻辑（`Input_Move`，Lyra 第 374-402 行）

```cpp
void ULyraHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}
```

### 3. 转视角逻辑（`Input_LookMouse`，Lyra 第 404-424 行）

```cpp
void ULyraHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}
```

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

| Lyra 有 | 我们这一步 | 为什么 |
|---------|-----------|--------|
| `InitializePlayerInput` 里一大段拿子系统/注册映射上下文/`ClearAllMappings`/`AddMappingContext` | **整个砍掉**，`SetupPlayerInputComponent` 里只留「Cast + BindNativeAction」 | 映射上下文注册是步4（要配资产），现在先只做绑定 |
| `ULyraLocalPlayer` / `GetSubsystem` | **砍掉** | 同上，步4 才需要 |
| `BindAbilityActions`（技能输入） | **砍掉** | 阶段三技能 |
| `Input_Crouch` / `Input_AutoRun` / `Input_LookStick` | **砍掉** | 只留移动 + 鼠标视角 |
| `SetIsAutoRunning(false)` | **砍掉** | 依赖 `ALyraPlayerController`，自动跑功能一并砍 |
| `LyraGameplayTags::InputTag_Move` 全局常量 | **改成 `FGameplayTag::RequestGameplayTag(TEXT("InputTag.Move"))`** | 全局 Tag 常量表是独立主题，先用运行时请求 |
| `GetPawn<APawn>()`（组件写法） | **改成 `this` / `GetController()`（角色类写法）** | 回调从组件搬进角色类，拿自己的姿势变了 |

> **保留（一比一照抄，只换前缀 + 改拿自己姿势）**：
> - `BindNativeAction(InputConfig, Tag, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false)` 绑定行
> - `Input_Move` 的移动数学（`MovementRotation` + `RotateVector` + `AddMovementInput`）
> - `Input_LookMouse` 的 `AddControllerYawInput` / `AddControllerPitchInput`

---

## 四、你要写的 `TenkaichiCharacterWithAbilities.cpp` 新增实现

文件路径（改现有文件）：

```
Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.cpp
```

### 顶部新增 include（要用的头文件）

```cpp
#include "Input/TenkaichiInputComponent.h"
#include "Input/TenkaichiInputConfig.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
```

### 新增三个函数实现

```cpp
void ATenkaichiCharacterWithAbilities::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 把引擎的输入组件转成我们自己那个（继承自 UEnhancedInputComponent）
	UTenkaichiInputComponent* TenkaichiIC = Cast<UTenkaichiInputComponent>(PlayerInputComponent);
	if (ensureMsgf(TenkaichiIC, TEXT("输入组件类型不对！应该用 UTenkaichiInputComponent 或其子类。")))
	{
		// 用「字典」把 Tag 查到 InputAction，再绑定到回调（只认 Tag，不认按键）
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Move")),       ETriggerEvent::Triggered, this, &ThisClass::Input_Move,     /*bLogIfNotFound=*/ false);
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Look.Mouse")), ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
	}
}

void ATenkaichiCharacterWithAbilities::Input_Move(const FInputActionValue& InputActionValue)
{
	// 角色类自己就是 Pawn，直接用 this；控制器用 GetController()（AActor 的方法）
	AController* Controller = GetController();

	if (Controller)
	{
		// 取出二维输入值：X = 左右，Y = 前后
		const FVector2D Value = InputActionValue.Get<FVector2D>();

		// 用「视角朝向的偏航角」当基准，把「前后左右」换算成世界方向
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			// 右向量绕视角旋转 → 得到「右」在世界里的方向，按 X 缩放
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			// 前向量绕视角旋转 → 得到「前」在世界里的方向，按 Y 缩放
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void ATenkaichiCharacterWithAbilities::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	// 取出二维输入值：X = 鼠标左右，Y = 鼠标上下
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		// 左右晃鼠标 → 偏航（左右转头）
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		// 上下晃鼠标 → 俯仰（抬头低头）
		AddControllerPitchInput(Value.Y);
	}
}
```

---

## 五、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `#include "Input/TenkaichiInputComponent.h"` | 要 `Cast<UTenkaichiInputComponent>` 和调 `BindNativeAction` |
| `#include "Input/TenkaichiInputConfig.h"` | 参数里用 `UTenkaichiInputConfig`（`InputConfig` 成员） |
| `#include "EnhancedInputComponent.h"` | `UEnhancedInputComponent` / `ETriggerEvent` 在这 |
| `#include "InputActionValue.h"` | `FInputActionValue` 类型在这 |
| `Super::SetupPlayerInputComponent(PlayerInputComponent);` | 先调基类（`ACharacter`）的默认处理，再补我们自己的 |
| `Cast<UTenkaichiInputComponent>(PlayerInputComponent)` | 把引擎传进来的 `UInputComponent*` 安全转成我们自己的输入组件；转失败返回空 |
| `ensureMsgf(TenkaichiIC, TEXT("..."))` | 断言非空，空就打日志；比 `check` 温和（不会崩，只报错） |
| `FGameplayTag::RequestGameplayTag(TEXT("InputTag.Move"))` | 运行时按名字请求一个 Tag（替代 Lyra 的全局常量 `LyraGameplayTags::InputTag_Move`） |
| `&ThisClass::Input_Move` | 取成员函数指针（`ThisClass` 是 UHT 生成的别名 = 当前类） |
| `ETriggerEvent::Triggered` | 触发时机 = 「按住持续触发」（移动/视角要持续响应） |
| `InputActionValue.Get<FVector2D>()` | 把输入值取成二维向量 |
| `Controller->GetControlRotation().Yaw` | 拿视角朝向的偏航角，用来把「前后左右」从玩家视角换算成世界方向 |
| `MovementRotation.RotateVector(FVector::RightVector)` | 把「右」向量按视角旋转，得到世界里的「右」方向 |
| `AddMovementInput(MovementDirection, Value.X)` | 往该方向加移动输入（`Value.X` 是力度/速度缩放） |
| `AddControllerYawInput(Value.X)` / `AddControllerPitchInput(Value.Y)` | 加偏航/俯仰旋转输入 |

---

## 六、这一步没教过的新方法（先扫一遍）

- **`SetupPlayerInputComponent(UInputComponent*)`**：引擎 `APawn` 虚函数，被 Possess 时自动调（P04-3b 已预告，这里落地）。
- **`Cast<UTenkaichiInputComponent>(...)`**：UE 的类型转换（向下转型），失败返回 `nullptr`，安全。
- **`ensureMsgf(...)`**：引擎断言宏，条件为假时打日志但不崩溃（比 `check` 温和）。
- **`FGameplayTag::RequestGameplayTag(TEXT("..."))`**：运行时按名字请求 Tag。
- **`AddMovementInput(FVector, float)`**：引擎 `APawn` 方法（已核实 `Pawn.h:486`），加移动输入。
- **`AddControllerYawInput(float)` / `AddControllerPitchInput(float)`**：引擎 `APawn` 方法（`Pawn.h:535`/`526`），加旋转输入。
- **`GetControlRotation()`**：引擎 `AController` 方法，拿当前视角朝向。
- **`RotateVector(...)`**：`FRotator` 方法，把向量按旋转转一下。
- **`GetController()`**：`AActor` 方法，拿控制自己的控制器。
- **`FInputActionValue` / `.Get<FVector2D>()`**：输入值类型 / 取二维向量。

> 这几个符号里，`AddMovementInput`、`AddControllerYawInput/PitchInput`、`GetControlRotation`、`SetupPlayerInputComponent` 是**重点新方法**，本步登记进 `已教方法.md`。

---

## 七、下一步

写完 `.cpp`，角色理论上就能移动 + 转视角了（但还差**步4**：配 `InputMappingContext` 映射上下文 + `InputAction` 资产 + `InputConfig` 资产，否则按键还没接到 InputAction 上，角色不会真的动）。

跟我说「懂了 / 下一步」，我先带你把本步新方法登记进 `已教方法.md`，然后进入 P04 步 4（配映射上下文，真正让按键生效）。