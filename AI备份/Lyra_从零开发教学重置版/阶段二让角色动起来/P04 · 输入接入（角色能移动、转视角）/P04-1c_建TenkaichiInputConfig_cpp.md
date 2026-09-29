# P04-1c — 建 TenkaichiInputConfig（.cpp 实现）

> **本步定位**：P04 第 1 步的 `.cpp` 部分。上一步（P04-1b）写好了 `.h` 声明，这一步写 `.cpp` 实现。
>
> **先记住**：这一步只写 `.cpp`。写完这一步，`TenkaichiInputConfig` 这个类就完整了（P04 第 1 步结束）。

---

## 一、这一步要解决什么问题（先讲为什么）

上一步的 `.h` 里，我们**声明**了三个东西，但都还没「落地」：

1. 构造函数 `UTenkaichiInputConfig(const FObjectInitializer&)`——只声明，没写函数体。
2. 查询函数 `FindNativeInputActionForTag(...)`——只声明，没写「怎么找」。

C++ 的规则是：**声明了就必须实现**，否则链接阶段报「未定义」错。所以这一步把这两个函数的「身体」补上。

> 类比：`.h` 是「菜单」（告诉别人有哪些菜），`.cpp` 是「后厨」（每道菜怎么做）。菜单上写了「红烧肉」，后厨就得有红烧肉的做法，不然客人点了做不出来。

---

## 二、Lyra 真实源码（先看它怎么写）

参考 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\LyraInputConfig.cpp`：

```cpp
#include "LyraInputConfig.h"
#include "LyraLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInputConfig)

ULyraInputConfig::ULyraInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* ULyraInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FLyraInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogLyra, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* ULyraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FLyraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogLyra, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
```

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

对照 Lyra，这一步要处理**两处差异**：

### 1. 删掉 `FindAbilityInputActionForTag` 函数（技能输入专用）

| Lyra 有 | 我们这一步 | 为什么 |
|---------|-----------|--------|
| `FindAbilityInputActionForTag` 的 `.cpp` 实现 | **删掉** | `.h` 里我们已经删了这个声明（见 P04-1b），是技能输入专用，阶段三才讲 |

> 规则：`.h` 里删了声明，`.cpp` 里对应的实现**也必须一起删**，两头成对（铁律 11 核对②）。

### 2. 日志通道 `LogLyra` → 换成我们自己的（关键，务必看懂）

Lyra 的 `.cpp` 里写了：

```cpp
#include "LyraLogChannels.h"          // Lyra 自己的日志通道头文件
UE_LOG(LogLyra, Error, TEXT("..."));  // 用 LogLyra 这个日志类别
```

**`LyraLogChannels.h` 是 Lyra 项目自定义的日志通道文件，我们项目还没有这个文件。** 这是两个选择，按铁律 19「能用简单方案别复杂化」+ 铁律 16「只做减法」，我们这一步用**引擎自带的 `LogTemp`** 临时顶上，不去新建日志通道文件（那是独立的另一个话题，不在这步塞进来）。

| Lyra 有 | 我们这一步 | 为什么 |
|---------|-----------|--------|
| `#include "LyraLogChannels.h"` | **不写**（改用引擎自带的日志） | 我们还没建自己的日志通道文件，不在这步引入 |
| `UE_LOG(LogLyra, Error, ...)` | `UE_LOG(LogTemp, Error, ...)` | `LogTemp` 是引擎自带的临时日志类别，无需额外头文件 |

> 注：将来如果要做正式的 `LogTenkaichi` 日志通道，那是另一个独立小主题，到那时再单独教。这一步先用 `LogTemp` 顶上，功能完全一样（能在输出日志里看到错误）。

---

## 四、你要写的 `TenkaichiInputConfig.cpp`（只换前缀 + 做上述减法）

文件路径（一比一对应 Lyra 的 `Input/` 目录）：

```
Source/Tenkaichi/Input/TenkaichiInputConfig.cpp
```

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputConfig)


UTenkaichiInputConfig::UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UTenkaichiInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// 遍历「原生输入动作」列表，逐个比对 Tag
	for (const FTenkaichiInputAction& Action : NativeInputActions)
	{
		// 找到匹配的条目（动作存在 且 Tag 相等），就返回它对应的 InputAction
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	// 没找到时，如果允许打日志，就报一条错误，方便排查
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
```

---

## 五、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `#include "TenkaichiInputConfig.h"` | 实现要看到自己在 `.h` 里的声明 |
| `#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputConfig)` | UE5 新的「内联生成代码」宏，替代老式 `.gen.cpp` include，加快编译（UE 的写法约定） |
| `UTenkaichiInputConfig::UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer) { }` | 构造函数：`.h` 里声明了，这里补空实现（`UDataAsset` 不需要在构造里做额外初始化，所以函数体是空的） |
| `for (const FTenkaichiInputAction& Action : NativeInputActions)` | 范围 for 循环，遍历字典里的每条「词条卡」，`const &` 避免拷贝 |
| `if (Action.InputAction && (Action.InputTag == InputTag))` | 两个条件：① 这条词条卡的 `InputAction` 非空；② Tag 和要找的相等。都满足才算「命中」 |
| `return Action.InputAction;` | 命中就返回对应的 InputAction（指针） |
| `if (bLogNotFound)` | 走到这说明没找到。`bLogNotFound` 是调用方传入的开关，决定要不要报错 |
| `UE_LOG(LogTemp, Error, ...)` | 报一条 Error 级日志，`%s` 填充 Tag 名和资产名，方便排查「为什么找不到」 |
| `*InputTag.ToString()` | `FGameplayTag` 的 `ToString()` 返回 `FString`，`*` 解引用成 C 字符串给 `%s` |
| `GetNameSafe(this)` | 安全地拿到这个资产的名字（空指针也不会崩） |
| `return nullptr;` | 确实没找到，返回空指针 |

---

## 六、这一步没教过的新方法（先扫一遍）

- **`UE_INLINE_GENERATED_CPP_BY_NAME(类名)`**：UE5 编译系统宏，用来替代老式 `#include "Xxx.generated.h"` 在 `.cpp` 里的对应物（`Xxx.gen.cpp`）。作用：告诉编译器「这个类的反射代码是内联生成的」。**新方法，先记住它是个「固定写法」**，每个 `.cpp` 文件顶部紧跟 include 之后写。
- **`GetNameSafe(this)`**：`UObject` 提供的安全取名函数，对象为空时返回「None」而不是崩溃。
- **`InputTag.ToString()`**：`FGameplayTag` 转成字符串，用于打印。

> 这几个符号都在上面第 5 点解释过了。`UE_INLINE_GENERATED_CPP_BY_NAME` 比较重要，先登记进 `已教方法.md`（见下一步）。

---

## 七、下一步

写完 `.cpp`，`TenkaichiInputConfig` 类就完整了，P04 第 1 步结束。跟我说「懂了 / 下一步」，我再带你把这一步新教的方法登记进 `已教方法.md`，然后进入 P04 第 2 步。