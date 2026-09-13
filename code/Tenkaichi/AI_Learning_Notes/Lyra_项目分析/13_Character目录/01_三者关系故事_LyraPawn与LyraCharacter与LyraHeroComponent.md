# 三个人的故事：`ALyraPawn`、`ALyraCharacter`、`ULyraHeroComponent`

> 涉及文件：
> - `Source/LyraGame/Character/LyraPawn.h`
> - `Source/LyraGame/Character/LyraCharacter.h`
> - `Source/LyraGame/Character/LyraHeroComponent.h`（配合少量 `.cpp` 事实佐证）
>
> 本文用"讲故事 + 贴代码"的方式，讲清这三个东西**到底是什么关系**。
> 故事是壳，代码是证据 —— 每一段剧情后面都跟着真实的行号。

---

## 〇、先认识这几位（人物卡）

| 角色 | 是谁 | 一句话功能 |
|---|---|---|
| **`ALyraPawn`** | 最基础的"躯壳" | 只会走路 + **知道自己属于哪个队** |
| **`ALyraCharacter`** | 升级版"躯壳" | 有血、有相机、能接能力系统 —— 但没有"手"（不接收输入） |
| **`ULyraHeroComponent`** | 一个**组件**（不是 Actor） | 给躯壳装"手"（输入）和"眼睛"（相机） |
| **`ULyraPawnExtensionComponent`** | 隐藏主角 · 一个组件 | **统筹**：管数据、管初始化顺序、保管 ASC |
| **`ALyraPlayerState`** | 幕后的"老板" | **玩家真正的 ASC（能力系统）住在它身上**，不是住在角色身上 |

> 最后一行很反直觉，但它是理解这整套关系的关键：**一个玩家的"能力"跟着人走，不跟着身体走**（死了换身体，能力还在）。所以 `ALyraCharacter` 只是**临时借用** PlayerState 上的 ASC。

---

## 第一幕：先破除三个误会

### 误会一：「`ALyraCharacter` 是 `ALyraPawn` 的子类吧？」

**不是。** 看两行类声明，都摆在头文件里：

```19:20:Source/LyraGame/Character/LyraPawn.h
UCLASS(MinimalAPI)
class ALyraPawn : public AModularPawn, public ILyraTeamAgentInterface
```

```97:98:Source/LyraGame/Character/LyraCharacter.h
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ALyraCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ILyraTeamAgentInterface
```

把它们画成家谱：

```
                    APawn                        ACharacter
                      │                              │
              AModularPawn                   AModularCharacter
                      │                              │
                 ALyraPawn                      ALyraCharacter
                      ↖──────── 兄弟 ────────↗
```

**两个是兄弟，不是父子。**
（`AModularPawn` / `AModularCharacter` 来自 ModularGameplay 插件，它们各自继承引擎的 `APawn` / `ACharacter`。）

**唯一的"血缘证据"是他们实现了同一个接口**：

```
ALyraPawn      : ILyraTeamAgentInterface
ALyraCharacter : ILyraTeamAgentInterface   ← 两边都有
```

所以两个头文件里能看到**几乎一模一样的队伍代码**——比如都有 `MyTeamID` + 队伍变化委托 + 一样注释的"失去控制后队伍怎么算"：

```56:65:Source/LyraGame/Character/LyraPawn.h
private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

private:
	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);
```

```206:210:Source/LyraGame/Character/LyraCharacter.h
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;
```

连那个"策略点"函数都是一字不差地写了两份：

```44:50:Source/LyraGame/Character/LyraPawn.h
protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}
```

> **记住**：这是"**实现同一个接口导致的代码相似**"，不是继承。看到相似代码先去看类声明，别急着画继承箭头。

---

### 误会二：「`ULyraHeroComponent` 应该是 `ALyraCharacter` 的成员吧？」

**也不是。** 翻遍 `LyraCharacter.h` 的成员区，里面只有三个组件：

```192:201:Source/LyraGame/Character/LyraCharacter.h
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;
```

**`ULyraHeroComponent` 不在这张名单里。**

