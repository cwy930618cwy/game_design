# `LyraCharacter.h` 干了什么

> 源码：`Source/LyraGame/Character/LyraCharacter.h`（全文 232 行）
> 本文**只讲头文件**，不涉及任何 `.cpp` 实现。
> 目的：读完知道**这个头文件里定义了哪些东西、它们各自负责什么**。

---

## 〇、一句话结论

`LyraCharacter.h` 里其实装了**两拨东西**：

```
拨一（配角）：两个"省流量"的网络小结构体 + 一个 traits 特化     ← 第 36 ~ 88 行
拨二（主角）：ALyraCharacter 类                                ← 第 90 ~ 231 行
```

而 `ALyraCharacter` 自己的定位，源码注释第 94~95 行一句话说得很干脆：

```90:96:Source/LyraGame/Character/LyraCharacter.h
/**
 * ALyraCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
```

翻译成人话：**它是个"壳 + 接线员"。它负责在正确时机把事件转发给身上的组件；新功能请加在组件里，别往它身上堆。**

记住这一句，后面所有函数你都能看懂"为什么长这样"。

---

## 一、开头的准备动作（第 1 ~ 31 行）

### 1. 五个 `#include`（第 5 ~ 9 行）——它要当哪几种"角色"

```5:9:Source/LyraGame/Character/LyraCharacter.h
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "Teams/LyraTeamAgentInterface.h"
```

这五个 include 不是随便写的，**每一个对应它后面要实现的一个身份**（见第三节的接口表）。看到这里就应该预感：这个类要"对外宣称"四种能力。

### 2. `UE_API` 宏（第 13 行 + 第 231 行 `#undef`）

```13:13:Source/LyraGame/Character/LyraCharacter.h
#define UE_API LYRAGAME_API
```

这是 Lyra 的一个统一写法：**整个头文件里的函数声明都写成 `UE_API xxx`**，文件末尾再 `#undef`。
好处是：以后如果要把这个类挪到别的模块，只要改这一行的宏定义（甚至改成空），不用逐个函数去改导出宏。
—— 这就是你之前那篇 `UE_API` 笔记在真实项目里的落点。

### 3. 一大坨 `class XXX;` 前置声明（第 15 ~ 31 行）

```15:31:Source/LyraGame/Character/LyraCharacter.h
class AActor;
class AController;
class ALyraPlayerController;
class ALyraPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class ULyraAbilitySystemComponent;
class ULyraCameraComponent;
class ULyraHealthComponent;
class ULyraPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;
```

为什么不直接 include 它们的头文件？因为**头文件里只用到"指针/引用"，不需要知道对方的完整定义**。
只写一句"有这么个类"就能编译，能少 include 一堆东西，**编译速度会快很多**（改一个公共头不至于引发全工程重编）。

配合看第 194 ~ 201 行的成员变量，全是 `TObjectPtr<...>`，正好对上前置声明：

```194:201:Source/LyraGame/Character/LyraCharacter.h
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;
```

**顺带一个重要发现**：`ULyraCameraComponent` 在 `Character/` 目录里**根本没有源文件**（它在 `Source/LyraGame/Camera/`），这里只出现了一个前置声明 + 一个成员指针。所以 `LyraCharacter.h` 是"引用"相机，不是"定义"相机。

---

## 二、拨一：两个网络小结构（第 33 ~ 88 行）

`ALyraCharacter` 之前先定义了两个 `USTRUCT`，都是**"为了少发点数据"**而存在的。当角色数量多、移动频繁时，这两个结构能实打实省带宽。

### ① `FLyraReplicatedAcceleration` —— 把加速度压成 3 个字节（第 33 ~ 49 行）

```36:49:Source/LyraGame/Character/LyraCharacter.h
USTRUCT()
struct FLyraReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};
```

**它在解决什么问题**：角色的加速度本来是个 `FVector`（3 个 `float` = 12 字节，且浮点复制精度开销大）。但"角色往哪个方向加速、加多猛"其实**不需要精确值**，玩家看不出区别。

**做法**：把方向、大小、Z 分量各自"量化"成一个小整数存起来：

| 字段 | 类型 | 存的什么 | 注释里的取值范围 |
|---|---|---|---|
| `AccelXYRadians` | `uint8` | XY 平面上加速度的**方向角** | `[0, 2π]` 映射到 0~255 |
| `AccelXYMagnitude` | `uint8` | XY 平面加速度的**大小** | `[0, MaxAcceleration]` |
| `AccelZ` | `int8` | 垂直方向加速度（有正负） | `[-MaxAcceleration, MaxAcceleration]` |

