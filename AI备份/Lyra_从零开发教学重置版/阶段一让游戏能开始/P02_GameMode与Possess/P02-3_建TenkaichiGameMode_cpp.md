# P02-3 — 建 `TenkaichiGameMode.cpp`（构造函数设默认 Pawn）

> **对应 Lyra**：`Source/LyraGame/GameModes/LyraGameMode.cpp`（第 33-43 行构造函数）
>
> **一句话**：实现 `TenkaichiGameMode` 的构造函数，在里面写 `DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();`——告诉引擎"玩家的默认身体用我们 P01 建好的角色类"。这一行就是让角色自动出现的关键。

---

## 一、这一小步要解决什么问题

上一步（P02-2）搭好了 `.h` 骨架，声明了构造函数但还没实现。

这一步实现构造函数，做**唯一一件**该做的事：把 `DefaultPawnClass` 设成我们的角色类。设完之后，玩家按 Play，引擎就会自动照这个类造身体并 Possess（P02-1 讲过的自动链路）。

**验收**：`.cpp` 建好、整体编译通过。

---

## 二、Lyra 是怎么写的（先查真实源码）

Lyra 的构造函数一口气设了一整套 Class：

```33:43:e:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\GameModes\LyraGameMode.cpp
ALyraGameMode::ALyraGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ALyraGameState::StaticClass();
	GameSessionClass = ALyraGameSession::StaticClass();
	PlayerControllerClass = ALyraPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = ALyraReplayPlayerController::StaticClass();
	PlayerStateClass = ALyraPlayerState::StaticClass();
	DefaultPawnClass = ALyraCharacter::StaticClass();
	HUDClass = ALyraHUD::StaticClass();
}
```

我们**做减法**：只保留 `DefaultPawnClass` 这一行（其余 GameState/HUD/PlayerController 等，阶段一用引擎默认即可，将来需要时再逐个加）。

---

## 三、关键点：`DefaultPawnClass = ...::StaticClass()`

- `DefaultPawnClass` 是 `AGameModeBase` 的字段（`GameModeBase.h` 第 106-108 行，类型 `TSubclassOf<APawn>`）。
- `ATenkaichiCharacterWithAbilities::StaticClass()` 拿到我们角色类的 `UClass*`。
- 两者一赋值，引擎在玩家加入时就会照这个类造 Pawn 并自动 Possess（P02-1 的 `SpawnDefaultPawnAtTransform_Implementation` 链路）。

**为什么复用 P01 的角色类**：P01 我们造好了带 ASC + HealthSet 的 `ATenkaichiCharacterWithAbilities`。现在 GameMode 把它设为默认 Pawn——P01 的成果在这一刻"活"了起来，玩家一进场就拥有这具带 GAS 的身体。

---

## 四、你要写的代码（`Source/Tenkaichi/GameModes/TenkaichiGameMode.cpp`）

```cpp
#include "TenkaichiGameMode.h"
#include "Character/TenkaichiCharacterWithAbilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameMode)

ATenkaichiGameMode::ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 玩家的默认身体 = 我们 P01 建好的带 GAS 角色类（对应 LyraGameMode.cpp 第 41 行）
	DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();
}
```

> `#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameMode)` 是 UE5 现代写法（Lyra 源码里也用，见 `LyraGameMode.cpp` 第 31 行），放在 include 之后、构造函数之前。

---

## 五、逐段回扣"为什么"

| 代码 | 为什么这么写 |
|------|-------------|
| `#include "TenkaichiGameMode.h"` | 实现自己的类，先引自己的头文件 |
| `#include "Character/TenkaichiCharacterWithAbilities.h"` | 要用 `ATenkaichiCharacterWithAbilities::StaticClass()`，必须引角色类完整定义（前置声明不够，`StaticClass()` 需要完整类型） |
| `#include UE_INLINE_GENERATED_CPP_BY_NAME(...)` | UE5 标准，引入本类的反射生成代码（对应 Lyra `.cpp` 第 31 行） |
| `: Super(ObjectInitializer)` | 把初始化参数透传给基类 `AGameMode`，UE 构造规范 |
| `DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();` | 核心一行：设默认 Pawn，引擎据此自动造身体 + Possess |

---

## 六、核对（.h ↔ .cpp 成对）

- `.h` 声明的构造函数：`ATenkaichiGameMode(const FObjectInitializer&)`
- `.cpp` 实现的构造函数：`ATenkaichiGameMode::ATenkaichiGameMode(const FObjectInitializer& ObjectInitializer)`

名字、参数完全一致（配合 11 号铁律）。

---

## 七、我们这版 vs Lyra 的差异

| 项 | Lyra | 我们 | 说明 |
|---|------|------|------|
| 设的 Class 数量 | 7 个（GameState/GameSession/PlayerController/ReplaySpectator/PlayerState/DefaultPawn/HUD） | 1 个（DefaultPawn） | 做减法：阶段一只聚焦"默认角色"，其余用引擎默认 |
| 默认 Pawn | `ALyraCharacter::StaticClass()` | `ATenkaichiCharacterWithAbilities::StaticClass()` | 复用 P01 成果 |
| `UE_INLINE_GENERATED_CPP_BY_NAME` | 有（第 31 行） | 有 | 一比一还原 |

---

## 八、一句话结论

**`TenkaichiGameMode.cpp` 的构造函数只做一件事：`DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass();`。这一行让引擎在玩家加入时自动造出带 GAS 的角色并 Possess。Lyra 设了 7 个 Class，我们做减法只留最关键的这一个。**

---

## 九、下一步

**P02-4：让关卡用上这个 GameMode + 出生点**——在 `Config/DefaultEngine.ini` 写一行 `GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode`（Lyra 真实做法，不动编辑器，见 19 号铁律），并在场景里放一个 `PlayerStart` 决定出生位置。然后 PIE 验收：角色自动出现在场景里（站桩，还不会动——动是阶段二 P04 的事）。