那它从哪来？看 `ALyraCharacter` 的构造函数（`.cpp` 里造零件的地方）：

```64:73:Source/LyraGame/Character/LyraCharacter.cpp
	PawnExtComponent = CreateDefaultSubobject<ULyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<ULyraHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<ULyraCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
```

只造了三个。**HeroComponent 是在蓝图子类里加上去的**（比如 Lyra 的 `B_Hero_ShooterMannequin` 这类角色蓝图，在组件列表里手动 Add 一个 `Lyra Hero Component`）。

> 为什么这么设计？因为 `ULyraHeroComponent` 头上有个标记：
> ```31:31:Source/LyraGame/Character/LyraHeroComponent.h
> UCLASS(MinimalAPI, Blueprintable, Meta=(BlueprintSpawnableComponent))
> ```
> `BlueprintSpawnableComponent` = **"我能在蓝图里被 Add 出来"**。
> 于是 C++ 只负责"身体的基础器官"，"要不要装操控器"交给具体角色蓝图决定 —— **同一个 Character 类，玩家版本装 Hero，NPC 版本不装。**

---

### 误会三：「那一定是 `Character` 主动去调用 `HeroComponent` 吧？」

**只对了一半。** 事实是**互相调用，但方向不同**：

| 方向 | 例子 | 含义 |
|---|---|---|
| `Character` → `HeroComponent` | `SetupPlayerInputComponent` 通知一声 | **"有输入组件了，你看着办"** |
| `HeroComponent` → `Character` | `Input_Crouch` 里调 `Character->ToggleCrouch()` | **"玩家按了蹲，你去蹲"** |
| 谁也不直接找谁（都找**统筹**） | `FindPawnExtensionComponent(Pawn)` | **"我先问统筹要数据/ASC"** |

第三条最关键：**它们大部分时候不是"直接对话"，而是通过 `ULyraPawnExtensionComponent` 中转**。这就是"室友共用一台冰箱"的关系 —— 谁都去冰箱拿东西，但没人从别人手里接东西。

---

## 第二幕：他们其实是"室友"

把这三个东西放到同一个 `Actor` 上，画面是这样：

```
┌───────────────────── ALyraCharacter（一个 Actor，就是"身体"） ─────────────────────┐
│                                                                                    │
│   [PawnExtComponent]        [HealthComponent]        [CameraComponent]             │
│    统筹 / 数据 / ASC 挂载      血量 / 死亡             相机（执行层）                 │
│         ▲                                                      ▲                   │
│         │ 问它要 PawnData / ASC                                 │ 问它"该用哪个镜头"   │
│         │                                                      │                   │
│   [HeroComponent] ──────────────────────────────────────────────┘                  │
│    输入 / 相机（决策层）      ← 蓝图里 Add 上来的                                      │
│                                                                                    │
│   MovementComponent（移动） · Mesh（模型） · Capsule（碰撞）      ← 引擎自带           │
└────────────────────────────────────────────────────────────────────────────────────┘
```

**三条规矩**（记住这三条，关系就通了）：

1. **身体（`ALyraCharacter`）不干活**，只负责"在正确时机通知大家"——它自己的注释就是这么写的：
   > "Responsible for sending events to pawn components. New behavior should be added via pawn components when possible."
2. **组件之间不直接持有对方**，靠 `FindXxxComponent(Actor)` 找 + 靠统筹（PawnExtension）拿共享数据。
3. **决策和执行分开**：HeroComponent 说"该用这个相机"（决策），CameraComponent 真的去切（执行）。

---

## 第三幕：开机 —— 三个人各自报到

游戏开始，`BeginPlay` 一响，三方各自做自己的准备。

**HeroComponent 的第一件事：蹲在统筹门口守消息。**

```206:216:Source/LyraGame/Character/LyraHeroComponent.cpp
void ULyraHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(ULyraPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(LyraGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}
```

这两行是**整段关系的核心动作**：