**一句话**：这是"为了网络，主动降低精度"的典型手法——**12 字节的向量 → 3 个字节**。

> 小细节：`FLyraReplicatedAcceleration` 本身**没有写 `NetSerialize`**，它就是靠三个小整数属性直接被复制；真正需要它的是第 203 行的 `ReplicatedAcceleration` 成员。

### ② `FSharedRepMovement` —— "快速共享移动"用的数据包（第 51 ~ 78 行）

```52:78:Source/LyraGame/Character/LyraCharacter.h
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};
```

**它在解决什么问题**：默认的属性复制有个特性——**如果这一帧角色的属性都没变（比如只是移动了位置），引擎会跳过整次复制**。这在省带宽上是好事，但会导致"移动看起来不跟手"。Lyra 的应对是：**属性复制跳过没关系，我再单独发一个很轻的"移动播报"**。

这个结构就是那个"移动播报包"，里面装：

| 成员 | 装的东西 | 为什么要它 |
|---|---|---|
| `RepMovement` | 标准移动数据（位置/旋转/速度，带量化压缩） | 引擎现成的结构，直接复用 |
| `RepTimeStamp` | 这次移动的时间戳 | 收包方要知道"这是哪一刻的状态" |
| `RepMovementMode` | 移动模式（走/跳/坠落…）用 1 字节存 | 枚举本来就要复制，压成 uint8 |
| `bProxyIsJumpForceApplied` | 是否被外部施加了跳跃力 | 模拟端要还原这个状态 |
| `bIsCrouched` | 是否蹲着 | 蹲伏会影响胶囊高度，必须同步 |

**另外三个"函数声明"要特别注意它的存在感**（这里只声明，实现都在 `.cpp`）：

| 函数 | 名字直译 | 干什么 |
|---|---|---|
| `FillForCharacter(ACharacter*)` | 从角色身上"抓取" | 从 `ACharacter` 读出当前状态，填满这个结构 |
| `Equals(Other, Character)` | 和上次比是否相同 | **用来判断"值不值得发"**，和上一份一样就别发了 |
| `NetSerialize(...)` | 自定义网络序列化 | 手写压缩的序列化规则（比默认的更省），参数是引擎标准签名 |

### ③ `TStructOpsTypeTraits<FSharedRepMovement>` 特化（第 80 ~ 88 行）

```80:88:Source/LyraGame/Character/LyraCharacter.h
template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};
```

这是给引擎的一个"**报名表**"：告诉 UHT/引擎"我这个结构有特殊能力"。

| 开关 | 含义 | 带来的效果 |
|---|---|---|
| `WithNetSerializer = true` | "我有自己的 `NetSerialize`" | 引擎复制时**调用我写的 `NetSerialize`**，而不是默认那套 |
| `WithNetSharedSerialization = true` | "我允许**共享序列化**" | 同一份数据发**很多个连接**时，可以只序列化一次、复用给所有人 —— 人多时省 CPU |

> 这两个开关是"上一行写了 `NetSerialize` 却可能不生效"的常见坑位。**声明了 `NetSerialize` 还不够，必须在 traits 里打开 `WithNetSerializer`。**

---

## 三、拨二：`ALyraCharacter` 类本体（第 90 ~ 231 行）

### 1. 类声明：它同时是"五种东西"（第 98 行）

```97:98:Source/LyraGame/Character/LyraCharacter.h
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class ALyraCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ILyraTeamAgentInterface
```

| 继承/实现的 | 意思（人话） |
|---|---|
| `AModularCharacter` | 一个"模块化"的 Character：**功能几乎全靠组件加**，自己保持干净 |
| `IAbilitySystemInterface` | "我能提供 ASC" —— 能力系统的标准入口 |
| `IGameplayCueInterface` | "我身上能播 GameplayCue" —— 特效/音效的表现入口 |
| `IGameplayTagAssetInterface` | "我能回答我有没有某个 Tag" —— 给外部查询用 |
| `ILyraTeamAgentInterface` | "我属于某个队伍" —— 队伍系统 |

`UCLASS` 上的三个标记：`MinimalAPI`（导出最小化）、`Config = Game`（**属性可在 ini 里配**）、`ShortTooltip`（编辑器里的提示语）。

### 2. 公开部分：给外面用的 API

