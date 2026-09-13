# GameModes 目录总览：都是干嘛的

> 结论先说：**这个目录真正的主题不是"GameMode"，而是 `Experience`（体验）。**
>
> 传统 UE：**地图 + GameMode** = 一局游戏长什么样
> Lyra：**地图 + Experience** = 一局游戏长什么样
>
> 而这里的所有文件，都在回答两个问题：
> **① Experience 是什么、怎么配**（数据层）
> **② 一局开始时，怎么把它一步步加载出来**（流程层）

📌 阅读顺序：**本篇（看地图）→ 单文件笔记**

---

## 一、先搞懂 Experience 是什么

先建立画面：

```
传统 UE 的做法                     Lyra 的做法
──────────────────────            ──────────────────────
地图 → World Settings 指定 GM      地图 → 指定默认 Experience
GameMode 里写死所有玩法规则         Experience 里"组装"玩法
想要新模式 = 新建 GameMode 子类    想要新模式 = 新建一个 Experience 资产
玩法代码全在主工程                 玩法可以是插件（GameFeature）
```

**一个 Experience 资产里装四样东西：**

| 字段 | 含义 |
|---|---|
| `GameFeaturesToEnable` | 这局要**激活哪些插件**（比如 ShooterCore） |
| `DefaultPawnData` | 默认给玩家发**什么角色**（血量、技能、移动参数打包） |
| `Actions` | 加载这局时要**执行哪些动作**（就是 GameFeatures 目录里那套） |
| `ActionSets` | 可复用的**动作包**（多个 Experience 共享一批动作） |

> ⚠️ 注意最后两行：**`Actions` 里装的就是 `UGameFeatureAction`** —— 和上一个目录（GameFeatures）里写的那些"中括号"是**同一批类**。
>
> 区别在于挂载点：
> - 挂在**插件**里 → 插件激活时执行
> - 挂在 **Experience** 里 → 这局游戏开始时执行
>
> **两条路，同一个机制。**

打比方：
> **Experience = 一份"这局游戏的完整配置单"**，
> 相当于餐厅点的一份套餐：主菜（地图）+ 配菜（插件）+ 餐具（UI/输入）+ 口味（PawnData）。

---

## 二、目录长相：20 个文件，分四块

```
GameModes/
├── 【数据层】 描述"一局游戏是什么"
│   ├── LyraExperienceDefinition           ← 配置单本体（给引擎看）
│   ├── LyraExperienceActionSet            ← 可复用的动作包
│   └── LyraUserFacingExperienceDefinition ← 大厅里那张卡片（给玩家看）
│
├── 【流程层】 把配置单加载出来
│   ├── LyraExperienceManagerComponent     ← ★ 核心状态机，挂在 GameState 上
│   ├── LyraExperienceManager              ← 编辑器 PIE 专用的引用计数
│   └── AsyncAction_ExperienceReady        ← 给蓝图用的"等待就绪"节点
│
├── 【传统三大件（被改造）】
│   ├── LyraGameMode                       ← 改成"先定 Experience，再定角色"
│   ├── LyraGameState                      ← 多挂了 Experience 组件 + 全局 ASC
│   └── LyraWorldSettings                  ← 地图级的"默认体验"设置
│
└── 【小工具】
    └── LyraBotCreationComponent           ← 开局自动生成 AI 机器人
```

| 分类 | 文件数 | 一句话 |
|---|---|---|
| 数据层 | 3 组 | "一局游戏"的三种视角：引擎视角、复用视角、玩家视角 |
| 流程层 | 3 组 | 状态机 + PIE 仲裁 + 蓝图等待节点 |
| 传统三大件 | 3 组 | GameMode/GameState 被改造成"Experience 的搬运工" |
| 小工具 | 1 组 | 补机器人 |

---

## 三、数据层：三张"配置单"

### ① `LyraExperienceDefinition` —— 配置单本体

```35:51:Source/LyraGame/GameModes/LyraExperienceDefinition.h
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** The default pawn class to spawn for players */
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const ULyraPawnData> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;
```

三个细节值得留意：
- `Instanced`：`Actions` 是**内联实例**，直接在这个资产里配，不用单独建资产
- `Const`：整份配置**只读**，加载过程不会改它
- 也是 `UPrimaryDataAsset`：所以能被 AssetManager 按 ID 查找、被纳入主资产的加载体系