| 代码 | 翻译 |
|---|---|
| `BindOnActorInitStateChanged(ULyraPawnExtensionComponent::NAME_ActorFeatureName, ...)` | **"统筹一旦有什么动静，立刻告诉我"** |
| `TryToChangeInitState(InitState_Spawned)` | **"我先报个到：我生出来了"** |
| `CheckDefaultInitialization()` | **"看看我现在能不能往下走"** |

**Character 这边更简单**：它把"控制器变了""PlayerState 复制好了""输入组件来了"这些事，转手通知统筹——

```263:268:Source/LyraGame/Character/LyraCharacter.cpp
void ALyraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}
```

```212:218:Source/LyraGame/Character/LyraCharacter.cpp
void ALyraCharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();
	...
```

> 看到了吗？**Character 从头到尾没提过 HeroComponent 一个字。**
> 它只负责"把外界变化告诉统筹"，至于"谁关心这个变化"，它不管 —— 这就是**"壳 + 接线员"**的真实含义。

---

## 第四幕：HeroComponent 的四道关卡

HeroComponent 想"接管操控权"，不是想做就能做的。它得经过状态机的四道关卡，代码就在 `CanChangeInitState` 里（这就是那个"等统筹"的机制）：

```90:140:Source/LyraGame/Character/LyraHeroComponent.cpp
	else if (CurrentState == LyraGameplayTags::InitState_Spawned && DesiredState == LyraGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<ALyraPlayerState>())
		{
			return false;
		}
		...
		if (bIsLocallyControlled && !bIsBot)
		{
			ALyraPlayerController* LyraPC = GetController<ALyraPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !LyraPC || !LyraPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == LyraGameplayTags::InitState_DataAvailable && DesiredState == LyraGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		ALyraPlayerState* LyraPS = GetPlayerState<ALyraPlayerState>();

		return LyraPS && Manager->HasFeatureReachedInitState(Pawn, ULyraPawnExtensionComponent::NAME_ActorFeatureName, LyraGameplayTags::InitState_DataInitialized);
	}
```

用"过安检"的方式翻译这四道关卡：

| 关卡 | 从哪 → 到哪 | 条件（人话） |
|---|---|---|
| 1 | 空 → `Spawned` | 只要挂在真的 Pawn 上就放行 |
| 2 | `Spawned` → `DataAvailable` | **必须有 `ALyraPlayerState`**（不然 ASC 无处可借）；本地玩家还额外要求 `InputComponent` + `ALyraPlayerController` + `LocalPlayer` **三件套齐了** |
| 3 | `DataAvailable` → `DataInitialized` | **必须等统筹（PawnExtensionComponent）也走到 `DataInitialized`** ← ★这一行就是"关系"最直接的代码证据 |
| 4 | `DataInitialized` → `GameplayReady` | 直接放行（代码里还留了句 `// TODO add ability initialization checks?`） |

关卡 3 的那一行：

```134:134:Source/LyraGame/Character/LyraHeroComponent.cpp
		return LyraPS && Manager->HasFeatureReachedInitState(Pawn, ULyraPawnExtensionComponent::NAME_ActorFeatureName, LyraGameplayTags::InitState_DataInitialized);
```

**翻译**："我（Hero）要往前走，得先确认**统筹（PawnExtensionComponent）已经到了 `DataInitialized`**。"
这就是为什么本文反复说：**HeroComponent 的初始化权不在自己手里。**

那"统筹动了"这个消息怎么传过来？就在 `OnActorInitStateChanged`：

```186:196:Source/LyraGame/Character/LyraHeroComponent.cpp
void ULyraHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == ULyraPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == LyraGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}
```

**"统筹到 `DataInitialized` 了 → 我再试一次能不能往下走。"** —— 闭环成立。

---

## 第五幕：过关那一刻（全场最关键的一段代码）

状态从 `DataAvailable` 走到 `DataInitialized` 时，HeroComponent 的 `HandleChangeInitState` 会干**三件大事**：