**(a) 拿"兄弟对象"（第 106 ~ 114 行）**

```104:119:Source/LyraGame/Character/LyraCharacter.h
	UE_API ALyraCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API ALyraPlayerController* GetLyraPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API ALyraPlayerState* GetLyraPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	UE_API ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
```

| 函数 | 给你什么 |
|---|---|
| `GetLyraPlayerController()` | 操控我的那个 `ALyraPlayerController` |
| `GetLyraPlayerState()` | 我的 `ALyraPlayerState` —— **角色最关键的 ASC 就挂在它上面** |
| `GetLyraAbilitySystemComponent()` | 我用的 Lyra 版 ASC |
| `GetAbilitySystemComponent()` | 接口要求的版本（**返回类型是引擎基类**，上面那个是 Lyra 版） |

> 为什么 Lyra 要提供**两个** `GetAbilitySystemComponent`？因为接口要求签名必须是基类 `UAbilitySystemComponent*`；但 Lyra 自己的代码想用 Lyra 扩展的方法，所以额外给一个返回 `ULyraAbilitySystemComponent*` 的非虚函数。这是 UE 里很常见的一招。

**(b) 四个 Tag 查询（第 116 ~ 119 行）**

```116:119:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
```

这四个就是 `IGameplayTagAssetInterface` 要求的"标准四问"。**它们本身没逻辑**，就是把问题转交给 ASC 去答。
（对照你之前那篇 `GameplayTagContainer` 笔记：`HasTag` 会做父子匹配，`HasTagExact` 不会 —— 这里就是对外暴露这层查询能力。）

**(c) 蹲伏开关（第 121 行）**

```121:121:Source/LyraGame/Character/LyraCharacter.h
	UE_API void ToggleCrouch();
```

对外一个"切换蹲伏"，蓝图里也能叫。

**(d) 重写的时机钩子（第 123 ~ 140 行）**

```123:140:Source/LyraGame/Character/LyraCharacter.h
	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	//~ILyraTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface
```

**看注释分界线 `//~AActor interface` / `//~End of ... interface`** —— 这是 UE 的书写规范，一眼能看出"这一坨是在重写哪个接口的函数"。三种：

- `AActor`：出生前初始化、开始、结束、重置、**声明哪些属性要复制**（`GetLifetimeReplicatedProps`）、**复制前预处理**（`PreReplication` ← 和下面的 FastShared 优化强相关）
- `APawn`：**控制器变了**（`NotifyControllerChanged`）
- `ILyraTeamAgentInterface`：队伍 ID 的读写 + "队伍变化委托"的获取

**(e) 快速共享移动的三个成员（第 142 ~ 149 行）★**

```142:149:Source/LyraGame/Character/LyraCharacter.h
	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	UE_API void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	UE_API virtual bool UpdateSharedReplication();
```

这三行是第二节那个 `FSharedRepMovement` 的**使用现场**：

| 成员 | 作用 | 关键词解读 |
|---|---|---|
| `FastSharedReplication(...)` | 把一份移动播报**广播给所有客户端** | `NetMulticast` = 一次调用、所有端都收到；`unreliable` = **不可靠传输**——丢了就丢了，反正下一帧还有新的，不值得重传 |
| `LastSharedReplication` | 记住"我上次发的是啥" | 注释直说：**避免重复发送**（配合 `Equals` 比较） |
| `UpdateSharedReplication()` | 由它来决定"这帧要不要发" | 返回 `bool`，"更不更新"，在 `PreReplication` 里被调用 |

**设计意图一句话**：默认属性复制跳过移动更新的那些帧，就用这个 `unreliable` 多播补上，**又平滑又省带宽**。

### 3. `protected`：真正"干活"的钩子（第 151 ~ 190 行）

这一区全是被引擎/基类回调的函数（`virtual` 重写）。不用记名字，**按职责分组记**：

**(a) 能力系统接线（第 153 ~ 154 行）**
```153:154:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();
```
ASC 就绪 / ASC 被摘掉时，角色要做的响应（把自己注册成 Avatar、初始化 Tag 等）。

**(b) 控制权变化（第 156 ~ 160 行）**
```156:160:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;

	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;
```
`PossessedBy/UnPossessed` 是**服务器**才知道的事；`OnRep_Controller/OnRep_PlayerState` 是**客户端**收到复制后的响应。两者成对出现 —— **服务器做逻辑，客户端补表现**，这是 UE 网络代码的固定套路。

