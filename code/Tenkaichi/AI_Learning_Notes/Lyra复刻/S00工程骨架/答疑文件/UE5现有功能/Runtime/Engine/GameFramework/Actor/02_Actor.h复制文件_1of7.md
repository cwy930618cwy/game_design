# 01 — `Actor.h` 复制文件（第 1 片：1~205 行）

> 依据：`D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`（**4763 行**，本片 1~205 行）
> **⚠️ 这是引擎源码讲解，不是让你往工程里新建这个文件** —— 引擎只读不改。
> **用法**：对照引擎源码逐行读懂。下面代码块里的 `//` 注释是中文讲解，敲的时候**整行注释都不用抄**。

```cpp
// Epic Games 公司版权声明，原样保留不翻译
// Copyright Epic Games, Inc. All Rights Reserved.

// 头文件保护：同一个编译单元里只展开一次
// 这个头在引擎里被广泛 include，必须有
#pragma once

// 统计宏：DECLARE_STATS_GROUP 等性能统计用
// 来自 Runtime\Core\Public\Stats\Stats.h
#include "Stats/Stats.h"

// UCLASS / UPROPERTY 等宏的定义
// 来自 Runtime\CoreUObject\Public\UObject\ObjectMacros.h
#include "UObject/ObjectMacros.h"

// UObject 基础工具函数
// 提供 IsValid / GetNameSafe 等
#include "UObject/UObjectBaseUtility.h"

// UObject 基类本体：AActor 的祖先
// 来自 Runtime\CoreUObject\Public\UObject\Object.h
#include "UObject/Object.h"

// 输入类型：FKey / ETouchIndex
// 下面的鼠标点击 / 触摸委托签名要用到
#include "InputCoreTypes.h"

// TSubclassOf 模板：类型安全的 UClass 指针
// AddComponentByClass 等接口的参数类型
#include "Templates/SubclassOf.h"

// 引擎基础类型：FTransform / FVector / EEndPlayReason 等
// 来自 Engine\Classes\Engine\EngineTypes.h
#include "Engine/EngineTypes.h"

// 更基础的引擎类型：ETeleportType / ESpawnActorCollisionHandlingMethod 等
#include "Engine/EngineBaseTypes.h"

// 属性对映射，配置系统用的键值结构
#include "PropertyPairsMap.h"

// 子 Actor 组件：AActor 可以挂 UChildActorComponent
#include "Components/ChildActorComponent.h"

// 渲染命令栅栏，保证渲染线程同步
#include "RenderCommandFence.h"

// 兼容 UE5.2 之前的 include 顺序：这些头现在已被间接包含
// 这个宏在 5.2 起默认关闭，所以下面 4 行一般不会展开
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2

// 关卡类 ULevel
#include "Engine/Level.h"

// 命中结果 FHitResult
#include "Engine/HitResult.h"

// 网络核心基础：FNetworkGUID 等
#include "UObject/CoreNet.h"

// 仅编辑器模式下：数据层（DataLayer）头
#if WITH_EDITOR

// 世界分区的数据层组件
#include "WorldPartition/DataLayer/ActorDataLayer.h"

// 编辑器专属 include 块结束（对应上面的 #if WITH_EDITOR）
#endif

// 旧版 include 顺序兼容块结束（对应 #if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2）
#endif

// 网络子对象注册表（Iris 复制系统用）
#include "Net/Core/Misc/NetSubObjectRegistry.h"

// 复制状态相关类型
#include "Engine/ReplicatedState.h"

// 仅编辑器模式下：文件夹头（Actor 在 Content Browser 里的 Folder 路径）
#if WITH_EDITOR

// UActorFolder 相关声明
#include "Folder.h"

// 编辑器专属 include 块结束（对应上面的 #if WITH_EDITOR）
#endif

// 世界分区的 Actor 描述符类型
// 决定 Actor 在世界分区里怎么被索引
#include "WorldPartition/WorldPartitionActorDescType.h"

// UHT（Unreal Header Tool）生成的反射代码头
// ⚠️ 必须放在所有手写 include 的最后一行，这是铁律
#include "Actor.generated.h"

// 以下是本文件用到的前置声明：只告诉编译器「有这么个类」
// 比 include 整个头文件便宜得多，能显著加快编译
// 本类自己：类文档和下面的委托签名里要引用
class AActor;
// 控制器基类：伤害委托里的 InstigatedBy 参数类型
class AController;
// 可被操纵的实体基类：MakeNoise 委托里的 APawn* 参数类型
class APawn;
// 玩家控制器：网络所有权与输入相关的接口会用到
class APlayerController;
// Actor 网络通道：服务器与客户端之间传输 Actor 数据
class UActorChannel;
// 子 Actor 组件：Actor 挂上它就能生成子 Actor
class UChildActorComponent;
// 网络驱动器：管理网络连接与属性复制
class UNetDriver;
// 图元组件基类：重叠 / 命中委托里的命中组件类型
class UPrimitiveComponent;
// 附着信息结构：记录 Actor 之间的父子附着关系
struct FAttachedActorInfo;
// 网络观察者：某条连接看到的视点，用于相关性判定
struct FNetViewer;
// 网络对象信息：Iris 复制系统里的对象元数据
struct FNetworkObjectInfo;
// 事务注解：编辑器撤销 / 重做时保存 Actor 状态
class FActorTransactionAnnotation;
// 组件实例数据缓存：构造脚本重建组件时保留原来的值
class FComponentInstanceDataCache;
// 已废弃的旧版数据层：仅为兼容保留
class UDEPRECATED_DataLayer;
// 数据层资产（新版）：把 Actor 按逻辑分组
class UDataLayerAsset;
// 外部数据层资产：定义在单独资产里的数据层
class UExternalDataLayerAsset;
// 数据层实例：数据层在世界里的具体实例
class UDataLayerInstance;
// 世界数据层 Actor：管理关卡内所有数据层
class AWorldDataLayers;
// 世界分区格子接口：判断 Actor 属于哪个运行时格子
class IWorldPartitionCell;

// 仅当启用 Iris（新一代网络复制系统）时
#if UE_WITH_IRIS

// Actor 开始复制的参数结构
struct FActorBeginReplicationParams;

// Iris 复制系统专属块结束（对应上面的 #if UE_WITH_IRIS）
#endif // UE_WITH_IRIS

// Actor 文件夹：编辑器里给 Actor 分组的目录
class UActorFolder;

// Actor 的数据层信息
struct FActorDataLayer;

// 命中结果：射线 / 碰撞检测的输出
struct FHitResult;

// UE 网络命名空间
namespace UE::Net
// 命名空间体开始
{

// 拆分复制（Tear Off）的设置器，Iris 内部用
class FTearOffSetter;

// 命名空间体结束（UE::Net）
}

// 是否保留 Actor 标签的编译开关
// 默认 Debug / Development / GPU Profile 保留；Test / Shipping 不保留
// 想让发行版也保留，可以手动 define
#define ACTOR_HAS_LABELS (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT || WITH_PROFILEGPU)

// 初始化时如何更新重叠状态（当前主要用于关卡流式加载）
// 标记为蓝图可用
UENUM(BlueprintType)

// 枚举：Actor 更新重叠的策略
enum class EActorUpdateOverlapsMethod : uint8
// 枚举体开始
{
	// 用原生类或配置里指定的默认值
	UseConfigDefault,

	// 总是更新重叠状态
	AlwaysUpdate,

	// 只有根组件是可移动（Movable）时才更新
	OnlyUpdateMovable,

	// 从不更新重叠状态
	NeverUpdate
// 枚举体结束（EActorUpdateOverlapsMethod）
};

// 生成 Actor 时，传入的缩放和根组件默认缩放怎么结合
// 标记为蓝图可用
UENUM(BlueprintType)

// 枚举：生成时的缩放处理方式
enum class ESpawnActorScaleMethod : uint8
// 枚举体开始
{
	// 忽略根组件的默认缩放，直接用 SpawnTransform 里的值硬设置
	OverrideRootScale						UMETA(DisplayName = "Override Root Component Scale"),

	// 把 SpawnTransform 的缩放乘以根组件默认缩放
	MultiplyWithRoot						UMETA(DisplayName = "Multiply Scale With Root Component Scale"),

	// 运行时再决定（隐藏，蓝图里看不到）
	SelectDefaultAtRuntime					UMETA(Hidden),
// 枚举体结束（ESpawnActorScaleMethod）
};

// 仅编辑器数据：世界分区相关（已废弃）
#if WITH_EDITORONLY_DATA

// 标记：这个枚举在 UE5.0 已废弃，不要在新代码里用
UENUM()

// 枚举：Actor 在分区里怎么放置（已废弃）
enum class UE_DEPRECATED(5.0, "EActorGridPlacement is deprecated.") EActorGridPlacement : uint8
// 枚举体开始
{
	// 用 Actor 的包围盒决定放进哪些运行时格子
	Bounds,

	// 用 Actor 的位置决定放进哪些运行时格子
	Location,

	// 总是加载（不放进格子），也会影响编辑器
	AlwaysLoaded,

	// 隐藏项
	None UMETA(Hidden)
// 枚举体结束（EActorGridPlacement）
};

// 仅编辑器数据块结束（对应 #if WITH_EDITORONLY_DATA）
#endif

// 声明 LogActor 日志类别：默认等级 Log，警告等级 Warning
// 宏展开后本文里就能用 UE_LOG(LogActor, ...)
ENGINE_API DECLARE_LOG_CATEGORY_EXTERN(LogActor, Log, Warning);

// 以下是各种事件委托的签名声明
// 用 DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_* 系列宏，支持蓝图绑定且省内存

// 受到任意伤害时广播：受伤者、伤害值、伤害类型、发起者、造成伤害的 Actor
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FiveParams( FTakeAnyDamageSignature, AActor, OnTakeAnyDamage, AActor*, DamagedActor, float, Damage, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser );

// 受到点伤害时广播：比上面多了命中位置、命中组件、骨骼名、射击方向
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_NineParams( FTakePointDamageSignature, AActor, OnTakePointDamage, AActor*, DamagedActor, float, Damage, class AController*, InstigatedBy, FVector, HitLocation, class UPrimitiveComponent*, FHitComponent, FName, BoneName, FVector, ShotFromDirection, const class UDamageType*, DamageType, AActor*, DamageCauser );

// 受到范围伤害时广播：多了爆炸中心和命中信息
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_SevenParams( FTakeRadialDamageSignature, AActor, OnTakeRadialDamage, AActor*, DamagedActor, float, Damage, const class UDamageType*, DamageType, FVector, Origin, const FHitResult&, HitInfo, class AController*, InstigatedBy, AActor*, DamageCauser );

// 开始重叠：两个 Actor 碰到一起
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorBeginOverlapSignature, AActor, OnActorBeginOverlap, AActor*, OverlappedActor, AActor*, OtherActor );

// 结束重叠：两个 Actor 分开
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorEndOverlapSignature, AActor, OnActorEndOverlap, AActor*, OverlappedActor, AActor*, OtherActor );

// 发生物理命中：带法线冲量和完整命中结果
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_FourParams( FActorHitSignature, AActor, OnActorHit, AActor*, SelfActor, AActor*, OtherActor, FVector, NormalImpulse, const FHitResult&, Hit );

// 鼠标移入 Actor
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FActorBeginCursorOverSignature, AActor, OnBeginCursorOver, AActor*, TouchedActor );

// 鼠标移出 Actor
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FActorEndCursorOverSignature, AActor, OnEndCursorOver, AActor*, TouchedActor );

// 鼠标点击 Actor：带按下的键
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorOnClickedSignature, AActor, OnClicked, AActor*, TouchedActor , FKey, ButtonPressed );

// 鼠标松开 Actor：带松开的键
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorOnReleasedSignature, AActor, OnReleased, AActor*, TouchedActor , FKey, ButtonReleased );

// 触摸开始：带手指索引
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorOnInputTouchBeginSignature, AActor, OnInputTouchBegin, ETouchIndex::Type, FingerIndex, AActor*, TouchedActor );

// 触摸结束：带手指索引
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorOnInputTouchEndSignature, AActor, OnInputTouchEnd, ETouchIndex::Type, FingerIndex, AActor*, TouchedActor );

// 手指移入 Actor
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorBeginTouchOverSignature, AActor, OnInputTouchEnter, ETouchIndex::Type, FingerIndex, AActor*, TouchedActor );

// 手指移出 Actor
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams( FActorEndTouchOverSignature, AActor, OnInputTouchLeave, ETouchIndex::Type, FingerIndex, AActor*, TouchedActor );

// Actor 被销毁时广播
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FActorDestroyedSignature, AActor, OnDestroyed, AActor*, DestroyedActor );

// Actor 结束游玩时广播：带原因（关卡切换 / 销毁 / 距离剔除等）
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FActorEndPlaySignature, AActor, OnEndPlay, AActor*, Actor , EEndPlayReason::Type, EndPlayReason);

// 发出噪音（AI 感知系统用）：响度、发起者、位置、最大范围、标签
DECLARE_DELEGATE_SixParams(FMakeNoiseDelegate, AActor*, float /*Loudness*/, class APawn*, const FVector&, float /*MaxRange*/, FName /*Tag*/);

// 仅编辑器模式：打包模式变化时广播（是否外部 Actor）
#if WITH_EDITOR

// 打包模式变化事件
DECLARE_EVENT_TwoParams(AActor, FActorOnPackagingModeChanged, AActor*, bool /* bExternal */);

// 编辑器专属委托块结束（对应上面的 #if WITH_EDITOR）
#endif

// 非发行版才有：ProcessEvent 的拦截委托（调试 / 热重载用）
#if !UE_BUILD_SHIPPING

// 处理事件时回调：返回 bool 表示是否拦截
DECLARE_DELEGATE_RetVal_ThreeParams(bool, FOnProcessEvent, AActor*, UFunction*, void*);

// 非发行版专属委托块结束（对应 #if !UE_BUILD_SHIPPING）
#endif

// TInlineComponentArray 是个「栈上预留空间」的 TArray
// 元素少的时候不堆分配，避免频繁 new
template<class T, uint32 NumElements = NumInlinedActorComponents>

// 模板类：继承 TArray，但用内联分配器
class TInlineComponentArray : public TArray<T, TInlineAllocator<NumElements>>
// 类体开始
{
	// 简化父类写法
	typedef TArray<T, TInlineAllocator<NumElements>> Super;

// 以下成员对外公开
public:

	// 默认构造
	TInlineComponentArray() : Super() { }

	// 从某个 Actor 收集组件来构造
	// bIncludeFromChildActors 决定是否连子 Actor 的组件一起收
	TInlineComponentArray(const AActor* Actor, bool bIncludeFromChildActors = false);
// 类体结束（TInlineComponentArray）
};

// ★★★ 以下是 AActor 的官方类文档，全文件最重要的一段注释 ★★★
// Actor 是可以放进关卡里、或者被动态生成出来的对象基类
// Actor 可以挂一组 ActorComponent，用来控制它怎么移动、怎么渲染等等
// Actor 的另一个主要职责是在游戏过程中把属性和函数调用复制到网络对端
//
// Actor 的初始化有好几步，下面是重要虚函数的调用顺序：
// - UObject::PostLoad：静态摆在关卡里的 Actor 会走，编辑器和游戏都有；新生成的 Actor 不走
// - UActorComponent::OnComponentCreated：编辑器或游戏里生成 Actor 时，原生组件会调用；
//   蓝图创建的组件在构造时调用；从关卡加载的组件不走
// - AActor::PreRegisterAllComponents：静态 Actor 和有原生根组件的生成 Actor 会走；
//   没有原生根组件的蓝图 Actor 会晚一点在构造时走
// - UActorComponent::RegisterComponent：编辑器和运行时所有组件都会注册，
//   这一步创建物理 / 视觉表现；可能跨帧分发，但一定在 PreRegisterAllComponents 之后
// - AActor::PostRegisterAllComponents：所有 Actor 都会走，是「所有情况都调用」的最后一个函数
// - AActor::PostActorCreated：编辑器或游戏里创建 Actor 时，紧接在构造之前调用；
//   从关卡加载的组件不走
// - AActor::UserConstructionScript：实现了构造脚本的蓝图会调用
// - AActor::OnConstruction：ExecuteConstruction 结束时调用，会执行蓝图构造脚本；
//   在所有蓝图组件创建注册完之后；游戏里只有生成时调用，编辑器里改蓝图可能重跑
// - AActor::PreInitializeComponents：在组件 InitializeComponent 之前调用；
//   只在游戏和某些编辑器预览窗口里调用
// - UActorComponent::Activate：只有 bAutoActivate 为 true 才自动调用，之后也可手动激活
// - UActorComponent::InitializeComponent：只有 bWantsInitializeComponent 为 true 才调用，每次游戏只一次
// - AActor::PostInitializeComponents：组件初始化完之后调用，只在游戏和某些编辑器预览里
// - AActor::BeginPlay：关卡开始 Tick 时调用，只在真正游戏过程中；
//   通常紧跟 PostInitializeComponents，但网络 Actor 或子 Actor 可能延迟
//
// 官方文档：https://docs.unrealengine.com/Programming/UnrealArchitecture/Actors
// 生命周期文档：https://docs.unrealengine.com/Programming/UnrealArchitecture/Actors/ActorLifecycle
// 相关类：UActorComponent
//
// 正式声明 UCLASS：蓝图可用、蓝图可继承、配置组为 Engine、带简短提示、最小 API 导出
UCLASS(BlueprintType, Blueprintable, config=Engine, meta=(ShortTooltip="An Actor is an object that can be placed or spawned in the world."), MinimalAPI)

// AActor 继承自 UObject
class AActor : public UObject
// AActor 类体开始
{
	// UHT 生成的反射代码必须放在类体第一行
	GENERATED_BODY()

// 以下成员对外公开（构造函数等）
public:

	// 默认构造函数
	ENGINE_API AActor();

	// 带 ObjectInitializer 的构造函数，为向后兼容保留
	ENGINE_API AActor(const FObjectInitializer& ObjectInitializer);
```

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 路径 | 只读对照 `Engine\Classes\GameFramework\Actor.h`，**不往工程里写** |
| 代码行 | 与引擎原文逐字一致（含缩进、宏名、`UMETA` 里的空格） |
| 注释中文化 | 原英文注释全译中文；零英文讲解注释 |
| 注释位置 | 全部独占一行、写在代码正上方；零行尾注释 |
| UHT 铁律 | `Actor.generated.h` 必须是最后一个 include；`GENERATED_BODY()` 必须是类体第一行 |
| 完整性 | 1~205 行一比一贴全，不能跳 `#if` / `#endif` |

## 本片新元素

| 元素 | 一句话 |
|---|---|
| `UCLASS(BlueprintType, Blueprintable, ...)` | 让这个类能在蓝图里用、能被蓝图继承 |
| `MinimalAPI` | 只导出必要的符号，减少编译时间和 DLL 体积 |
| `GENERATED_BODY()` | UHT 生成的反射代码插入点，必须类体第一行 |
| `DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_*` | 支持蓝图绑定的多播委托，`SPARSE` 表示未绑定时不占内存 |
| `ENGINE_API` | 导出宏，让别的模块能链接到这个函数 |
| `TInlineAllocator<N>` | 在对象内部预留 N 个元素的空间，避免堆分配 |

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h` 第 1~205 行（逐行实查）
