# P04-4c — 角色类写「注册映射上下文」逻辑（.cpp）

> **本步定位**：P04 第 4 步的 `.cpp` 部分。上一步（P04-4b）在 `.h` 里加了 `DefaultInputMappingContext` 成员，这一步在**已有的** `SetupPlayerInputComponent` 里补上「拿输入子系统 → 清空旧映射 → 注册 IMC」这段代码，让按键真正接到 InputAction 上。
>
> **⚠️ 写代码边界（务必记住）**：你工程 `Source\` 里的代码**由你自己写**，我只讲解、给对照。下面的代码是「教你怎么改」，请你照着一比一写进 `.cpp`，别让我代改。

---

## 一、这一步要解决什么问题（先讲为什么）

### 1. 现在卡在哪？

P04-3 已经在 `SetupPlayerInputComponent` 里写好了两行 `BindNativeAction`（把 `Input_Move` / `Input_LookMouse` 绑到 Tag 上）。但**最前面一层还没接通**：

```
按键(WASD/鼠标)  →  [???]  →  InputAction  →  Input_Move / Input_LookMouse 回调
                     ↑
               这层还没接通
```

这一层靠的是「**映射上下文**」（`InputMappingContext`，简称 IMC）。IMC 是一张表，写着「W 键 → `IA_Move` 的 Y 分量」「鼠标 X → `IA_Look` 的 X 分量」……

**光有 IMC 资产还不够，必须在代码里把它「注册」进输入子系统，引擎才会去读它。** 这一步要写的，就是「注册」这段代码。

### 2. 注册 = 往「输入子系统」里塞 IMC

引擎里有个 `UEnhancedInputLocalPlayerSubsystem`（本地玩家输入子系统），它像一个**总开关板**：

- 把一个个 IMC「插」到开关板上（`AddMappingContext`），它才生效；
- 插的时候给个「优先级」（数字越大越先响应）；
- 想清空开关板，用 `ClearAllMappings`。

> **类比**：IMC 资产是「一张写好的接线图」，输入子系统是「墙上的插座板」。光有接线图没用，你得把它**插进插座板**，电流才通。`AddMappingContext` 就是「插」这个动作，`Priority` 就是「插哪个孔（优先级）」。

---

## 二、Lyra 真实源码（先看它怎么写）

`LyraHeroComponent.cpp` 第 225-269 行，抽出「注册映射上下文」的核心：

```cpp
void ULyraHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>())
		{
			if (const ULyraInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.LoadSynchronous())
					{
						// ...（省略 bRegisterWithSettings / 用户设置）
						Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
					}
				}
				// ...（后面接 Cast 输入组件 + BindNativeAction，P04-3 已做过）
			}
		}
	}
}
```

> 关键三句（我们要保留的核心）：
> 1. `Subsystem->ClearAllMappings();` —— 清空旧映射。
> 2. `Subsystem->AddMappingContext(IMC, Priority, Options);` —— 把 IMC 插进子系统。
> 3. 前面拿 `Subsystem` 的链路：`PlayerController → LocalPlayer → GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()`。

---

## 三、这一步的减法（相对 Lyra，我们砍掉什么、为什么）

| Lyra 有 | 我们这一步 | 为什么 |
|---------|-----------|--------|
| `InitializePlayerInput` 独立函数 + 一长串 `GetPawn`/`GetController`/`GetLocalPlayer` | **精简成**「在 `SetupPlayerInputComponent` 里拿到 `PlayerController` 即可」 | 我们没建 HeroComponent，注册逻辑直接并入角色类已有的 `SetupPlayerInputComponent` |
| `ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(PC->GetLocalPlayer())` | **改成** `ULocalPlayer* LP = PC->GetLocalPlayer()` | Lyra 自定义的 `ULyraLocalPlayer` 我们没建，用引擎原生 `ULocalPlayer` |
| `ULyraPawnExtensionComponent` / `ULyraPawnData` 层层拿 `InputConfig` | **砍掉** | 我们没建这些，IMC 引用直接放角色类成员（P04-4b 已加） |
| `DefaultInputMappings` 数组 + `bRegisterWithSettings` + 用户设置 | **砍掉**，直接一个 IMC 成员 + `AddMappingContext` | 只需要一个默认 IMC，不用数组/用户设置 |
| `LoadSynchronous()`（软引用加载） | **改成** `TObjectPtr<UInputMappingContext>` 硬引用成员直接拿来用 | 简化，编辑器直指资产 |
| `check(...)` 一长串 | **酌情保留**，拿子系统处用 `ensureMsgf` | 保持温和报错，不崩 |
| `FModifyContextOptions Options` 变量 | **省略**，直接传默认构造 | 我们不需要定制注册选项，用默认值 |

> **保留（一比一照抄）**：
> - `ClearAllMappings()` —— 清空旧映射
> - `AddMappingContext(IMC, Priority, Options)` —— 注册核心
> - `GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()` —— 拿输入子系统
> - `GetLocalPlayer()` —— 从控制器拿本地玩家

---

## 四、这一步新出现、需要先教的方法（逐个讲清）

### 1. `GetLocalPlayer()`

- **干嘛**：`APlayerController` 的方法，返回它所属的「本地玩家」`ULocalPlayer*`。
- **源码**：`Engine/Source/Runtime/Engine/Classes/GameFramework/PlayerController.h:1803`：`ENGINE_API class ULocalPlayer* GetLocalPlayer() const;`
- **为什么用**：输入子系统 `UEnhancedInputLocalPlayerSubsystem` 是挂在「本地玩家」身上的（它继承 `ULocalPlayerSubsystem`），所以要先从控制器拿到本地玩家，才能从本地玩家身上拿子系统。

### 2. `GetSubsystem<T>()`

- **干嘛**：模板方法，从某个「子系统宿主」身上取指定类型的子系统。这里是 `ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()`。
- **源码**：`ULocalPlayer` 继承自 `UObject`，`GetSubsystem` 由 `USubsystem` 体系提供（`Subsystem.h` 里的模板方法）。
- **为什么用**：输入子系统不是全局单例，而是每个本地玩家一个；必须通过「本地玩家」这个宿主去取。

### 3. `ClearAllMappings()`

- **干嘛**：清空输入子系统里所有已注册的映射上下文。
- **源码**：`EnhancedInputSubsystemInterface.h:247`：`virtual void ClearAllMappings();`
- **为什么用**：防止旧的 IMC 残留、或重复注册导致按键重复触发。注册前先清一遍，保证干净。

### 4. `AddMappingContext(IMC, Priority, Options)`

- **干嘛**：把一个 IMC 注册进输入子系统，并指定优先级。
- **源码**：`EnhancedInputSubsystemInterface.h:256`：`virtual void AddMappingContext(const UInputMappingContext* MappingContext, int32 Priority, const FModifyContextOptions& Options = FModifyContextOptions());`
- **为什么用**：这是「让按键生效」的核心动作。`Priority` 数字越大越先响应（我们只有一个 IMC，传 `0` 即可）；`Options` 是可选配置，这里用默认值（直接省略第三个参数）。

### 5. `FModifyContextOptions`

- **干嘛**：注册/移除映射上下文时的可选配置结构体（控制「注册瞬间已按下的键怎么处理」等）。
- **源码**：`EnhancedInputSubsystemInterface.h:48`，有默认构造函数，三个字段都是默认值（`bIgnoreAllPressedKeysUntilRelease=true` 等）。
- **为什么用**：我们不需要定制，直接传默认构造（甚至省略这个参数）。这里提它，是让你知道 `AddMappingContext` 第三个参数是什么、为什么可以不写。

---

## 五、你要在 `.cpp` 里改的东西

文件路径（改现有文件）：

```
Source/Tenkaichi/Character/TenkaichiCharacterWithAbilities.cpp
```

### 1. 顶部新增两个 include

```cpp
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
```

> - `EnhancedInputSubsystems.h` → 提供 `UEnhancedInputLocalPlayerSubsystem`（引擎核实：`EnhancedInputSubsystems.h:21`）。
> - `InputMappingContext.h` → 提供 `UInputMappingContext` 的完整定义（`.cpp` 里要解引用指针，得 include 完整定义）。

### 2. 在 `SetupPlayerInputComponent` 里，`BindNativeAction` **之前**补注册逻辑

```cpp
void ATenkaichiCharacterWithAbilities::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 把引擎的输入组件转成我们自己那个（继承自 UEnhancedInputComponent）
	UTenkaichiInputComponent* TenkaichiIC = Cast<UTenkaichiInputComponent>(PlayerInputComponent);
	if (ensureMsgf(TenkaichiIC, TEXT("输入组件类型不对！应该用 UTenkaichiInputComponent 或其子类。")))
	{
		// ===== 注册映射上下文：让「按键 → InputAction」这张表生效 =====
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 从控制器拿本地玩家，再从本地玩家身上拿「输入子系统」
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					// 先清空旧映射，再注册我们自己的 IMC（优先级 0 即可，只有一个）
					Subsystem->ClearAllMappings();
					Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
				}
			}
		}

		// 用「字典」把 Tag 查到 InputAction，再绑定到回调（只认 Tag，不认按键）
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Move")),       ETriggerEvent::Triggered, this, &ThisClass::Input_Move,     /*bLogIfNotFound=*/ false);
		TenkaichiIC->BindNativeAction(InputConfig, FGameplayTag::RequestGameplayTag(TEXT("InputTag.Look.Mouse")), ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
	}
}
```

---

## 六、逐段回扣「为什么这么写」

| 代码 | 为什么 |
|------|--------|
| `Cast<APlayerController>(GetController())` | `GetController()` 返回 `AController*`，要拿 `GetLocalPlayer()` 得先 Cast 成 `APlayerController`（`GetLocalPlayer` 是 `APlayerController` 的方法） |
| `PC->GetLocalPlayer()` | 输入子系统挂在「本地玩家」身上，先拿到它 |
| `LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()` | 从本地玩家身上取输入子系统（每个本地玩家一个） |
| `Subsystem->ClearAllMappings()` | 注册前清空，防止残留/重复 |
| `Subsystem->AddMappingContext(DefaultInputMappingContext, 0)` | 核心动作：把 IMC 插进子系统；`0` 是优先级（只有一个 IMC，固定 0 即可）；省略第三个 `Options` 参数用默认值 |
| 整段套三层 `if`（PC → LocalPlayer → Subsystem） | 每一层都可能拿不到（nullptr），用 `if` 链式保护，比 Lyra 的 `check` 更温和，避免崩 |

---

## 七、⚠️ 现状勘误：`.h` 里还缺一个前向声明

我在核对你的 `.h` 时发现：`P04-4b` 文档里写了要加 `class UInputMappingContext;` 前向声明，但你当前的 `TenkaichiCharacterWithAbilities.h` 里**还没有这一行**。

`.h` 里 `TObjectPtr<UInputMappingContext> DefaultInputMappingContext;` 用了 `UInputMappingContext` 类型，但没声明它，会**编译报错**。请顺手补上：

在 `.h` 顶部前向声明区（`class UTenkaichiInputConfig;` 附近）加一行：

```cpp
class UInputMappingContext;
```

> 这是 P04-4b 的尾巴，这里一并提醒你补上，否则 `.cpp` 的 include 再全，`.h` 这边还是过不了编译。

---

## 八、这一步没教过的新方法（登记进 `已教方法.md`）

| 方法/类型 | 所属 | 作用 | 源码位置 |
|-----------|------|------|---------|
| `GetLocalPlayer()` | `APlayerController` | 拿控制器所属的本地玩家 | `PlayerController.h:1803` |
| `GetSubsystem<T>()` | `ULocalPlayer`（`USubsystem` 体系） | 从宿主身上取指定子系统 | `Subsystem.h` |
| `UEnhancedInputLocalPlayerSubsystem` | Enhanced Input | 本地玩家输入子系统，管理 IMC 注册/清除 | `EnhancedInputSubsystems.h:21` |
| `ClearAllMappings()` | `IEnhancedInputSubsystemInterface` | 清空所有已注册的映射上下文 | `EnhancedInputSubsystemInterface.h:247` |
| `AddMappingContext(...)` | `IEnhancedInputSubsystemInterface` | 注册 IMC 并指定优先级 | `EnhancedInputSubsystemInterface.h:256` |
| `FModifyContextOptions` | Enhanced Input | 注册/移除时的可选配置结构体 | `EnhancedInputSubsystemInterface.h:48` |

---

## 九、下一步

写完这段 `.cpp`（并补上 `.h` 的前向声明），P04 的**代码部分**就全部完成了。

但注意：目前 `DefaultInputMappingContext`、`InputConfig` 成员都是**空**的（还没在编辑器里指到资产），所以按键还不会真的动。等你在编辑器里：
1. 建 `InputAction`（`IA_Move` / `IA_Look`）资产；
2. 建 `InputMappingContext`（`IMC_Default`）资产，把按键映射进去；
3. 建 `InputConfig` 资产，把 Tag 映射到 InputAction；
4. 在角色蓝图上把这几个资产指到对应成员上。

——按键才会真正生效。

写完后跟我说「写完 / 下一步」，我再决定后续（继续 P05，或先补资产绑定的教程）。