### ② `LyraExperienceActionSet` —— 可复用的动作包

就是"把一批 Actions 打包，便于多个 Experience 共用"。

举个场景：
> 有 5 个射击模式，**每个都要同一套 HUD、同一套输入**。
> 不用复制 5 遍 → 建一个 ActionSet，5 个 Experience 都引用它，**改一处全生效**。

它自己也能带 `GameFeaturesToEnable`，所以还能表达**插件依赖**。

### ③ `LyraUserFacingExperienceDefinition` —— 大厅里那张卡片

**这张才是玩家真正看到的东西**：

| 字段 | 作用 |
|---|---|
| `MapID` + `ExperienceID` | 进哪张图 + 玩什么玩法 |
| `TileTitle` / `TileSubTitle` / `TileDescription` / `TileIcon` | UI 卡片的文案和图片 |
| `LoadingScreenWidget` | 进/出这局时用哪个加载界面 |
| `MaxPlayerCount` | 这局最多几个人 |
| `bIsDefaultExperience` / `bShowInFrontEnd` | 是否快玩默认 / 是否在大厅列表显示 |
| `bRecordReplay` | 是否录制回放 |
| `ExtraArgs` | 附加的 URL 参数 |
| `CreateHostingRequest()` | **真正用来开房**：生成一个开房请求对象 |

> **两者关系一句话记：**
> `UserFacingExperienceDefinition` = **菜单上的菜名和图片**（玩家视角）
> `ExperienceDefinition` = **后厨的配方**（引擎视角）
> 玩家点菜 → 系统翻译成配方 → 去加载。

---

## 四、流程层：★ `LyraExperienceManagerComponent`

**这是整个目录最重要的文件**，一个挂在 `GameState` 上的组件。

### 一个六步状态机

```18:27:Source/LyraGame/GameModes/LyraExperienceManagerComponent.h
enum class ELyraExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};
```

| 状态 | 在干什么 |
|---|---|
| `Unloaded` | 还没决定玩什么 |
| `Loading` | 加载 Experience 资产本身（+ ActionSet） |
| `LoadingGameFeatures` | 加载并**激活**它要求的所有插件 |
| `LoadingChaosTestingDelay` | **故意的随机延迟**，用来测试"加载太久"的表现 |
| `ExecutingActions` | 执行 Experience 里配的 Actions |
| `Loaded` | 完事，开始通知各方 |
| `Deactivating` | 退出时反向卸载 |

### 关键流程一步步看

**Step 1：谁来指定 Experience？**
服务器由 GameMode 调 `SetCurrentExperience()`；客户端则靠**属性复制**：

```82:83:Source/LyraGame/GameModes/LyraExperienceManagerComponent.h
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;
```

客户端收到复制 → 触发 `OnRep_CurrentExperience` → 自己也开始加载。
**服务器和客户端各自加载，但目标一致。**

**Step 2：加载资产（区分客户端/服务器数据）**

```154:164:Source/LyraGame/GameModes/LyraExperienceManagerComponent.cpp
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);
	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}
```

服务器不加载纯客户端资产，客户端不加载纯服务器资产。**这和上一个目录里 `LyraGameFeaturePolicy` 干的是同一件事，两个层面各做一遍。**

**Step 3：把插件名字变成 URL 并激活**

```262:271:Source/LyraGame/GameModes/LyraExperienceManagerComponent.cpp
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = ELyraExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			ULyraExperienceManager::NotifyOfPluginActivation(PluginURL);
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
```

注意这是**并行加载 + 计数器**（`NumGameFeaturePluginsLoading--`，减到 0 才算完），不是排队一个个来。

**Step 4：执行 Actions —— 和 GameFeatures 目录接上了**

```320:334:Source/LyraGame/GameModes/LyraExperienceManagerComponent.cpp
	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};
```

**这里就是 Experience 和 GameFeatureAction 的交汇点**：
一个 Action 无论挂在插件上还是挂在 Experience 上，**生命周期回调都是这三个**（注册 → 加载 → 激活），卸载时是另外两个（停用 → 反注册）。

> 也顺便解释了上个目录里 `AddGameplayCuePath` 为什么要在"注册"阶段处理 —— 因为注册最早。

**Step 5：三级广播，顺序有讲究**

```347:354:Source/LyraGame/GameModes/LyraExperienceManagerComponent.cpp
	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();
```