**(c) 输入（第 162 行）**
```162:162:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
```
引擎问"你要绑输入吗"，角色答："我转给 `ULyraHeroComponent` 去绑。"

**(d) Tag 初始化（第 164 行）**
```164:164:Source/LyraGame/Character/LyraCharacter.h
	UE_API void InitializeGameplayTags();
```
给角色初始打上一批 Tag（比如"我是角色"）。

**(e) 掉出世界（第 166 行）**
```166:166:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void FellOutOfWorld(const class UDamageType& dmgType) override;
```
掉进虚空了 —— 通常直接算死。

**(f) 死亡流程（第 168 ~ 182 行）★**
```168:182:Source/LyraGame/Character/LyraCharacter.h
	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

	UE_API void DisableMovementAndCollision();
	UE_API void DestroyDueToDeath();
	UE_API void UninitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	UE_API void K2_OnDeathFinished();
```
这是**整份头文件里最完整的一段业务链**，配合 `ULyraHealthComponent` 的两个委托使用：

| 阶段 | 函数 | 干什么（看注释） |
|---|---|---|
| ① 死亡开始 | `OnDeathStarted` | 关碰撞、停移动（`DisableMovementAndCollision`） |
| ② 死亡结束 | `OnDeathFinished` | 脱控制器、销毁 Pawn（`DestroyDueToDeath` / `UninitAndDestroy`） |
| ③ 通知蓝图 | `K2_OnDeathFinished` | **`BlueprintImplementableEvent`** —— 纯给蓝图的事件，C++ 侧没有实现，策划/美术在这儿接死亡表现 |

> `K2_` 前缀 + `BlueprintImplementableEvent` = "只有 C++ 声明、实现在蓝图的函数"。这是 UE 里 C++ 给蓝图留口子的标准做法。

**(g) 移动模式 + 蹲伏 + 跳跃（第 184 ~ 190 行）**
```184:190:Source/LyraGame/Character/LyraCharacter.h
	UE_API virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	UE_API void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	UE_API virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	UE_API virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UE_API virtual bool CanJumpInternal_Implementation() const;
```
| 函数 | 干什么 |
|---|---|
| `OnMovementModeChanged` | 移动模式变了（走/跳/坠落/游泳…）时响应 |
| `SetMovementModeTag(...)` | **把"当前移动模式"翻译成 GameplayTag 打/拔**（`bTagEnabled` 控制是加还是删）—— 这样能力系统/UI 就能用 Tag 判断"我现在在飞还是在跑" |
| `OnStartCrouch` / `OnEndCrouch` | 蹲下/站起时调整（胶囊高度变了，可能要改碰撞或相机） |
| `CanJumpInternal_Implementation` | 引擎"我现在能跳吗"的钩子（`_Implementation` = 蓝图的 `BlueprintNativeEvent` 生成的 C++ 实现体名字） |

### 4. `private` 成员：它身上的"零件"和"状态"（第 192 ~ 210 行）

```192:210:Source/LyraGame/Character/LyraCharacter.h
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FLyraReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;
```

| 成员 | 是什么 | 标记解读 |
|---|---|---|
| `PawnExtComponent` | 初始化协调 + PawnData + ASC 挂载点 | `VisibleAnywhere` = 编辑器里能看不能改；`BlueprintReadOnly` = 蓝图能读；`AllowPrivateAccess` = **private 也允许蓝图像公开一样访问**（UE 里私有成员上蓝图的通行做法） |
| `HealthComponent` | 血量/死亡 | 同上 |
| `CameraComponent` | 相机（**注意它在 `Camera/` 目录，不在本目录**） | 同上 |
| `ReplicatedAcceleration` | 第二节那个压缩加速度 | `Transient`（不存档，只跑网络）+ `ReplicatedUsing = OnRep_ReplicatedAcceleration`（客户端收到后调这个回调） |
| `MyTeamID` | 我属于哪个队 | `ReplicatedUsing = OnRep_MyTeamID` |
| `OnTeamChangedDelegate` | 队伍变化时广播 | 就是接口里 `GetOnTeamIndexChangedDelegate` 返回的那个 |

**注意一个不对称**：`PawnExtComponent` / `HealthComponent` / `CameraComponent` 三个组件**都没有加 `Replicated`** —— 因为**组件本身是随 Actor 一起创建的、天然各端都有**，不需要复制属性；而 `ReplicatedAcceleration` / `MyTeamID` 是**只存在服务器、需要同步给客户端的"状态"**，所以必须标 `Replicated`。