```147:183:Source/LyraGame/Character/LyraHeroComponent.cpp
	if (CurrentState == LyraGameplayTags::InitState_DataAvailable && DesiredState == LyraGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ALyraPlayerState* LyraPS = GetPlayerState<ALyraPlayerState>();
		if (!ensure(Pawn && LyraPS))
		{
			return;
		}

		const ULyraPawnData* PawnData = nullptr;

		if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(LyraPS->GetLyraAbilitySystemComponent(), LyraPS);
		}

		if (ALyraPlayerController* LyraPC = GetController<ALyraPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// Hook up the delegate for all pawns, in case we spectate later
		if (PawnData)
		{
			if (ULyraCameraComponent* CameraComponent = ULyraCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}
	}
```

逐条翻译：

| 动作 | 代码 | 说明 |
|---|---|---|
| **① 拿数据** | `PawnData = PawnExtComp->GetPawnData<ULyraPawnData>()` | 从**统筹**手里拿 `PawnData`（里面有 `InputConfig` 和 `DefaultCameraMode`） |
| **② 挂 ASC** | `PawnExtComp->InitializeAbilitySystem(LyraPS->GetLyraAbilitySystemComponent(), LyraPS)` | **注意：这一步是 HeroComponent 触发的！** 把 PlayerState 上的 ASC 绑到这个 Pawn 身上，Pawn 从此成为 ASC 的"化身（Avatar）"。注释写得极清楚："**ASC 和属性集住在 PlayerState 上**" |
| **③ 绑输入** | `InitializePlayerInput(Pawn->InputComponent)` | 前提是**有 `ALyraPlayerController`**（真玩家才有） |
| **④ 交相机** | `CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode)` | **"决策层"把自己交给"执行层"** —— 以后 CameraComponent 想知道"该用哪个镜头"，就调 HeroComponent 的 `DetermineCameraMode()` |

**第 ④ 条是理解"相机三层分工"的钥匙**：

```
技能 → HeroComponent::SetAbilityCameraMode()   （改"决策"）
              ↓
        HeroComponent::DetermineCameraMode()   （做"决策"）
              ↓ 通过委托 DetermineCameraModeDelegate
        ULyraCameraComponent                   （做"执行"：真正切镜头、混合过渡）
```

**这也解释了本文第一幕的第三条规矩：决策与执行分开。**

另外注意 ② 的连锁反应：`InitializeAbilitySystem` 会触发 `PawnExtensionComponent` 的"ASC 就绪"广播，而 `ALyraCharacter` 早就在构造函数里订了这个消息：

```64:66:Source/LyraGame/Character/LyraCharacter.cpp
	PawnExtComponent = CreateDefaultSubobject<ULyraPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
```

于是 Character 的 `OnAbilitySystemInitialized` 被调用，**顺手把血量组件也初始化了**：

```197:205:Source/LyraGame/Character/LyraCharacter.cpp
void ALyraCharacter::OnAbilitySystemInitialized()
{
	ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
	check(LyraASC);

	HealthComponent->InitializeWithAbilitySystem(LyraASC);

	InitializeGameplayTags();
}
```

**完整的一条链终于串起来了**：

```
HeroComponent 过第 3 关
   → 调 PawnExtComp->InitializeAbilitySystem(PlayerState 的 ASC, PlayerState)
      → PawnExtensionComponent 广播"ASC 就绪"
         → ALyraCharacter::OnAbilitySystemInitialized()
            → HealthComponent->InitializeWithAbilitySystem(LyraASC)   ← 血量开始能用
   → InitializePlayerInput(...)                                       ← 输入绑好了
   → CameraComponent->DetermineCameraModeDelegate.BindUObject(this,...) ← 相机有"决策者"了
```

> **一句话**：`HeroComponent` 是那个**"按下开关"的人**，一按下去，血量、输入、相机三件事同时接通。
> 所以它在整个 Character 目录里的地位，比看上去重要得多。

---

## 第六幕：按下一个键，会发生什么

输入绑好之后（`InitializePlayerInput` 里通过 `ULyraInputComponent::BindNativeAction` 把 Tag 和函数对上），玩家的操作会走三条不同的路：

### 路线 1：能力键 → 给 ASC