**高 → 中 → 低**三级，用来解决"谁先初始化"的顺序问题。
配套的注册函数是 `CallOrRegister_OnExperienceLoaded_*()` —— 名字里的 `CallOr` 意思是：
**已经加载完了就立刻回调，没加载完就排队等。** 这个模式非常实用，避免了"时序不对就拿不到数据"。

**Step 6：加载界面靠谁撑着？**
它实现了 `ILoadingProcessInterface`：

```447:452:Source/LyraGame/GameModes/LyraExperienceManagerComponent.cpp
bool ULyraExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState != ELyraExperienceLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
```

**只要没加载完就一直转圈**，而且能把原因（`OutReason`）显示出来。加载界面系统统一问所有实现了这个接口的对象，有一个说"要"就继续显示。

### 另外两个流程层文件

**`LyraExperienceManager`（EngineSubsystem）** —— 只在编辑器里有意义：

```31:49:Source/LyraGame/GameModes/LyraExperienceManager.cpp
bool ULyraExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		...
		// Only let the last requester to get this far deactivate the plugin
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			...
			return true;
		}

		return false;
	}
```

**问题**：PIE 里可能开着多个窗口（客户端 + 服务器），两个窗口都要激活同一个插件。
如果窗口 A 退出就把插件卸了，窗口 B 就炸了。

**解法**：引用计数 —— 谁激活谁 +1，谁退出谁 -1，**只有减到 0 才真正卸载**。
（是不是很眼熟？和上个目录 `SplitscreenConfig` 的投票机制是同一个思路。）
注意非编辑器下这两个函数是空实现 —— 因为真机上不需要这种仲裁。

**`AsyncAction_ExperienceReady`** —— 给蓝图用的等待节点。
四个 step 的名字已经把逻辑说完了：

```
Step1_HandleGameStateSet        → 等 GameState 出现
Step2_ListenToExperienceLoading → 挂上加载完成的回调
Step3_HandleExperienceLoaded    → 等到了
Step4_BroadcastReady            → 广播 OnReady
```

蓝图里一连上这个节点，就能"等这局准备好再做某事"，不用自己处理一堆时序判断。

---

## 五、传统三大件：被改造成"Experience 的搬运工"

### ① `LyraGameMode`

**最核心的变化：不再"地图直接决定角色"，而是"Experience 决定角色"。**

构造函数先把一整套类都换掉：

```33:43:Source/LyraGame/GameModes/LyraGameMode.cpp
	GameStateClass = ALyraGameState::StaticClass();
	GameSessionClass = ALyraGameSession::StaticClass();
	PlayerControllerClass = ALyraPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = ALyraReplayPlayerController::StaticClass();
	PlayerStateClass = ALyraPlayerState::StaticClass();
	DefaultPawnClass = ALyraCharacter::StaticClass();
	HUDClass = ALyraHUD::StaticClass();
```

**"要找哪个 Experience"的优先级**（从上到下第一个有效的赢）：

| 优先级 | 来源 |
|---|---|
| 1 | URL 参数 `?Experience=xxx` |
| 2 | 开发者设置（仅 PIE 有效，方便调试） |
| 3 | 命令行 `-Experience=xxx` |
| 4 | 地图的 World Settings |
| 5 | 专用服务器登录流程 |
| 6 | 兜底：`B_LyraDefaultExperience` |

找到后交给状态机：

```295:297:Source/LyraGame/GameModes/LyraGameMode.cpp
		ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
```

**最体现设计意图的一处 —— 玩家先别生成：**

```391:399:Source/LyraGame/GameModes/LyraGameMode.cpp
void ALyraGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay starting new players until the experience has been loaded
	// (players who log in prior to that will be started by OnExperienceLoaded)
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}
```

**进来得早的玩家，先挂起不生成角色**；等 Experience 加载完，`OnExperienceLoaded` 里统一补上：

```305:321:Source/LyraGame/GameModes/LyraGameMode.cpp
void ALyraGameMode::OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience)
{
	// Spawn any players that are already attached
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}
```

**为什么必须这样？** 因为 `PawnData`（角色用什么血、什么技能）来自 Experience。
**Experience 还没加载，就不知道该怎么造这个角色。**

**"这个玩家该用什么角色"的三级取法：**