> 这是 UE 网络编程里一个反复出现的判断："**结构性的东西（组件）不复制，状态性的东西才复制。**"

### 5. `protected` 的一个小策略函数（第 212 ~ 218 行）

```212:218:Source/LyraGame/Character/LyraCharacter.h
protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}
```

名字直译：**"控制结束时，我的新队伍 ID 应该是什么？"**
默认答案：`NoTeam`（退出队伍）。注释明确告诉你这是个**留给你改的策略点**：想保留原队伍就 `return OldTeamID`，想变成中立阵营就返回别的。

**注意它是 `protected virtual` 且直接在头文件里给了默认实现** —— 子类想改就改，不改也能用。这种"带默认实现的虚函数"在 Lyra 里很常见（叫"策略点 / hook"）。

### 6. `private` 三个回调（第 220 ~ 228 行）

```220:228:Source/LyraGame/Character/LyraCharacter.h
private:
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	UE_API void OnRep_ReplicatedAcceleration();

	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);
```

| 回调 | 谁来调 |
|---|---|
| `OnControllerChangedTeam` | **控制器**换队伍了，跟着更新自己 |
| `OnRep_ReplicatedAcceleration` | 客户端：压缩加速度复制到位，把数据"解压"回 `Acceleration` |
| `OnRep_MyTeamID` | 客户端：队伍 ID 复制到位，广播队伍变化 |

三个都带 `UFUNCTION()` —— **这是必须的**：UE 的反射系统要能按名字找到回调函数（`NetMulticast`、`ReplicatedUsing`、委托绑定都依赖它）。

---

## 四、这份头文件的四层结构（最省脑的记法）

```
┌─ 前戏（1~31）     include 定身份 + UE_API 宏 + 前置声明
├─ 配角（33~88）    两个网络结构 + traits 报名表    → 主题：省带宽
│                   FLyraReplicatedAcceleration（压缩加速度）
│                   FSharedRepMovement（快速移动播报包）
├─ 主角（90~149）   ALyraCharacter 的"对外面孔"
│                   4 个接口身份 / 拿 PlayerController·PlayerState·ASC / Tag 四问 / 蹲伏 / FastShared 多播
├─ 里子（151~190）  protected 钩子 = 真正干活的地方
│                   ASC 接线 · 控制权变化 · 输入 · 死亡链 · 移动模式转 Tag
└─ 零件（192~228）  private 成员 + 三个网络回调
                    三个组件（不复制）+ 两个状态（要复制）
```

---

## 五、必须带走的 6 个认知

1. **`ALyraCharacter` 是"壳"**：源码注释亲口说"新功能请加到 Pawn 组件里"。看到它身上逻辑很少，不是没写完，是**故意的**。
2. **它同时是 4 种身份**：ASC 提供者、Cue 播放者、Tag 回答者、队伍成员。
3. **三个组件成员是它的"内脏"**：`PawnExtComponent`（初始化+数据+ASC）、`HealthComponent`（血量死亡）、`CameraComponent`（相机，**定义在 `Camera/` 目录**）。
4. **两个网络结构是它的"省流量装备"**：加速度压成 3 字节；移动用 `unreliable` 多播补发 + `LastSharedReplication` 去重。
5. **有 `NetSerialize` 一定要配 `WithNetSerializer`**（traits 里），否则可能不生效。
6. **复制有讲究**：组件是结构性的 → 不复制；`ReplicatedAcceleration` / `MyTeamID` 是状态性的 → 复制 + `OnRep_` 回调。

---

## 六、下一步该看什么

这份头文件里，**所有"怎么做到的"都藏在 `.cpp` 里**，建议按这个顺序对照读：

| 想搞清楚 | 去 `.cpp` 里看 |
|---|---|
| 初始化链怎么串起来 | `PreInitializeComponents` / `BeginPlay` → 与 `LyraPawnExtensionComponent` 的交互 |
| 能力系统怎么挂上去 | `OnAbilitySystemInitialized` / `PossessedBy` / `OnRep_PlayerState` |
| 移动怎么省带宽 | `UpdateSharedReplication` / `PreReplication` / `FastSharedReplication` |
| 死亡流程完整路径 | `OnDeathStarted` → `DisableMovementAndCollision` → `OnDeathFinished` → `UninitAndDestroy` |
| 移动模式怎么变成 Tag | `OnMovementModeChanged` → `SetMovementModeTag` |
