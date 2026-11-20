# P04-2b — 建 TenkaichiInputComponent（.h 声明）

> **本步定位**：P04 第 2 步的 `.h` 部分。上一步（P04-2）讲清了「为什么需要一个输入组件（接线员）」，这一步动手写 `TenkaichiInputComponent.h`。
>
> **先记住**：本步只写 `.h`（声明），`.cpp`（实现）是下一个 md `P04-2c`，别混在一起。

---

## 一、这一步要解决什么问题（先讲为什么）

上一步说好了：要建一个「接线员」`TenkaichiInputComponent`，它继承引擎的 `UEnhancedInputComponent`，并加一个 `BindNativeAction` 方法，做到「上层代码只传 Tag、不碰具体 InputAction 资产」。

但动手写之前，得先回答三个「为什么」：

1. **为什么要继承 `UEnhancedInputComponent`，而不是自己从零写？**
   因为「把 InputAction 绑定到回调」这件苦活，引擎已经做好了（`BindAction`）。我们继承它，就白拿这套能力，只需在上面加一层「查字典」的封装。**继承 = 复用引擎已有的绑定能力**。

2. **为什么 `BindNativeAction` 要写成「模板函数」（`template<class UserClass, typename FuncType>`）？**
   因为「回调函数」可能是**任意类的任意成员函数**（`Input_Move` 是 `AHero` 的成员，未来技能回调又可能是别的类）。模板能「适配任意类型的对象 + 任意类型的函数指针」，一份代码通用。如果写死成 `void BindNativeAction(AActor* Obj, void(AHero::*Func)())`，那只能绑 `AHero` 的移动函数，绑不了别的。

3. **为什么类名上有 `UCLASS(Config = Input)`？**
   这是告诉 UE：这个组件可以用 `Config` 系统从 `.ini` 配置文件里读配置（`[/Script/...]` 段）。Lyra 这么写是为后续输入配置留的口子，我们照抄。

> 类比：接线员 = 一个「会打电话的机器人」，它继承了电话机的「拨号」能力（`BindAction`），又加了自己的一套「查电话本」（`FindNativeInputActionForTag`）。模板函数 = 这个机器人不管你要打给「移动部」还是「技能部」，都能帮你接通。

---

## 二、Lyra 真实源码（先看它怎么写）

参考 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\LyraInputComponent.h`：

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "LyraInputConfig.h"

#include "LyraInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;

/**
 * ULyraInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class ULyraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	ULyraInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const ULyraInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const ULyraInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const ULyraInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const ULyraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

// ===== 模板函数：因为含模板，必须把实现写在 .h 里（不能放 .cpp） =====
template<class UserClass, typename FuncType>
void ULyraInputComponent::BindNativeAction(const ULyraInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}
```

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

按铁律「只做减法、说清删了什么」，对照 Lyra 我们这一步**暂时不教**：

| Lyra 有 | 我们这一步 | 为什么先不教 |
|---------|-----------|-------------|
| `BindAbilityActions` 模板函数 | **先删掉** | 技能输入是 GAS 那套，阶段三才讲；且依赖 `AbilityInputActions`（P04-1 已删） |
| 它的模板实现体（`.h` 里 `for` 循环那段） | **先删掉** | 同上 |

> 其余（`AddInputMappings`、`RemoveInputMappings`、`BindNativeAction` 模板 + 实现、`RemoveBinds`）**一比一照抄**，只把前缀 `Lyra` 换成 `Tenkaichi`。

---

## 四、你要写的 `TenkaichiInputComponent.h`（只换前缀 + 做上述减法）

文件路径（一比一对应 Lyra 的 `Input/` 目录）：

```
Source/Tenkaichi/Input/TenkaichiInputComponent.h
```

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "TenkaichiInputConfig.h"

#include "TenkaichiInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;

/**
 * UTenkaichiInputComponent
 *
 *	用「输入配置数据资产」来管理输入映射与绑定的组件。
 */
UCLASS(Config = Input)
class UTenkaichiInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

// ===== 模板函数：因为含模板，必须把实现写在 .h 里（不能放 .cpp） =====
template<class UserClass, typename FuncType>
void UTenkaichiInputComponent::BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}
```

---

## 五、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `#include "EnhancedInputComponent.h"` | 要继承 `UEnhancedInputComponent`，先引基类 |
| `#include "TenkaichiInputConfig.h"` | 方法参数里要用 `UTenkaichiInputConfig`，得先引它 |
| `#include "TenkaichiInputComponent.generated.h"` | UE 反射要求，**必须放最后一个 include** |
| `UCLASS(Config = Input)` | 告诉 UE 这个组件支持从 `.ini` 读配置（`Config=Input` 指定配置类别） |
| `class UTenkaichiInputComponent : public UEnhancedInputComponent` | 继承引擎输入组件，复用它的 `BindAction` 绑定能力 |
| `UTenkaichiInputComponent(const FObjectInitializer&)` | 标准构造函数签名，`.cpp` 里写 |
| `template<class UserClass, typename FuncType>` | 模板：让 `Object` 和 `Func` 可以是任意类、任意函数指针，通用 |
| `const FGameplayTag& InputTag` | 传 Tag（不是 InputAction 资产），保持「只认 Tag」的设计 |
| `ETriggerEvent TriggerEvent` | 触发时机（按下/按住/松开），调用时指定 |
| `RemoveBinds(TArray<uint32>&)` | 按句柄数组批量解绑，`uint32` 是绑定句柄类型 |
| `check(InputConfig)` | 断言：字典为空就崩溃，开发期立刻暴露 bug |
| `if (const UInputAction* IA = ...FindNativeInputActionForTag(...))` | 查字典；`if` 里定义变量 + 判空，是 C++ 常用写法（查到了才进花括号） |
| `BindAction(IA, TriggerEvent, Object, Func)` | 引擎的绑定函数（`UEnhancedInputComponent` 提供），把 InputAction 绑到回调 |

---

## 六、这一步没教过的新方法（先扫一遍）

- **`template`（模板函数）**：C++ 的关键字，让函数能「适配任意类型」。这里 `UserClass`、`FuncType` 是「占位类型」，编译器根据你调用时传的真实类型自动生成对应版本。
- **`BindAction`**：引擎 `UEnhancedInputComponent` 提供（已核实：`EnhancedInputComponent.h` 第 448 行附近），真正把「InputAction + 触发时机 + 对象 + 回调」绑在一起的方法。
- **`ETriggerEvent`**：触发时机枚举，常用值：`Started`（按下瞬间）、`Triggered`（按住持续）、`Completed`（松开）。移动/转视角用 `Triggered`。
- **`check(...)`**：引擎断言宏，条件为假就立刻崩溃并打印调用栈，开发期抓 bug 用（发布版会被编译掉）。
- **`UEnhancedInputComponent`**：引擎 Enhanced Input 插件的输入组件基类。
- **`TArray<uint32>` 句柄**：`BindAction` 返回一个「绑定句柄」（一个整数 ID），攒起来以后可以按句柄解绑。

> 这几个符号都在上面第 5 点「逐段回扣」里解释过了。若想深挖某个，再提。

---

## 七、下一步

写完 `.h`，先别急着动 `.cpp`。跟我说「懂了 / 下一步」，我再建 `P04-2c_建TenkaichiInputComponent_cpp.md`。