```343:355:Source/LyraGame/Character/LyraHeroComponent.cpp
void ULyraHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (ULyraAbilitySystemComponent* LyraASC = PawnExtComp->GetLyraAbilitySystemComponent())
			{
				LyraASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}
```

**"按键" → "找统筹要 ASC" → "把 Tag 交给它"。**
HeroComponent 根本不关心这个 Key 是"开火"还是"开镜"——它只认 **Tag**，具体哪个 Tag 对应哪个技能，由能力系统那边的 `InputConfig` / `AbilitySet` 决定。

### 路线 2：蹲 → 交给 Character（顺着讲清 `OnStartCrouch`）

```451:457:Source/LyraGame/Character/LyraHeroComponent.cpp
void ULyraHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (ALyraCharacter* Character = GetPawn<ALyraCharacter>())
	{
		Character->ToggleCrouch();
	}
}
```

**英雄组件不做"蹲"这件事，它只会喊一声"你要不要蹲一下"。** 真正判断和执行在 Character：

```426:438:Source/LyraGame/Character/LyraCharacter.cpp
void ALyraCharacter::ToggleCrouch()
{
	const ULyraCharacterMovementComponent* LyraMoveComp = CastChecked<ULyraCharacterMovementComponent>(GetCharacterMovement());

	if (IsCrouched() || LyraMoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (LyraMoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}
```

然后注意一个细节：**必须站在地上才能蹲**（`IsMovingOnGround()`）。`Crouch()` 是引擎 `ACharacter` 的函数，引擎内部做完胶囊高度的调整后，会回调 `OnStartCrouch` —— 而 Lyra 在这个回调里做了一件很"Lyra 味"的事：

```440:459:Source/LyraGame/Character/LyraCharacter.cpp
void ALyraCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(LyraGameplayTags::Status_Crouching, 1);
	}


	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void ALyraCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(LyraGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}
```

**它把"我正在蹲"这件事，翻译成了一个 GameplayTag：`Status.Crouching`。**

| 代码 | 说明 |
|---|---|
| `SetLooseGameplayTagCount(Tag, 1)` | 给 ASC 打上这个 Tag（`1` = 层数） |
| `SetLooseGameplayTagCount(Tag, 0)` | 摘掉（层数设 0） |
| `Super::OnStartCrouch(...)` | **别忘调父类** —— 引擎原本要做的"调整模型/相机偏移"还得让它做 |

**为什么要翻译成 Tag？** 因为这样**全项目就能用统一的方式问"我现在在蹲吗"**：

- 能力：某个技能可以配"必须处于 `Status.Crouching` 才能放"；
- UI：`HiddenByTags` 里写这个 Tag，蹲下时某块 UI 自动隐藏；
- 动画/表现：蓝图里查 Tag 切动画。

> **这就是 `LyraPawn.h` 里那个"移动模式 Tag"的同类思想**（`LyraCharacter::SetMovementModeTag` 也是把"在飞/在跑"翻译成 Tag）：
> **消 `Loose` Tag 是 Lyra 里"把 C++ 状态暴露给能力系统"的标准手法。**

顺便回顾 `OnStartCrouch` 的参数（引擎语义）：

| 参数 | 含义 |
|---|---|
| `HalfHeightAdjust` | **站姿半高 − 蹲姿半高**（胶囊从 88 变成 65 时，这个值是"半高缩了多少"） |
| `ScaledHalfHeightAdjust` | 上面那个值**乘上 Actor 缩放**后的结果 —— 因为模型/相机的偏移要按缩放比例调整，不是按原始值 |

> 所以这个回调的时机是：**引擎已经把胶囊高度改完了，现在轮到"依赖身高的东西"（模型位置、相机高度、自定义逻辑）自己调整。** Lyra 就在这儿顺手打 Tag。

### 路线 3：移动 / 视角 / 自动跑 → 交给控制器