| 顺序 | 来源 |
|---|---|
| 1 | PlayerState 上已经有的 PawnData（比如复活时保留） |
| 2 | 当前 Experience 的 `DefaultPawnData` |
| 3 | AssetManager 里的全局默认 |

拿到后，在生成 Pawn 的瞬间塞给它：

```356:360:Source/LyraGame/GameModes/LyraGameMode.cpp
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				if (const ULyraPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
```

> 这里是和 `13_Character` 目录的接口：**GameMode 负责"选角色配方"，PawnExtensionComponent 负责"按配方初始化"。**

另外，GameMode 里很多函数（`ChoosePlayerStart`、`PlayerCanRestart`、`FinishRestartPlayer`）都**不做实事**，而是转发给 GameState 上的组件：

```401:408:Source/LyraGame/GameModes/LyraGameMode.cpp
AActor* ALyraGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (ULyraPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<ULyraPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ChoosePlayerStart(Player);
	}
```

**这是 Lyra 的通用套路**：GameMode 只做骨架，具体策略放在可替换的组件里。

### ② `LyraGameState`

它比传统 GameState 多了两样东西：

**1. Experience 加载组件**（就是上面那个状态机）—— 因为**Experience 是"这一局的状态"，属于 GameState 而不是 GameMode**（GameMode 只在服务器存在）。

**2. 自己的 ASC**：

```28:32:Source/LyraGame/GameModes/LyraGameState.cpp
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<ULyraAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ExperienceManagerComponent = CreateDefaultSubobject<ULyraExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
```

注释写得很清楚：**"game-wide things (primarily gameplay cues)"** —— 这是**全局的**技能系统，主要给"全场性特效"用（比如全场胜利烟花），不是给某个角色的。

另外还有两个广播消息的函数（可靠版 / 不可靠版）：

| 函数 | 用途 |
|---|---|
| `MulticastMessageToClients` | 不可靠，**可以丢**的通知（击杀播报、加入提示） |
| `MulticastReliableMessageToClients` | 可靠，**不能丢**的通知 |

### ③ `LyraWorldSettings`

改动很小，就一件事：**给地图指定"默认玩哪个 Experience"**。

```29:35:Source/LyraGame/GameModes/LyraWorldSettings.h
	// Returns the default experience to use when a server opens this map if it is not overridden by the user-facing experience
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	// The default experience to use when a server opens this map if it is not overridden by the user-facing experience
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<ULyraExperienceDefinition> DefaultGameplayExperience;
```

也就是说：**地图可以有自己的默认玩法，但在大厅里选的玩法优先级更高。** 这样单独 PIE 一张图也能跑起来。
另外还有个编辑器专用的 `ForceStandaloneNetMode`（PIE 时强制单机，方便调试前端地图）。

---

## 六、小工具：`LyraBotCreationComponent`

挂在 GameState 上的组件，干一件事：**开局自动补 AI**。

```22:26:Source/LyraGame/GameModes/LyraBotCreationComponent.h
	//~UActorComponent interface
	virtual void BeginPlay() override;
	//~End of UActorComponent interface

private:
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);
```

注意它的 `BeginPlay` 里做的事就是**注册 `OnExperienceLoaded` 回调**（又是这个模式）——因为它必须等 Experience 加载完才知道该给机器人配置什么角色。

配的东西：`NumBotsToCreate`（数量）、`BotControllerClass`（AI 控制器）、`RandomBotNames`（随机名字池）。
还提供了作弊函数 `Cheat_AddBot()` / `Cheat_RemoveBot()`，方便调试。

---

## 七、把整个目录串起来：进一局游戏发生了什么

