# P02-2 — 建 `TenkaichiGameMode.h`（GameMode 头文件骨架）

> **对应 Lyra**：`Source/LyraGame/GameModes/LyraGameMode.h`（第 36-43 行类声明）
>
> **一句话**：建 `GameModes/TenkaichiGameMode.h`，继承引擎原生 `AGameMode`（不是 Lyra 的 `AModularGameModeBase`），声明一个构造函数。本小步只写 `.h`，构造函数里设 `DefaultPawnClass` 的 `.cpp` 留到 P02-3。

---

## 一、这一小步要解决什么问题

上一步（P02-1）结论：**GameMode 是一局游戏的中枢，玩家的默认身体由 `DefaultPawnClass` 决定，Possess 由引擎自动完成**。

这一步去建我们自己的 GameMode 类。本小步只搭 `.h` 头文件——声明"继承谁、构造函数叫什么"，真正设 `DefaultPawnClass` 的 `.cpp` 放到 P02-3。

**验收**：`.h` 建好、能编译（配合 P02-3 的 `.cpp` 后整体通过）。

---

## 二、命名：一比一对应 Lyra（重要）

| Lyra 真实 | Tenkaichi 对应 |
|-----------|---------------|
| `ALyraGameMode` | `ATenkaichiGameMode` |
| `LyraGameMode.h/.cpp` | `TenkaichiGameMode.h/.cpp` |
| 路径 `Source/LyraGame/GameModes/` | `Source/Tenkaichi/GameModes/` |

> ⚠️ 铁律提醒（01/15 号）：Lyra 叫什么名，我们就叫什么名，**只换前缀 `Lyra`→`Tenkaichi`**，不砍后缀、不擅自简化。

---

## 三、两个关键点（先讲清为什么，再给代码）

### 关键点 ①：继承引擎原生 `AGameMode`（不是 `AModularGameModeBase`）

Lyra 的 GameMode 继承 `AModularGameModeBase`（来自 GameFeatures 插件，绑死 Experience 体系）。我们阶段一**不碰**那套，直接继承引擎原生 `AGameMode`：

```34:37:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Classes\GameFramework\GameMode.h
UCLASS(MinimalAPI)
class AGameMode : public AGameModeBase
{
	GENERATED_UCLASS_BODY()
```

**为什么**：`AGameMode` 已经自带"选出生点 + 比赛状态机"的默认行为，够我们阶段一用了。Experience/GameFeature 是阶段七（P15）的内容，现在引入只会徒增复杂度。这是"简化外围基础设施"，核心链路（设 DefaultPawnClass → 引擎自动 Possess）完整保留。

### 关键点 ②：构造函数用 `const FObjectInitializer&`（照 Lyra）

Lyra 的构造函数签名：

```43:43:e:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\GameModes\LyraGameMode.h
	UE_API ALyraGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
```

我们一比一还原这个签名（去掉 Lyra 的 `UE_API` 导出宏，那是插件 DLL 导出用的，我们单模块不需要）。`FObjectInitializer` 是 UE 构造对象时的标准参数，`Super(ObjectInitializer)` 会把它透传给基类。

---

## 四、你要写的代码（`Source/Tenkaichi/GameModes/TenkaichiGameMode.h`）

```cpp
#pragma once

#include "GameFramework/GameMode.h"
#include "TenkaichiGameMode.generated.h"

class APawn;
class AController;

UCLASS()
class ATenkaichiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// 一比一还原 Lyra 的构造函数签名（去掉插件专用的 UE_API 导出宏）
	ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
```

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "GameFramework/GameMode.h"` | 要继承 `AGameMode`，得先引进基类头文件 |
| 前置声明 `class APawn;` / `class AController;` | 头文件里暂时只用到指针，前置声明即可，避免多余 `#include`（同 Lyra `.h` 第 11-16 行做法） |
| `class ATenkaichiGameMode : public AGameMode` | 类名一比一对应 Lyra 的 `ALyraGameMode`；基类用引擎原生 `AGameMode`（不接 Lyra 的 `AModularGameModeBase`） |
| `GENERATED_BODY()` | UE 反射宏，`UCLASS` 必备 |
| 构造函数 `ATenkaichiGameMode(const FObjectInitializer&)` | 一比一还原 Lyra 签名，`.cpp` 里在这里设 `DefaultPawnClass` |

---

## 六、核对预告（.h ↔ .cpp 成对）

这个 `.h` 声明了 **1 个函数**：构造函数 `ATenkaichiGameMode(const FObjectInitializer&)`。

下一小步 P02-3 的 `.cpp` 会实现它，名字 / 参数完全一致（配合 11 号铁律：.h 声明什么，.cpp 就实现什么）。

---

## 七、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 类名 | `ALyraGameMode` | `ATenkaichiGameMode` | 一比一对应，只换前缀 |
| 基类 | `AModularGameModeBase`（GameFeatures 插件） | `AGameMode`（引擎原生） | 阶段一不碰 Experience 体系 |
| 构造函数参数 | `const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()` | 同（去掉 `UE_API`） | 一比一还原 |
| 导出宏 | `UE_API`（= `LYRAGAME_API`） | 无 | 单模块不需要 DLL 导出 |

---

## 八、一句话结论

**`TenkaichiGameMode.h` = 类名 `ATenkaichiGameMode`（一比一对应 Lyra）+ 继承引擎原生 `AGameMode` + 一个构造函数 `ATenkaichiGameMode(const FObjectInitializer&)`。`.cpp` 里再设 `DefaultPawnClass`。**

---

## 九、下一步

**P02-3：写 `Source/Tenkaichi/GameModes/TenkaichiGameMode.cpp`**——实现构造函数，在里面写 `DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();`（对应 Lyra `LyraGameMode.cpp` 第 41 行 `DefaultPawnClass = ALyraCharacter::StaticClass();`）。