| 输入 | 去哪 |
|---|---|
| `Input_Move` | 用 `Controller` 的偏航角把二维输入转成世界方向；顺带 `SetIsAutoRunning(false)`（**一推摇杆就取消自动跑**） |
| `Input_LookMouse` | `Pawn->AddControllerYawInput/PitchInput`（鼠标是"位移量"） |
| `Input_LookStick` | 同上，但**乘 `DeltaSeconds`**（摇杆是"变化速率"，要和帧时间相乘才对） |
| `Input_AutoRun` | `ALyraPlayerController::SetIsAutoRunning(...)` 取反 |

> **规律**：HeroComponent 里所有 `Input_XXX` 都是**薄薄一层转发**，没有任何玩法逻辑。这就是"**组件只管把输入送到正确的人手里**"。

---

## 第七幕：`ALyraPawn` 在这场戏里站哪

前面都在讲 Character，那 `ALyraPawn` 呢？它其实是"**简配版的躯壳**"：

| 能力 | `ALyraPawn` | `ALyraCharacter` |
|---|---|---|
| 队伍 ID | ✅ 有 | ✅ 有 |
| ASC（能力系统） | ❌ 没有 | ✅ 通过 `PawnExtComponent` 借 PlayerState 的 |
| 血量 / 死亡流程 | ❌ 没有 | ✅ 有 `HealthComponent` |
| 相机 | ❌ 没有 | ✅ 有 `CameraComponent` |
| 类声明里的接口 | 只有 `ILyraTeamAgentInterface` | **四个接口**（ASC / Cue / TagAsset / Team） |
| 典型用途 | "能被人操控的非角色"（载具、炮台、观战化身之类的通用底子） | 玩家角色 |

**关键结论：`ULyraHeroComponent` 并不"绑定" `ALyraCharacter`。**

看它的关卡条件就知道——它只要三样东西：

| 需求 | 代码里的判断 | `ALyraPawn` 能满足吗 |
|---|---|---|
| 是个真 Pawn | `GetPawn<APawn>()` | ✅ |
| 有 `ALyraPlayerState` | `GetPlayerState<ALyraPlayerState>()` | ✅（Pawn 也能被 Controller 占用并拿到 PlayerState） |
| 有 `PawnExtensionComponent`（拿数据/ASC） | `FindPawnExtensionComponent(Pawn)` | ⚠️ 得自己在蓝图/C++ 里加 |

但注意 `Input_Crouch` 里有这么一句：

```453:453:Source/LyraGame/Character/LyraHeroComponent.cpp
	if (ALyraCharacter* Character = GetPawn<ALyraCharacter>())
```

**它明确 `Cast` 到 `ALyraCharacter`** —— 也就是说：**HeroComponent 在"通用输入"这部分对 `ALyraPawn` 一视同仁，但在"蹲"这种具体玩法上，只认 `ALyraCharacter`。**

> 这就是 Lyra 的常见手法：**核心逻辑保持通用，具体玩法上用 `Cast` 做"有条件支持"**（Cast 失败就静默不做，不会崩）。

**那 `ALyraPawn` 和 `ALyraCharacter` 的关系总结成一句话**：

> **同一套"模块化 Pawn 思路"下的两个成品：一个是空壳子（队伍 + 被控制），一个是满配房（壳子 + 血 + 相机 + 借来的能力）。HeroComponent 是"可选装的外设"，谁装谁就能被玩家操控。**

---

## 结局：一张总图

