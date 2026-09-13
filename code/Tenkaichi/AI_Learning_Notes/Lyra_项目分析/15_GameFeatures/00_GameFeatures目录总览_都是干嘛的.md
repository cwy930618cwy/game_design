# GameFeatures 目录总览：都是干嘛的

> 结论先说：**这个目录 = Lyra 给"插件式功能包"写的动作库 + 加载策略。**
>
> 一个 GameFeature（游戏功能）本质上是**一个 UE 插件**：里面自带内容、资源、代码。
> 而这个目录里的 16 个文件，回答的是两个问题：
> **① 插件什么时候加载、怎么加载**（策略层）
> **② 插件加载后，到底对游戏做什么**（动作层）

📌 阅读顺序：**本篇（看地图）→ 单文件笔记**

---

## 一、先搞懂"GameFeature"是什么

先建立画面，再看代码：

```
传统做法                           GameFeature 做法
─────────────────────────         ─────────────────────────
玩法代码全写进主工程                玩法打包成一个插件
关掉玩法要删代码 / 改分支            卸载插件即可
新玩法上线要发新版本                可以运行时动态加载
不同模式共用一套 UI/输入             每个模式带自己的 UI/输入
```

打个比方：主工程是**操作系统**，每个 GameFeature 是**一个 App**。
App 里可以带自己的界面、自己的快捷键、自己的功能。

**那"这个 App 要做什么"，写在哪？**

写在插件里的 `UGameFeatureData` 资产上 —— 它有**一个 Actions 列表**：

```
ShooterCore 插件
└── GameFeatureData
    └── Actions:
        ├── [Add Abilities]          ← 给角色发技能
        ├── [Add Input Binds]        ← 加按键绑定
        ├── [Add Input Mapping]      ← 加键位映射
        ├── [Add Widgets]            ← 加 HUD
        └── [Splitscreen Config]     ← 禁用分屏
```

**每一个中括号，就是一个 `UGameFeatureAction` 子类。**
→ 而这个目录，**就是 Lyra 自己写的那些中括号**。

---

## 二、目录长相：16 个文件，分三块

```
GameFeatures/
├── 【策略层】 LyraGameFeaturePolicy.h/.cpp        ← 插件怎么被发现和加载
│
├── 【动作基类】 GameFeatureAction_WorldActionBase.h/.cpp
│                └─ 所有"和世界有关"的动作都继承它
│
└── 【6 个具体动作】
    ├── GameFeatureAction_AddAbilities          ← 发技能/属性
    ├── GameFeatureAction_AddInputBinding       ← 发"按键→标签"绑定
    ├── GameFeatureAction_AddInputContextMapping← 发键位映射（IMC）
    ├── GameFeatureAction_AddWidget             ← 发 UI
    ├── GameFeatureAction_SplitscreenConfig     ← 禁用分屏
    └── GameFeatureAction_AddGameplayCuePath    ← 注册特效目录
```

| 分类 | 文件数 | 一句话 |
|---|---|---|
| 策略层 | 1 组（内含 2 个观察者） | 谁来发现插件、加载哪些数据、加载时做哪些额外事 |
| 动作基类 | 1 组 | 把动作分发到"所有相关的世界"，并处理"以后新开的世界" |
| 具体动作 | 6 组 | 一个动作解决一件事：技能、输入、UI、分屏、特效路径 |

---

## 三、策略层：`LyraGameFeaturePolicy`

它继承 `UDefaultGameFeaturesProjectPolicies`，是**整个 GameFeature 系统的总开关**。做三件事：

**1. 注册两个"观察者"（Observer）**

```19:31:Source/LyraGame/GameFeatures/LyraGameFeaturePolicy.cpp
void ULyraGameFeaturePolicy::InitGameFeatureManager()
{
	Observers.Add(NewObject<ULyraGameFeature_HotfixManager>());
	Observers.Add(NewObject<ULyraGameFeature_AddGameplayCuePaths>());
	...
}
```

| 观察者 | 监听时机 | 干什么 |
|---|---|---|
| `ULyraGameFeature_HotfixManager` | `OnGameFeatureLoading` | 加载插件时，**顺手请求一趟热更** |
| `ULyraGameFeature_AddGameplayCuePaths` | `OnGameFeatureRegistering` | 把插件里的 `GameplayCues` 目录**注册进 Cue 管理器** |

第 2 个观察者特别值得注意：它做的事，其实和 `GameFeatureAction_AddGameplayCuePath` 是**同一件事**。
区别在于**时机**：

