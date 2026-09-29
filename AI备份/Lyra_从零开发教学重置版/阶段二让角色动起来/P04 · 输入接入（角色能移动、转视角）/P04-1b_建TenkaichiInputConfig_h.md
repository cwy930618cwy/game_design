# P04-1b — 建 TenkaichiInputConfig（.h 声明）

> **本步定位**：P04 第 1 步的 `.h` 部分。上一步（P04-1）讲清了「三件套 + 为什么用 GameplayTag 当翻译官」，这一步动手写 `TenkaichiInputConfig.h`。
>
> **先记住**：本步只写 `.h`（声明），`.cpp`（实现）是下一个 md `P04-1c`，别混在一起。

---

## 一、这一步要解决什么问题（先讲为什么）

上一步说好了：要建一本「翻译字典」，把 `InputTag.Move`（Tag）→ `IA_Move`（InputAction）对应起来。

但要建这本字典，得先回答两个「为什么」：

1. **为什么这本字典是个 `UDataAsset`（数据资产），不是个普通 C++ 类？**
   因为「Move 对应哪个 Tag」这种事，**是配置，不是逻辑**。逻辑（代码）应该永远不变，而「哪个键触发哪个动作」应该交给策划/美术在编辑器里填。`UDataAsset` 就是 UE 专门用来存「一堆配置数据」的资产类型——你在编辑器里建一个 `DA_TenkaichiInputConfig` 资产，往它的数组里填「Tag → InputAction」条目就行，不用改代码。

2. **为什么「一个条目」要单独做一个结构体 `FTenkaichiInputAction`，而不是两个平行数组？**
   因为「Tag」和「InputAction」**必须成对出现**。用结构体把两者绑成一个整体（`{ Tag, InputAction }`），放进一个 `TArray<FTenkaichiInputAction>`，就天然保证「一对一对」不会错位；如果用两个数组（一个存 Tag、一个存 Action），靠下标对应，一旦删错一条就全错位了。

> 类比：字典 = 一个「词条本」，结构体 = 一页「词条卡」（正面 Tag、背面 InputAction），数组 = 这本本子装下的所有词条卡。

---

## 二、Lyra 真实源码（先看它怎么写）

参考 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\LyraInputConfig.h`，核心就一个结构体 + 一个类：

```cpp
// ===== 结构体：一条「Tag ↔ InputAction」对应 =====
USTRUCT(BlueprintType)
struct FLyraInputAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UInputAction> InputAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
    FGameplayTag InputTag;
};

// ===== 类：字典本体（一个 UDataAsset） =====
UCLASS(BlueprintType, Const)
class ULyraInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    ULyraInputConfig(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
    const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

    UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
    const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FLyraInputAction> NativeInputActions;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
    TArray<FLyraInputAction> AbilityInputActions;
};
```

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

按铁律「只做减法、说清删了什么」，对照 Lyra 我们这一步**暂时不教**：

| Lyra 有 | 我们这一步 | 为什么先不教 |
|---------|-----------|-------------|
| `FindAbilityInputActionForTag`（技能输入查找） | **先删掉这个函数声明** | 它是「技能输入」那套（把 Tag 对应到 GAS 技能），阶段三才讲，现在用不到 |
| `AbilityInputActions` 数组 | **先删掉这个成员** | 同上，技能输入专用，现在没技能 |
| `ULyraInputConfig(const FObjectInitializer&)` 构造函数 | 保留（`.cpp` 里写空实现，见 P04-1c） | 这是 `UDataAsset` 的标准写法，保留 |

> 其余（结构体 `FTenkaichiInputAction`、类 `UTenkaichiInputConfig`、`NativeInputActions`、`FindNativeInputActionForTag`）**一比一照抄**，只把前缀 `Lyra` 换成 `Tenkaichi`。

---

## 四、你要写的 `TenkaichiInputConfig.h`（只换前缀 + 做上述减法）

文件路径（一比一对应 Lyra 的 `Input/` 目录）：

```
Source/Tenkaichi/Input/TenkaichiInputConfig.h
```

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "TenkaichiInputConfig.generated.h"

class UInputAction;
class UObject;
struct FFrame;

/**
 * FTenkaichiInputAction
 *
 *	用于把「一个输入动作（InputAction）」映射到「一个 GameplayTag」的结构体。
 */
USTRUCT(BlueprintType)
struct FTenkaichiInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * UTenkaichiInputConfig
 *
 *	不可变的数据资产，用来存放输入配置。
 */
UCLASS(BlueprintType, Const)
class UTenkaichiInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// 宿主使用的输入动作列表。这些输入动作映射到 GameplayTag，需要手动绑定。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTenkaichiInputAction> NativeInputActions;
};
```

---

## 五、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `#include "Engine/DataAsset.h"` | 要继承 `UDataAsset`，得先把基类引进来 |
| `#include "GameplayTagContainer.h"` | 要用 `FGameplayTag`，它在这个头文件里 |
| `#include "TenkaichiInputConfig.generated.h"` | UE 反射系统要求，且**必须放最后一个 include** |
| `USTRUCT(BlueprintType)` | 让结构体也能进蓝图/编辑器（策划能在资产里看到、编辑它） |
| `TObjectPtr<const UInputAction> InputAction` | 用 `TObjectPtr`（UE5 新指针），`const` 表示这本字典「只读不写」动作资产 |
| `Meta = (Categories = "InputTag")` | 限制 Tag 选择器只显示 `InputTag` 分类下的 Tag，防止填错分类 |
| `UCLASS(BlueprintType, Const)` | `Const` 表示这是个「不可变」数据资产（运行时不改） |
| `class UTenkaichiInputConfig : public UDataAsset` | 继承 `UDataAsset`，告诉 UE「我是个配置资产」 |
| `UTenkaichiInputConfig(const FObjectInitializer&)` | `UDataAsset` 的标准构造函数签名，`.cpp` 里写 |
| `FindNativeInputActionForTag(...)` | 核心查询函数：给定 Tag → 找到对应 InputAction（`const` 表示不改数据，`bLogNotFound` 控制找不到时要不要打日志） |
| `TArray<FTenkaichiInputAction> NativeInputActions` | 字典本体：一数组的「词条卡」 |

---

## 六、这一步没教过的新方法（先扫一遍）

- **`TObjectPtr`**：UE5 新的「智能指针」写法，用来替代裸指针 `UInputAction*`，能自动跟踪引用、防止野指针。我们 P01 里没用过，这里第一次出现。
- **`UDataAsset`**：数据资产基类，专门存「纯配置数据」的类。
- **`FGameplayTag`**：GameplayTag 的类型，一个「分层标签」（如 `InputTag.Move`）。
- **`Meta = (Categories = ...)` / `TitleProperty = ...`**：`UPROPERTY` 的编辑器修饰，控制资产里这个字段在编辑器里怎么显示。

> 这几个符号都在上面第 5 点「逐段回扣」里解释过了，先不单独建详解 md。若想深挖，再提。

---

## 七、下一步

写完 `.h`，先别急着动 `.cpp`。跟我说「懂了 / 下一步」，我再建 `P04-1c_建TenkaichiInputConfig_cpp.md`。