```
                        ┌──────────────────────────┐
                        │      ALyraPlayerState     │
                        │   ASC + 属性集（真身在这）  │
                        └────────────┬─────────────┘
                                     │ 借 ASC
                                     ▼
┌────────────────────────── ALyraCharacter（身体 / 壳）──────────────────────────┐
│                                                                              │
│   ALyraPawn 那一套：ILyraTeamAgentInterface（队伍 ID + OnRep_MyTeamID）         │
│   也是它的一部分（同一个接口，各写一份）                                          │
│                                                                              │
│  ┌─ PawnExtComponent ──────┐  ← 统筹：PawnData / ASC / 初始化顺序               │
│  │  （所有 Pawn 都用）       │                                                │
│  └──────────┬──────────────┘                                                 │
│             │ ①拿 PawnData ②挂 ASC                                            │
│  ┌──────────▼──────────────┐        ┌─ HealthComponent ─┐                     │
│  │    HeroComponent        │        │ 血量 / 死亡（只在  │                     │
│  │  输入 + 相机（决策层）    │        │ ASC 就绪后才可用） │                     │
│  │  ← 蓝图里 Add 的外设     │        └─────────▲────────┘                     │
│  └───┬──────────────┬──────┘                  │                              │
│      │ 转发输入      │ DetermineCameraMode 委托│ 初始化                       │
│      ▼              ▼                        │                              │
│  能力→ASC       CameraComponent（执行层）───────┘ 也会回调 Character 的死亡流程 │
│  Input_Crouch→ ALyraCharacter::ToggleCrouch()                                │
│                   ↓                                                          │
│            引擎 Crouch() → OnStartCrouch() → 打 Tag: Status.Crouching          │
└──────────────────────────────────────────────────────────────────────────────┘
```

**三条规矩（最终版）**：

1. **身体不干活**：`ALyraCharacter` 只转发事件；功能都在组件里。
2. **组件之间靠统筹（`PawnExtensionComponent`）共享数据**，不互相持有。
3. **决策与执行分离**：HeroComponent 出"该用什么相机/输入该给谁"，CameraComponent / ASC / Character 去执行。

---

## 附录 A：一句话记住三个东西

| 名字 | 类比 | 记住这一句 |
|---|---|---|
| `ALyraPawn` | 空白躯壳 | **只管"我是谁、我属于哪队"** |
| `ALyraCharacter` | 满配躯壳 | **有血有相机、但自己不会动，靠组件** |
| `ULyraHeroComponent` | 外设（手柄 + 眼睛） | **玩家操控权的入口：输入 + 相机** |

---

## 附录 B：排错/看代码时的对照表

| 你想找 | 去哪找 | 关键函数 |
|---|---|---|
| 输入到底绑在哪 | `LyraHeroComponent.cpp` | `InitializePlayerInput`（结尾还有 `SendGameFrameworkComponentExtensionEvent(..., NAME_BindInputsNow)` 广播） |
| 谁决定初始化顺序 | `LyraHeroComponent.cpp` + `LyraPawnExtensionComponent.cpp` | `CanChangeInitState` / `HandleChangeInitState` / `CheckDefaultInitialization` |
| ASC 到底在哪一步挂上 | `LyraHeroComponent.cpp` | `HandleChangeInitState` 里的 `InitializeAbilitySystem(...)` ← **不是 Character 干的** |
| 血量什么时候能用 | `LyraCharacter.cpp` | `OnAbilitySystemInitialized`（由 PawnExtension 广播触发） |
| 玩家按键怎么变成能力 | `LyraHeroComponent.cpp` | `Input_AbilityInputTagPressed` → `ASC->AbilityInputTagPressed(Tag)` |
| 蹲下为什么会带 Tag | `LyraCharacter.cpp` | `ToggleCrouch` → 引擎 `Crouch()` → `OnStartCrouch` → `Status_Crouching` |
| 相机为什么能被技能改 | `LyraHeroComponent.cpp` | `SetAbilityCameraMode` / `ClearAbilityCameraMode` / `DetermineCameraMode` |
| 相机"执行者"是谁 | `Camera/` 目录 | `ULyraCameraComponent::DetermineCameraModeDelegate` |
| 队伍 ID 怎么同步 | `LyraPawn.h` / `LyraCharacter.h` | `MyTeamID` + `OnRep_MyTeamID` + `DetermineNewTeamAfterPossessionEnds` |

---

## 附录 C：三个"反直觉点"复习

1. **`ALyraCharacter` 不是 `ALyraPawn` 的子类**，只是"兄弟 + 都实现了队伍接口"。
2. **`ULyraHeroComponent` 不在 `ALyraCharacter` 的 C++ 成员里**，是蓝图里 Add 的外设（因为标了 `BlueprintSpawnableComponent`）。
3. **ASC 的初始化是 `HeroComponent` 触发的**，不是 Character 自己 —— 谁先走到那一步，谁就按下开关。
