# P02-1 — 认识 GameMode：它到底管什么

> **对应 Lyra**：`Source/LyraGame/GameModes/LyraGameMode.h/.cpp`
>
> **一句话**：GameMode 是"一局游戏的规则中枢"——它决定这局用哪个角色（Pawn）、哪个控制器（PlayerController）、哪个 HUD、哪个 PlayerState。玩家按下 Play 时，引擎问 GameMode："默认生成什么 Pawn？" GameMode 用 `DefaultPawnClass` 回答。这一步先搞清"为什么需要它"，还不写代码。

---

## 一、这一小步要解决什么问题

P01 我们造好了一具"带 GAS 的身体"（`ATenkaichiCharacterWithAbilities`），但它现在**孤零零地躺在工程里**——玩家按下 Play，没有任何东西会自动把这个角色放进场景。

这一步要理解：谁来负责"把角色塞给玩家"？答案是 **GameMode**。搞清它的职责，下一步我们就能建自己的 `TenkaichiGameMode`，让角色自动出现。

**验收**：读完本文，能用自己的话说清"GameMode 管什么、`DefaultPawnClass` 是干嘛的、Possess 是谁干的"。本步不写代码。

---

## 二、GameMode 是什么（先查引擎真实定义）

引擎里 `AGameMode` 的类注释写得很清楚：

```29:37:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Classes\GameFramework\GameMode.h
/**
 * GameMode is a subclass of GameModeBase that behaves like a multiplayer match-based game.
 * It has default behavior for picking spawn points and match state.
 * If you want a simpler base, inherit from GameModeBase instead.
 */
UCLASS(MinimalAPI)
class AGameMode : public AGameModeBase
```

翻译：`AGameMode` 是 `AGameModeBase` 的子类，多了"选出生点 + 比赛状态机"的默认行为。它管一整局游戏。

### GameMode 手里握着哪几张"王牌"

这些字段都定义在基类 `AGameModeBase` 里（引擎源码核实）：

| 字段 | 作用 | 定义位置 |
|------|------|---------|
| `DefaultPawnClass` | 玩家的默认身体（Pawn）类 | `GameModeBase.h` 第 106-108 行 |
| `PlayerControllerClass` | 用哪个 PlayerController | `GameModeBase.h`（同区块） |
| `GameStateClass` | 这局游戏的全局状态类 | 同上 |
| `PlayerStateClass` | 每个玩家的状态类 | 同上 |
| `HUDClass` | 用哪个 HUD | 同上 |

> `DefaultPawnClass` 原文注释：`/** The default pawn class used by players. */`（`GameModeBase.h` 第 106-107 行）

**为什么需要它**：玩家（PlayerController）本身只是个"灵魂"，没有身体。GameMode 在玩家加入时，照着 `DefaultPawnClass` 造一具身体，并把它交给玩家控制——这个"灵魂附体"的动作就是 **Possess**。没有 GameMode 指定默认 Pawn，玩家进来就是个空 Controller，看不到也控不了任何东西。

---

## 三、Possess 是 GameMode 自动干的（不用我们手写）

这是本步最关键的认知。玩家加入时，引擎的 `AGameModeBase` 会自动走这条链路（引擎源码核实）：

```1231:1243:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Private\GameModeBase.cpp
APawn* AGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning, TEXT("SpawnDefaultPawnAtTransform: Couldn't spawn Pawn of type %s at %s"), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
	}
	return ResultPawn;
}
```

链路拆解：
1. 玩家加入 → GameMode 调 `SpawnDefaultPawnFor` / `SpawnDefaultPawnAtTransform`
2. 内部用 `GetDefaultPawnClassForController` 拿到 `DefaultPawnClass`
3. `SpawnActor` 把这具身体造出来
4. 引擎再 `SetPawn` → 内部触发 **Possess**，玩家就"附体"了

**结论**：我们**不用手写 Possess**。只要把 `DefaultPawnClass` 设成我们的角色类，剩下的引擎全自动完成。这就是为什么 P02 能让角色"自动出现"。

---

## 四、我们这版 vs Lyra 的差异（为什么用原生 AGameMode）

Lyra 的 GameMode 长这样：

```36:43:e:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\GameModes\LyraGameMode.h
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ALyraGameMode : public AModularGameModeBase
```

注意它继承的是 **`AModularGameModeBase`**（不是引擎原生 `AGameMode`）。这个 `ModularGameModeBase` 来自 GameFeatures 插件，**绑死了 Experience / GameFeature 体系**——那是阶段七（P15"世界可换"）才教的内容。

| 项 | Lyra | 我们（阶段一） | 说明 |
|---|------|--------------|------|
| 基类 | `AModularGameModeBase`（GameFeatures 插件） | `AGameMode`（引擎原生） | Lyra 的基类绑死 Experience 体系，阶段一先不碰 |
| 构造函数设的 Class | 一整套（GameState/GameSession/PlayerController/PlayerState/HUD/DefaultPawn） | 仅 `DefaultPawnClass` | 做减法：只聚焦"默认角色"这一个知识点，其余用引擎默认 |
| 默认 Pawn | `ALyraCharacter::StaticClass()` | `ATenkaichiCharacterWithAbilities::StaticClass()` | 复用 P01 成果 |

**为什么这么选**（对应拆解文档 P02 步 2 的说明）：阶段一的目标是"让角色自动出现"这一个知识点。如果一上来就用 Lyra 的 `AModularGameModeBase`，会被 Experience/GameFeature 这套外围基础设施淹没，得不偿失。所以先用引擎原生 `AGameMode`，把 Possess 链路吃透；Experience 体系留到阶段七再补。

> 这是"简化外围基础设施"（正确），不是"简化核心范式"（错误）——我们依然保留"GameMode 设 DefaultPawnClass → 引擎自动 Possess"这条 Lyra 真实的核心链路。

---

## 五、一句话结论

**GameMode = 一局游戏的规则中枢；它握着 `DefaultPawnClass` 等几张王牌。玩家按 Play 时，引擎的 `AGameModeBase` 自动照 `DefaultPawnClass` 造身体并 Possess，我们不用手写。阶段一用引擎原生 `AGameMode`（不碰 Lyra 的 Experience 体系），只设 `DefaultPawnClass` 这一个字段。**

---

## 六、下一步

**P02-2：建 `Source/Tenkaichi/GameModes/TenkaichiGameMode.h/.cpp`**——继承引擎原生 `AGameMode`，在构造函数里设 `DefaultPawnClass = ATenkaichiCharacterWithAbilities::StaticClass()`。写完编译，为下一步"让角色自动出现"打好代码基础。