- **AddGameplayCuePath 动作**：只是**声明**"我这个插件有这些特效目录"
- **AddGameplayCuePaths 观察者**：在插件**注册的最早阶段**就去真正执行注册

为什么要提前到"注册"阶段？因为特效路径要尽早知道，不能等到进游戏才补。

**2. 决定加载模式**

```55:60:Source/LyraGame/GameFeatures/LyraGameFeaturePolicy.cpp
void ULyraGameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
	// Editor will load both, this can cause hitching as the bundles are set to not preload in editor
	bLoadClientData = !IsRunningDedicatedServer();
	bLoadServerData = !IsRunningClientOnly();
}
```

客户端不加载"纯服务器数据"，服务器不加载"纯客户端数据"。分类依据是资产上的 **AssetBundles** 标记（你在动作的 UPROPERTY 里能看到 `meta=(AssetBundles="Client,Server")`）。

**3. 预加载与插件白名单**
`GetPreloadAssetListForGameFeature()`（加载插件时顺带预载哪些资产）、`IsPluginAllowed()`（这个插件 URL 允不允许加载）。Lyra 里基本是转调父类，属于**留给你扩展的钩子**。

---

## 四、动作基类：`WorldActionBase` —— 为什么需要它

先看它干了什么：

```10:23:Source/LyraGame/GameFeatures/GameFeatureAction_WorldActionBase.cpp
void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	GameInstanceStartHandles.FindOrAdd(Context) = FWorldDelegates::OnStartGameInstance.AddUObject(this, 
		&UGameFeatureAction_WorldActionBase::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}
```

**它解决的是一个很容易忽略的问题：一个动作要作用于"哪个世界"？**

现实里有多个世界同时存在：编辑器里的 PIE 世界、独立服务器世界、客户端世界……所以基类做了**两路分发**：

| 路 | 时机 | 目的 |
|---|---|---|
| **① 遍历现有世界** | 插件激活的当下 | 把动作加到**已经存在**的每个合适世界 |
| **② 订阅 `OnStartGameInstance`** | 以后 | 保证**后面才开**的世界也不会漏 |

而子类只需要实现一个纯虚函数：

```cpp
virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) = 0;
```

**注意：`AddGameplayCuePath` 没有继承它** —— 因为"注册特效目录"跟具体世界无关，越早越好，所以它直接继承最上层的 `UGameFeatureAction`。

---

## 五、6 个具体动作

### ① `AddAbilities` —— 给角色发技能（最重要）

配置长这样：

```
AbilitiesList
└── [0]
    ├── ActorClass          = 要加给哪一类 Actor（比如 BP_LyraCharacter）
    ├── GrantedAbilities    = 要给的技能（GA 类）
    ├── GrantedAttributes   = 要给的属性集（+ 初始化用 DataTable）
    └── GrantedAbilitySets  = 要给的"能力礼包"（ULyraAbilitySet）
```

**它做的事：给这类 Actor 发技能，并在插件卸载时收回来。**

发送：
- 技能 → `ASC->GiveAbility()`，记下 handle
- 属性集 → `NewObject` + `AddAttributeSetSubobject()`，也可以从 DataTable 初始化数值
- 能力礼包 → `ULyraAbilitySet::GiveToAbilitySystem()`，记下 handles

收回（`RemoveActorAbilities`）：
- 属性集 → `RemoveSpawnedAttribute()`
- 技能 → `SetRemoveAbilityOnEnd(handle)`
- 礼包 → `GrantedHandles.TakeFromAbilitySystem()`

**这里有一个非常关键的细节** —— 它不是"直接去找 Actor"，而是**注册一个监听器**：

```121:125:Source/LyraGame/GameFeatures/GameFeatureAction_AddAbilities.cpp
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
						this, &UGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex, ChangeContext);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);
```

**为什么？**

> 因为 GameFeature 是**异步加载**的。插件加载完成的那一刻，目标角色**可能还没出生**。

`AddExtensionHandler` 的意思是：

> "以后凡是生成一个 `ActorClass` 类型的 Actor，或者它进入了某个初始化阶段，**叫我一声**。"

然后 `HandleActorExtension` 根据回调事件名决定加还是减：

| 事件名 | 含义 | 动作 |
|---|---|---|
| `NAME_ExtensionAdded` | 这类 Actor 出现了 | **加**技能 |
| `NAME_LyraAbilityReady` | PlayerState 的能力系统准备好了 | **加**技能 |
| `NAME_ExtensionRemoved` | Actor 被移除了 | **收**技能 |
| `NAME_ReceiverRemoved` | 接受者没了 | **收**技能 |

