# P04-2c — 建 TenkaichiInputComponent（.cpp 实现）

> **本步定位**：P04 第 2 步的 `.cpp` 部分。上一步（P04-2b）写好了 `.h` 声明，这一步写 `.cpp` 实现。
>
> **先记住**：这一步只写 `.cpp`。写完这一步，`TenkaichiInputComponent` 这个类就完整了（P04 第 2 步结束）。

---

## 一、这一步要解决什么问题（先讲为什么）

上一步的 `.h` 里，我们**声明**了四个函数，但都还没「落地」：

1. 构造函数 `UTenkaichiInputComponent(const FObjectInitializer&)`——只声明，没写函数体。
2. `AddInputMappings(...)`——只声明，没写。
3. `RemoveInputMappings(...)`——只声明，没写。
4. `RemoveBinds(TArray<uint32>&)`——只声明，没写。

C++ 的规则是：**声明了就必须实现**，否则链接阶段报「未定义」错。所以这一步把它们的「身体」补上。

> 注：`BindNativeAction` 是模板函数，它的实现已经在 `.h` 里写了（上一步讲过：模板函数必须写 `.h`），所以 `.cpp` 里**不用再写它**。

> 类比：`.h` 是「菜单」，`.cpp` 是「后厨」。菜单上写了四道菜，后厨就得把四道菜都做出来。其中 `BindNativeAction` 这道菜是「现做的模板菜」，直接写在菜单（`.h`）上，后厨不用管。

---

## 二、Lyra 真实源码（先看它怎么写）

参考 `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\LyraInputComponent.cpp`：

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/LyraLocalPlayer.h"
#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInputComponent)

class ULyraInputConfig;

ULyraInputComponent::ULyraInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void ULyraInputComponent::AddInputMappings(const ULyraInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void ULyraInputComponent::RemoveInputMappings(const ULyraInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void ULyraInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
```

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

对照 Lyra，这一步的 `.cpp` 要处理**两处差异**：

### 1. 删掉两个用不到的 include

| Lyra 有 | 我们这一步 | 为什么 |
|---------|-----------|--------|
| `#include "Player/LyraLocalPlayer.h"` | **删掉** | 我们还没建本地玩家类，用不到 |
| `#include "Settings/LyraSettingsLocal.h"` | **删掉** | 我们还没建设置类，用不到 |

> 规则：`.cpp` 里的 include 只留「真的用到的」。删掉后，剩下 `#include "TenkaichiInputComponent.h"` 和 `#include "EnhancedInputSubsystems.h"`（后者是因为函数签名里出现了 `UEnhancedInputLocalPlayerSubsystem`）。

### 2. 日志通道（这步 `.cpp` 里其实没有 `UE_LOG`）

注意到 Lyra 这步的 `.cpp` **没有 `UE_LOG` 日志**（不像 P04-1c 有），所以**不用处理日志通道**。只有 P04-1c 那步才碰到 `LogLyra` 的问题。

---

## 四、你要写的 `TenkaichiInputComponent.cpp`（只换前缀 + 做上述减法）

文件路径（一比一对应 Lyra 的 `Input/` 目录）：

```
Source/Tenkaichi/Input/TenkaichiInputComponent.cpp
```

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInputComponent.h"

#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputComponent)

class UTenkaichiInputConfig;

UTenkaichiInputComponent::UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UTenkaichiInputComponent::AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 如果需要，可以在这里处理「从输入配置里添加映射」的自定义逻辑（Lyra 这里留空）
}

void UTenkaichiInputComponent::RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 如果需要，可以在这里处理「移除上面添加的映射」的自定义逻辑（Lyra 这里留空）
}

void UTenkaichiInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
```

---

## 五、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `#include "TenkaichiInputComponent.h"` | 实现要看到自己在 `.h` 里的声明 |
| `#include "EnhancedInputSubsystems.h"` | 函数参数里用了 `UEnhancedInputLocalPlayerSubsystem`，得先引它的头文件 |
| `#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputComponent)` | 上一步（P04-1c）讲过的 UE5 固定写法，`.cpp` 顶部紧跟 include 之后写 |
| `class UTenkaichiInputConfig;` | 前向声明（告诉编译器「这是个类，具体长啥样在别处」）。因为 `.h` 已经 include 了它，这里其实是 Lyra 原样保留的一行 |
| `UTenkaichiInputComponent::UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer) { }` | 构造函数空实现（不需要额外初始化） |
| `check(InputConfig); check(InputSubsystem);` | 断言两个参数非空，空就崩（开发期抓 bug） |
| `AddInputMappings` / `RemoveInputMappings` 的空函数体 | Lyra 里这两个就是**预留的钩子**，函数体只有注释，功能留空。我们照抄 |
| `for (uint32 Handle : BindHandles)` | 遍历所有「绑定句柄」 |
| `RemoveBindingByHandle(Handle)` | 按句柄逐个解绑（引擎 `UEnhancedInputComponent` 提供，已核实 `EnhancedInputComponent.h:432`） |
| `BindHandles.Reset()` | 解绑完清空数组，避免下次重复解绑 |

---

## 六、这一步没教过的新方法（先扫一遍）

- **`RemoveBindingByHandle(uint32 Handle)`**：引擎 `UEnhancedInputComponent` 提供的方法（已核实：`EnhancedInputComponent.h` 第 432 行），传入一个「绑定句柄」ID，就把这条绑定解掉。返回 `bool`（是否成功移除）。
- **`UEnhancedInputLocalPlayerSubsystem`**：Enhanced Input 的「本地玩家子系统」，负责管理输入映射上下文（`InputMappingContext`）。这一步只作为函数参数类型出现，真正用它要到 P04 步 4 配置映射时。
- **前向声明 `class UTenkaichiInputConfig;`**：只告诉编译器「这是个类名」，不引入完整定义，能加快编译。这里 Lyra 原样保留了这一行（虽然 `.h` 已经 include 过）。

> 其余（`check`、`UE_INLINE_GENERATED_CPP_BY_NAME`、`TArray<uint32>` 句柄）都在前面 P04-2b / P04-1c 讲过了。

---

## 七、下一步

写完 `.cpp`，`TenkaichiInputComponent` 类就完整了，P04 第 2 步结束。

下一步我会带你把这一、二两步新教的方法（`BindNativeAction`、`BindAction`、`RemoveBindingByHandle`、`check`、`ETriggerEvent`、`UEnhancedInputComponent` 等）登记进 `已教方法.md`。跟我说「懂了 / 下一步」。