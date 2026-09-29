# P02-4 — 让关卡用上 GameMode + 出生点（PIE 验收）

> **对应 Lyra**：Lyra 通过 `ALyraWorldSettings` + Experience 体系指定 GameMode；阶段一我们用**引擎原生方式**（项目设置 Default GameMode + 场景 PlayerStart），更直接。
>
> **一句话**：代码建好了 `ATenkaichiGameMode`，还要"告诉引擎用它"。这一步在项目设置里把默认 GameMode 指给我们的类，再在场景放一个 `PlayerStart` 决定出生位置，最后 PIE 验收——角色自动出现。

---

## 一、这一小步要解决什么问题

前三步我们写好了 `ATenkaichiGameMode`（构造函数里设了 `DefaultPawnClass`）。但**引擎还不知道要用它**——如果直接 PIE，引擎会用默认的 `AGameMode`（没有我们的角色），玩家进场什么都看不到。

这一步做两件事：① 让引擎用上我们的 GameMode；② 指定出生点。然后就能验收了。

---

## 二、用配置文件指定 GameMode（Lyra 真实做法，不动编辑器）

> ⚠️ **本步只改配置文件，不开编辑器**。编辑器找半天还卡，能写代码/配置搞定的就别动编辑器。

### 方式 A：写 `Config/DefaultEngine.ini`（推荐，Lyra 就是这么干的）

**Lyra 真实源码证据（不是我们编的）**——Lyra 就是在配置文件里指定 GameMode 的：

```65:68:e:\ue5\LyraStarterGame5.6\LyraStarterGame\Config\DefaultEngine.ini
[/Script/EngineSettings.GameMapsSettings]
GlobalDefaultGameMode=/Game/B_LyraGameMode.B_LyraGameMode_C
GameInstanceClass=/Game/B_LyraGameInstance.B_LyraGameInstance_C
GameDefaultMap=/Game/System/FrontEnd/Maps/L_LyraFrontEnd.L_LyraFrontEnd
```

`GlobalDefaultGameMode` 就是"项目设置 → Default GameMode"那个字段在 ini 里的样子（项目设置面板本质就是编辑这个 ini）。Lyra 直接写文件，我们照做。

**你要做的**：在 `Config/DefaultEngine.ini` 已有的 `[/Script/EngineSettings.GameMapsSettings]` 段里，加一行 `GlobalDefaultGameMode`：

```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Engine/Maps/Templates/OpenWorld
GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode
```

> ⚠️ **C++ 类 vs 蓝图的路径格式不同（关键，别写错）**：
> - Lyra 的 GameMode 是**蓝图** `B_LyraGameMode`，路径格式 `/Game/.../B_LyraGameMode.B_LyraGameMode_C`（带 `_C` 后缀）。
> - 我们的 GameMode 是**纯 C++ 类** `ATenkaichiGameMode`，路径格式 `/Script/模块名.类名` = `/Script/Tenkaichi.TenkaichiGameMode`（**不带 `_C`**）。
> - 规律：C++ 类用 `/Script/`，蓝图资产用 `/Game/` 且带 `_C`。

**为什么推荐**：改一个 ini 文件就全局生效，不用开编辑器、不用逐关配。这就是 Lyra 的做法，也是"能写配置就别动编辑器"的体现。

### 方式 B：关卡的 World Settings → GameMode Override（按关卡覆盖，编辑器方式）

**操作路径**：打开某关卡 → `Window → World Settings → GameMode → GameMode Override`，勾选后选 GameMode。

- 只对当前关卡生效，优先级高于项目默认。
- **阶段一不用**（我们用方式 A 全局配）。知道有这种方式即可，阶段七 P15"世界可换"、或不同关卡要不同 GameMode 时才会用到。

> 阶段一我们用**方式 A**（写 ini），一行配置搞定，不开编辑器。

---

## 三、放一个 PlayerStart 决定出生点

GameMode 造身体时，需要知道"在哪造"。这个位置由 `PlayerStart` Actor 决定。

**操作**：
1. 打开你的战斗关卡（如 `Lvl_PIE` 或新建一个关卡）。
2. 在 `Place Actors` 面板搜 `PlayerStart`，拖进场景。
3. 把它摆到你想要角色出现的位置（比如一片空地中央）。

**原理**（引擎源码核实）：`SpawnDefaultPawnFor` 会拿 `PlayerStart` 的位置和朝向来生成 Pawn：

```1220:1229:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Private\GameModeBase.cpp
APawn* AGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Don't allow pawn to be spawned with any pitch or roll
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();

	FTransform Transform = FTransform(StartRotation, StartLocation);
	return SpawnDefaultPawnAtTransform(NewPlayer, Transform);
}
```

**如果不放 PlayerStart**：引擎会用默认位置（通常是世界原点，或已有 Pawn 的位置）。能跑，但出生点不可控，不推荐。

---

## 四、PIE 验收

1. 确认：`Config/DefaultEngine.ini` 已加 `GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode`（方式 A）；场景里放好了 `PlayerStart`。
2. 点 **Play**（PIE）。
3. **预期现象**：一个 `ATenkaichiCharacterWithAbilities` 角色**自动出现**在 PlayerStart 位置——就是你 P01 建的那具带 GAS 的身体。
4. 它现在是**站桩**的（不会移动、不能转视角——那是阶段二 P04-P06"动起来"的事）。

**验收通过标准**：
- [ ] 角色自动出现在场景里（不用手写 Spawn / Possess）
- [ ] 位置在 PlayerStart 处
- [ ] 打开角色 Details，能看到身上挂着 ASC + HealthSet 子对象（P01 的成果）

---

## 五、常见坑

| 现象 | 原因 | 解决 |
|------|------|------|
| PIE 后场景里没人 | 没设 Default GameMode，或设错了 | 检查 `Config/DefaultEngine.ini` 有 `GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode`（注意 C++ 类不带 `_C`） |
| 角色出现在世界原点/奇怪位置 | 没放 PlayerStart | 场景里放一个 PlayerStart 并摆好位置 |
| 编译报错找不到 `ATenkaichiCharacterWithAbilities` | `.cpp` 没 `#include` 角色类头文件 | 见 P02-3，确认 include 了 `Character/TenkaichiCharacterWithAbilities.h` |
| 角色出现但没 GAS 组件 | 角色类构造函数没挂 ASC（P01 没做完） | 回 P01-4-3 检查 `CreateDefaultSubobject` |

---

## 六、一句话结论

**代码建好 GameMode 后，用"项目设置 → Default GameMode"指定它（方式 A，一劳永逸），再在场景放 `PlayerStart` 定出生点。PIE 后角色自动出现——Possess 全程引擎干的，我们一行没写。这就是 P02 的目标。**

---

## 七、P02 完成 → 下一步

P02 结束，你已掌握：
- [x] 理解 GameMode 的作用，能建自定义 GameMode 并设默认 Pawn
- [x] 理解 Possess 是 GameMode 自动完成的，不用手写
- [x] 会用项目设置指定 GameMode + 用 PlayerStart 定出生点

**接下来 P03 · 开始游戏 UI**：现在是一按 Play 就直接进战斗场景。P03 要加一个"开始游戏"界面——启动先进主菜单，点"开始游戏"按钮才 `OpenLevel` 进战斗关卡。届时会引入 UMG Widget、按钮回调、关卡跳转。