**顺带一提：它还负责"补组件"**（`FindOrAddComponentForActor`）。如果目标 Actor 身上没有 ASC，它会通过 `AddComponentRequest` 请求一个（请求是引用计数的，所以不能随便重复请求）。

---

### ② `AddInputContextMapping` —— 加"键位映射"

概念先分清：

```
InputMappingContext（IMC）= 一副键位表
    左键 → IA_Fire   空格 → IA_Jump   1 → IA_SwitchWeapon

InputConfig = "某个按键该干什么"的绑定表
    IA_Fire → GameplayTag: InputTag.Weapon.Fire
```

`AddInputContextMapping` 加的是**前者（键位表）**，作用于 **PlayerController / LocalPlayer**。

两个特点：
1. **可以注册到设置里**（`bRegisterWithSettings`）—— 这样"设置界面"就能列出这些按键让玩家改键
2. 有 **Priority（优先级）**：高优先级的映射会压过低优先级的（比如"开镜时"的映射压过"平时"）

它除了跟世界分发，还要额外处理 **"以后才加入的本地玩家"**（比如 PIE 里开了 2 个玩家），所以才有 `RegisterInputContextMappingsForLocalPlayer()` 这类函数。

---

### ③ `AddInputBinding` —— 加"按键→标签"绑定

加的是**前者概念里那个 `ULyraInputConfig`**，作用于 **Pawn**。

关键代码只有一句：

```137:137:Source/LyraGame/GameFeatures/GameFeatureAction_AddInputBinding.cpp
						HeroComponent->AddAdditionalInputConfig(BindSet);
```

也就是说：**它把绑定表交给 `HeroComponent`，由 HeroComponent 统一管理。**
而且它必须等 HeroComponent 说"我可以绑输入了"（`IsReadyToBindInputs()` / `NAME_BindInputsNow` 事件）才动手。

> 这两者的分工可以用一句话记：
> **ContextMapping 管"哪个键"，AddInputBinding 管"这个键对应什么技能标签"。**

---

### ④ `AddWidgets` —— 加 UI

也是两种粒度：

| 配置 | 加什么 | 位置 |
|---|---|---|
| `Layout` | 一整套 HUD 布局（`UCommonActivatableWidget`） | 插到某个 **Layer**（`UI.Layer.*`） |
| `Widgets` | 单个小部件（`UUserWidget`） | 插到某个 **SlotID**（走 `UIExtensionSystem`） |

意义：**不同玩法模式可以带自己的 HUD**，而不是把所有人的 UI 都塞进主工程。

---

### ⑤ `SplitscreenConfig` —— 禁用分屏（投票机制）

这个动作很小但设计挺巧：**它不是"我禁用分屏"，而是"我投一票禁用分屏"**。

```54:74:Source/LyraGame/GameFeatures/GameFeatureAction_SplitscreenConfig.cpp
void UGameFeatureAction_SplitscreenConfig::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	if (bDisableSplitscreen)
	{
		...
				LocalDisableVotes.Add(ViewportKey);

				int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
				VoteCount++;
				if (VoteCount == 1)
				{
					VC->SetForceDisableSplitscreen(true);
				}
```

- 每个插件投一票 → 计数 +1，**只有从 0 变 1 时才真的禁用**
- 插件卸载时撤票，**只有减到 0 才恢复分屏**

为什么要投票？因为可能有**两个不同的模式**都要求禁用分屏。如果按"我走了就恢复"来写，先卸载的那个会把后者的禁用也给解了。

---

### ⑥ `AddGameplayCuePath` —— 注册特效目录

最小巧的一个：只是一个**声明**，告诉 Cue 管理器"我这个插件里的这些目录里放着特效"。

值得注意的是它的默认值 —— 构造函数里已经帮你填好了常用路径：

```12:16:Source/LyraGame/GameFeatures/GameFeatureAction_AddGameplayCuePath.cpp
UGameFeatureAction_AddGameplayCuePath::UGameFeatureAction_AddGameplayCuePath()
{
	// Add a default path that is commonly used
	DirectoryPathsToAdd.Add(FDirectoryPath{ TEXT("/GameplayCues") });
}
```

真正干活的是策略层那个观察者（见第三节）。

---

## 六、把整个目录串起来：加载"占点模式"插件会发生什么

假设现在要从大厅进入"占点模式"（一个 GameFeature 插件）：