```
① 玩家在大厅点一张"卡片"
   └── 【数据层】UserFacingExperienceDefinition
        └─ CreateHostingRequest() → 开房，带上 MapID + ExperienceID + ExtraArgs

② 地图加载，GameMode 出场
   └── 【传统三大件】ALyraGameMode
        ├─ 构造函数换掉 GameState/PlayerController/PlayerState/Pawn/HUD 类
        └─ InitGame → 下一帧开始"找 Experience"

③ 按优先级挑一个 Experience
   URL > 开发者设置(PIE) > 命令行 > WorldSettings > 专用服务器 > 默认
   └── 【数据层】LyraExperienceDefinition（就是个 ID）
        └─ 交给 GameState 上的 ExperienceManagerComponent

④ ★ 加载状态机开始跑
   └── 【流程层】ULyraExperienceManagerComponent
        ├─ Loading：           加载 Experience 资产（区分 client/server bundle）
        ├─ LoadingGameFeatures：并行激活所有要求的插件（计数器等齐）
        ├─ （可选混沌延迟：故意卡一下，测加载表现）
        ├─ ExecutingActions：  执行 Actions —— 【GameFeatures 目录】的那些动作
        │                      OnGameFeatureRegistering → Loading → Activating
        └─ Loaded：            高 → 中 → 低 三级广播

⑤ 各方收到"就绪"通知，各自开工
   ├─ 【传统三大件】GameMode.OnExperienceLoaded → 给等待中的玩家生成角色
   ├─ 【小工具】BotCreationComponent        → 生成 AI
   ├─ 【UI】加载界面                     → 收起（ShouldShowLoadingScreen 返回 false）
   └─ 【技能系统】AddAbilities 动作       → 给角色挂技能（等角色出生时再补）

⑥ 退出这局
   └─ EndPlay → 反向执行 Actions（Deactivating → Unregistering）
   └─ 【流程层】LyraExperienceManager → 引用计数减到 0 才真卸插件
```

---

## 八、四个容易困惑的点

**Q1：`ExperienceDefinition` 和 `UserFacingExperienceDefinition` 到底啥区别？**

| | UserFacing | 非 UserFacing |
|---|---|---|
| 给谁看 | **玩家**（大厅 UI） | **引擎**（加载流程） |
| 有什么 | 标题、图标、描述、人数、加载界面 | 插件列表、PawnData、Actions |
| 关系 | 指向一个 ExperienceDefinition | ← 被指向 |
| 类比 | 菜单上的菜 | 后厨的配方 |

**Q2：为什么这个目录叫 `GameModes` 但主角是 Experience？**
因为是历史包袱 —— 目录沿用旧名，但 Lyra 的玩法组织方式已经换成 Experience 了。GameMode 在这个体系里退化成"搬运工 + 生成角色的执行者"。

**Q3：为什么玩家进来先挂起不生成？**
因为"角色长什么样"（PawnData）来自 Experience。都不确定这局玩什么，就不知道该造什么角色。
**顺带的好处**：避免了"先生成一个默认角色，等 Experience 加载完再换掉"的抖动。

**Q4：为什么 Actions 在 Experience 和 GameFeature 里都能挂？**
指的是同一个 `UGameFeatureAction` 基类，生命周期回调也一样。区别只是**触发时机**：
- 插件里的 Actions → 由插件激活驱动（更细粒度，插件自己的事）
- Experience 里的 Actions → 由这局游戏加载驱动（更粗粒度，跨插件的事）

---

## 九、三句话总结

1. **这个目录的核心是 `Experience`**：它把"一局游戏长什么样"从 GameMode 代码里搬了出来，变成一个可组合的数据资产（插件 + PawnData + Actions + ActionSets）。
2. **`LyraExperienceManagerComponent` 是发动机**：一个跑在 GameState 上的六步状态机，负责"加载资产 → 激活插件 → 执行动作 → 广播就绪"，并且全程撑着加载界面。
3. **整套设计在对抗"顺序不确定"**：玩家可能先进来、插件可能并行加载、UI 可能先就绪 —— 于是到处是 `CallOrRegister_OnExperienceLoaded`（**没加载完就排队，加载完了就立刻回调**）这种写法。

---

## 十、建议的深入顺序

| 顺序 | 文件 | 为什么要看 |
|---|---|---|
| 1 | `LyraExperienceDefinition.h` | 先看清"一局游戏"到底由什么组成 |
| 2 | `LyraExperienceManagerComponent.h/.cpp` | ★ 全目录核心，加载状态机 |
| 3 | `LyraGameMode.cpp` | 理解体验是怎么被选中、以及玩家为什么要等 |
| 4 | `LyraUserFacingExperienceDefinition.h` | 理解"大厅 → 开局"这一跳 |
| 5 | `LyraGameState.cpp` | 理解 Experience 组件和全局 ASC 为什么在 GameState 上 |
| 6 | `AsyncAction_ExperienceReady` / `LyraExperienceManager` / `BotCreationComponent` / `WorldSettings` | 四个小而实用的补充件 |