```
① 插件开始加载
   └─ 策略层决定：客户端只读 Client 数据，服务器只读 Server 数据
   └─ 【观察者】HotfixManager 顺手请求一次热更
   └─ 【观察者】AddGameplayCuePaths 把插件里的 /GameplayCues 注册给 CueManager
        （这些都发生在"注册"阶段，比进游戏还早）

② 插件激活
   └─ 每个 Action 的 OnGameFeatureActivating 被调用
   └─ 若继承 WorldActionBase：
        ├─ 先订阅 OnStartGameInstance（保证以后新世界不漏）
        └─ 遍历当前所有世界 → 逐个 AddToWorld()

③ 各 Action 在 AddToWorld 里做自己的事
   ├─ AddAbilities            → 注册"监听该类角色"的扩展处理器
   ├─ AddInputContextMapping  → 给本地玩家挂上键位表
   ├─ AddInputBinding         → 等 Pawn 的 Hero 说"准备好绑输入了"→ 加绑定
   ├─ AddWidgets              → 把 HUD 布局插进 Layer，小部件插进 Slot
   └─ SplitscreenConfig       → 投一票"禁用分屏"

④ 玩家角色出生（可能在插件加载之后很久）
   └─ GameFrameworkComponentManager 广播"某类 Actor 有了扩展"
   └─ AddAbilities 的回调被触发 → 给这个角色的 ASC 发技能/属性集
   └─ AddInputBinding 的回调被触发 → 给这个角色的 Hero 加绑定

⑤ 插件卸载
   └─ 各 Action 的 OnGameFeatureDeactivating
   └─ AddAbilities 把技能/属性集/礼包**全部收回**
   └─ SplitscreenConfig 撤票（减到 0 才恢复分屏）
```

---

## 七、四个容易困惑的点

**Q1：为什么不直接用 `GetAllActorsOfClass` 找角色发技能？**
因为**时机不对**。插件加载完成时目标角色很可能还不存在。用 `AddExtensionHandler` 相当于"挂个长期监听"，新老角色都能覆盖到。

**Q2：为什么动作都要区分"哪个世界"？**
因为一个游戏进程可能有多个世界（PIE 多窗口、客户端+服务器）。动作必须知道"我这次是要加到这个世界，还是不关我事"，否则会出现"在编辑器里给这个窗口也偷偷加了一份"的问题。

**Q3：`AddInputBinding` 和 `AddInputContextMapping` 我总分不清？**

| | ContextMapping | AddInputBinding |
|---|---|---|
| 加的东西 | IMC（键位表） | InputConfig（键 → 标签） |
| 加给谁 | PlayerController / LocalPlayer | Pawn |
| 典型用途 | "这个模式用这套键位" | "这个模式多一个技能键" |

**Q4：为什么 `AddAbilities` 里要 `FindOrAddComponent` 而不是要求角色自己带 ASC？**
因为要兼容"不同角色蓝图"。有一类 Actor 可能压根没挂 ASC，但插件仍希望给它发能力 —— 那就通过组件请求系统补一个（而且请求是引用计数的）。

---

## 八、三句话总结

1. **这个目录不实现玩法，只实现"玩法怎么挂进游戏"**：它是一个**动作库**，每个 Action 解决一件挂载的事（技能、输入、UI、分屏、特效路径）。
2. **一切设计都在对抗"时机不确定"**：插件是异步加载的、角色是后来出生的、玩家是后来加入的、世界是后来创建的 —— 所以到处是"先注册监听，等回调再执行"的写法。
3. **有加必有减**：每个动作都配套了卸载逻辑（收技能、撤票、移除映射、移除 UI），这是动态加载能安全反复开关的前提。

---

## 九、建议的深入顺序

| 顺序 | 文件 | 为什么要看 |
|---|---|---|
| 1 | `GameFeatureAction_WorldActionBase.h/.cpp` | 理解"动作如何分发到世界"，所有动作的基础 |
| 2 | `GameFeatureAction_AddAbilities.h/.cpp` | 最典型、最完整的动作，能看清"加/减 + 监听"整套模式 |
| 3 | `LyraGameFeaturePolicy.h/.cpp` | 理解插件发现、加载模式、观察者 |
| 4 | `GameFeatureAction_AddInputBinding` + `AddInputContextMapping` | 输入系统是怎么被插件扩展的 |
| 5 | `GameFeatureAction_AddWidget.h/.cpp` | UI 是怎么被插件扩展的 |
| 6 | `SplitscreenConfig` / `AddGameplayCuePath` | 两个小巧但设计思路值得学的例子 |
