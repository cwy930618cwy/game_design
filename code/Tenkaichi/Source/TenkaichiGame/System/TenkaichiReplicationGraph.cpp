// 版权声明（可保留或省略，不影响编译）

/*
*	===================== TenkaichiReplicationGraph 复制 =====================
*
*	概述
*
*		这改变了 Actor 相关性的工作方式。这个系统里不用 AActor::IsNetRelevantFor！
*
*		取而代之，UTenkaichiReplicationGraph 包含 UReplicationGraphNode（复制图节点）。
*		这些节点负责为每个连接生成"要复制的 Actor 列表"。
*		大部分列表跨帧持久存在。这让大部分收集工作（"哪些 Actor 该考虑复制"）可以共享/复用。
*		节点可以是全局的（所有连接用）、连接特定的（每个连接自己的节点）、或共享的（比如队伍：同队伍的所有连接共享）。
*		Actor 可以在多个节点里！比如一个 Pawn 既在空间化节点、又在"队伍始终相关"节点。对队友它会返回两次。
*		这没问题，不过应该尽可能减少。
*
*		UTenkaichiReplicationGraph 的意图是不被游戏代码直接使用。也就是说，你不应该在其他任何地方 include TenkaichiReplicationGraph.h。
*		相反，UTenkaichiReplicationGraph 依赖游戏代码，并注册游戏代码广播的事件（比如玩家加入/离开队伍的事件）。
*		做这个选择是因为它给了 UTenkaichiReplicationGraph 一个 Actor 复制的完整整体视图。
*		而不是暴露一堆游戏代码任何地方都能调用的通用公开函数，所有通知都显式注册在 UTenkaichiReplicationGraph::InitGlobalActorClassSettings 里。
*
*	Tenkaichi 节点
*
*		这些是当前使用的顶层节点：
*
*		UReplicationGraphNode_GridSpatialization2D：
*		这是空间化节点。所有"基于距离相关"的 Actor 都路由到这里。这个节点把地图分成 2D 网格。
*		网格里每个单元包含子节点，子节点根据 Actor 如何更新/休眠来持有 Actor 列表。Actor 放在多个单元里。连接从它们所在的单个单元拉取。
*
*		UReplicationGraphNode_ActorList：
*		这是持有"始终相关" Actor 的 Actor 列表节点。这些 Actor 对每个连接都始终相关。
*
*		UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection：
*		这是连接特定的"始终相关" Actor 节点。这个节点不维护持久列表，而是每帧构建。
*		这可行是因为（当前）这些 Actor 都能从 PlayerController 轻松访问。
*		持久列表需要在这些 Actor 变化时广播通知，这可行但目前没必要。
*
*		UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter：
*		一个处理 PlayerState 复制的自定义节点。它复制一小批滚动的 PlayerState（当前每帧 2 个）。
*		这是为了让 PlayerState 以低频、稳定的频率复制到模拟连接，并利用序列化共享。
*		自动代理 PlayerState 通过 UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection 以更高频率复制（只到拥有连接）。
*
*		UReplicationGraphNode_TearOff_ForConnection：
*		处理"撕离"（TearOff）Actor 的连接特定节点。这在复制图的基类实现里创建和管理。
*
*	如何使用
*
*		让某物始终相关：请尽量避免 :) 如果必须，只要在类默认值里设置 AActor::bAlwaysRelevant = true 就行。
*
*		让某物对连接始终相关：你需要修改 UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection。
*		你也要确保 Actor 不被放进其他节点。最安全的做法是在 UTenkaichiReplicationGraph::InitGlobalActorClassSettings 里把它的 EClassRepNodeMapping 设成 NotRouted。
*
*	如何调试
*
*		直接禁用复制图看你的问题是不是这个系统特有的，还是普通复制/玩法问题，这是个好主意。
*
*		如果是复制图相关，有几个有用的命令可以用：见 ReplicationGraph_Debugging.cpp。最有用的在下面。用 'cheat' 命令从客户端在服务器上运行它们。
*
*		"Net.RepGraph.PrintGraph" - 把图打印到日志：每个节点和 Actor。
*		"Net.RepGraph.PrintGraph class" - 同上，但按类分组。
*		"Net.RepGraph.PrintAll <Frames> <ConnectionIdx> <\"Class\"/\"Nclass\">" - 打印整张图、收集到的 Actor、以及它们在给定连接上 X 帧内是如何被优先化的。
*		"Net.RepGraph.PrintAllActorInfo <ActorMatchString>" - 打印与 Actor/类关联的类、全局、连接复制信息。MatchString 为空就打印所有。直接从客户端调用。
*		"Tenkaichi.RepGraph.PrintRouting" - 打印每个类的 EClassRepNodeMapping。也就是某个 Actor 类在复制图里如何路由（或不路由）。
*
*/

#include "TenkaichiReplicationGraph.h"

#include "Net/UnrealNetwork.h"
#include "Engine/LevelStreaming.h"
#include "EngineUtils.h"
#include "CoreGlobals.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/NetConnection.h"
#include "UObject/UObjectIterator.h"

#include "TenkaichiReplicationGraphSettings.h"
#include "Character/TenkaichiCharacter.h"
#include "Player/TenkaichiPlayerController.h"

// 定义复制图日志通道（配合 .h 里的 DECLARE_LOG_CATEGORY_EXTERN）。
DEFINE_LOG_CATEGORY( LogTenkaichiRepGraph );

// 命名空间：复制图相关的控制台变量。
namespace Tenkaichi::RepGraph
{
	// 破坏信息的最大复制距离。
	float DestructionInfoMaxDist = 30000.f;
	static FAutoConsoleVariableRef CVarTenkaichiRepGraphDestructMaxDist(TEXT("Tenkaichi.RepGraph.DestructInfo.MaxDist"), DestructionInfoMaxDist, TEXT("Max distance (not squared) to rep destruct infos at"), ECVF_Default);

	// 显示客户端关卡流送（调试）。
	int32 DisplayClientLevelStreaming = 0;
	static FAutoConsoleVariableRef CVarTenkaichiRepGraphDisplayClientLevelStreaming(TEXT("Tenkaichi.RepGraph.DisplayClientLevelStreaming"), DisplayClientLevelStreaming, TEXT(""), ECVF_Default);

	// 网格单元大小。
	float CellSize = 10000.f;
	static FAutoConsoleVariableRef CVarTenkaichiRepGraphCellSize(TEXT("Tenkaichi.RepGraph.CellSize"), CellSize, TEXT(""), ECVF_Default);

	// 复制空间的"最小 X"。这只是初始值，如果有 Actor 出现在这个范围之外，系统会重置自己。
	float SpatialBiasX = -150000.f;
	static FAutoConsoleVariableRef CVarTenkaichiRepGraphSpatialBiasX(TEXT("Tenkaichi.RepGraph.SpatialBiasX"), SpatialBiasX, TEXT(""), ECVF_Default);

	// 复制空间的"最小 Y"。这只是初始值，如果有 Actor 出现在这个范围之外，系统会重置自己。
	float SpatialBiasY = -200000.f;
	static FAutoConsoleVariableRef CVarTenkaichiRepSpatialBiasY(TEXT("Tenkaichi.RepGraph.SpatialBiasY"), SpatialBiasY, TEXT(""), ECVF_Default);

	// 把动态、空间化的 Actor 分散到多少个桶里。数字越高 = 桶越多 = 有效复制频率越低。这发生在单个 Actor 做它们自己的 NetUpdateFrequency 检查之前。
	int32 DynamicActorFrequencyBuckets = 3;
	static FAutoConsoleVariableRef CVarTenkaichiRepDynamicActorFrequencyBuckets(TEXT("Tenkaichi.RepGraph.DynamicActorFrequencyBuckets"), DynamicActorFrequencyBuckets, TEXT(""), ECVF_Default);

	// 是否禁用空间重建。
	int32 DisableSpatialRebuilds = 1;
	static FAutoConsoleVariableRef CVarTenkaichiRepDisableSpatialRebuilds(TEXT("Tenkaichi.RepGraph.DisableSpatialRebuilds"), DisableSpatialRebuilds, TEXT(""), ECVF_Default);

	// 是否记录惰性初始化类。
	int32 LogLazyInitClasses = 0;
	static FAutoConsoleVariableRef CVarTenkaichiRepLogLazyInitClasses(TEXT("Tenkaichi.RepGraph.LogLazyInitClasses"), LogLazyInitClasses, TEXT(""), ECVF_Default);

	// FastShared 移动更新用多少带宽。这个独立于 NetDriver 的目标带宽计算。
	int32 TargetKBytesSecFastSharedPath = 10;
	static FAutoConsoleVariableRef CVarTenkaichiRepTargetKBytesSecFastSharedPath(TEXT("Tenkaichi.RepGraph.TargetKBytesSecFastSharedPath"), TargetKBytesSecFastSharedPath, TEXT(""), ECVF_Default);

	// FastShared 路径的剔除距离百分比。
	float FastSharedPathCullDistPct = 0.80f;
	static FAutoConsoleVariableRef CVarTenkaichiRepFastSharedPathCullDistPct(TEXT("Tenkaichi.RepGraph.FastSharedPathCullDistPct"), FastSharedPathCullDistPct, TEXT(""), ECVF_Default);

	// 是否启用 FastShared 路径。
	int32 EnableFastSharedPath = 1;
	static FAutoConsoleVariableRef CVarTenkaichiRepEnableFastSharedPath(TEXT("Tenkaichi.RepGraph.EnableFastSharedPath"), EnableFastSharedPath, TEXT(""), ECVF_Default);

	// 工厂函数：按需创建复制驱动（复制图）。
	UReplicationDriver* ConditionalCreateReplicationDriver(UNetDriver* ForNetDriver, UWorld* World)
	{
		// 只为 GameNetDriver 创建
		if (World && ForNetDriver && ForNetDriver->NetDriverName == NAME_GameNetDriver)
		{
			const UTenkaichiReplicationGraphSettings* TenkaichiRepGraphSettings = GetDefault<UTenkaichiReplicationGraphSettings>();

			// 通过开发者设置启用/禁用
			if (TenkaichiRepGraphSettings && TenkaichiRepGraphSettings->bDisableReplicationGraph)
			{
				UE_LOG(LogTenkaichiRepGraph, Display, TEXT("Replication graph is disabled via TenkaichiReplicationGraphSettings."));
				return nullptr;
			}

			UE_LOG(LogTenkaichiRepGraph, Display, TEXT("Replication graph is enabled for %s in world %s."), *GetNameSafe(ForNetDriver), *GetPathNameSafe(World));

			TSubclassOf<UTenkaichiReplicationGraph> GraphClass = TenkaichiRepGraphSettings->DefaultReplicationGraphClass.TryLoadClass<UTenkaichiReplicationGraph>();
			if (GraphClass.Get() == nullptr)
			{
				GraphClass = UTenkaichiReplicationGraph::StaticClass();
			}

			UTenkaichiReplicationGraph* TenkaichiReplicationGraph = NewObject<UTenkaichiReplicationGraph>(GetTransientPackage(), GraphClass.Get());
			return TenkaichiReplicationGraph;
		}

		return nullptr;
	}
};

// 构造函数：绑定"创建复制驱动"委托。
UTenkaichiReplicationGraph::UTenkaichiReplicationGraph()
{
	if (!UReplicationDriver::CreateReplicationDriverDelegate().IsBound())
	{
		UReplicationDriver::CreateReplicationDriverDelegate().BindLambda(
			[](UNetDriver* ForNetDriver, const FURL& URL, UWorld* World) -> UReplicationDriver*
			{
				return Tenkaichi::RepGraph::ConditionalCreateReplicationDriver(ForNetDriver, World);
			});
	}
}

// 重置游戏世界状态（清空流式关卡列表 + 重置连接节点）。
void UTenkaichiReplicationGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();

	AlwaysRelevantStreamingLevelActors.Empty();

	for (UNetReplicationGraphConnection* ConnManager : Connections)
	{
		for (UReplicationGraphNode* ConnectionNode : ConnManager->GetConnectionGraphNodes())
		{
			if (UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
			{
				AlwaysRelevantConnectionNode->ResetGameWorldState();
			}
		}
	}

	for (UNetReplicationGraphConnection* ConnManager : PendingConnections)
	{
		for (UReplicationGraphNode* ConnectionNode : ConnManager->GetConnectionGraphNodes())
		{
			if (UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
			{
				AlwaysRelevantConnectionNode->ResetGameWorldState();
			}
		}
	}
}

// 获取类的节点映射（有显式设置就用，没有就按 Actor 的旧版复制设置推断）。
EClassRepNodeMapping UTenkaichiReplicationGraph::GetClassNodeMapping(UClass* Class) const
{
	if (!Class)
	{
		return EClassRepNodeMapping::NotRouted;
	}

	if (const EClassRepNodeMapping* Ptr = ClassRepNodePolicies.FindWithoutClassRecursion(Class))
	{
		return *Ptr;
	}

	AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
	if (!ActorCDO || !ActorCDO->GetIsReplicated())
	{
		return EClassRepNodeMapping::NotRouted;
	}

	auto ShouldSpatialize = [](const AActor* CDO)
	{
		return CDO->GetIsReplicated() && (!(CDO->bAlwaysRelevant || CDO->bOnlyRelevantToOwner || CDO->bNetUseOwnerRelevancy));
	};

	auto GetLegacyDebugStr = [](const AActor* CDO)
	{
		return FString::Printf(TEXT("%s [%d/%d/%d]"), *CDO->GetClass()->GetName(), CDO->bAlwaysRelevant, CDO->bOnlyRelevantToOwner, CDO->bNetUseOwnerRelevancy);
	};

	// 只有这个类和父类不同才单独处理。没必要把每个子类都显式放进图类映射。
	UClass* SuperClass = Class->GetSuperClass();
	if (AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
	{
		if (SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
			&& SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant
			&& SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner
			&& SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy
			)
		{
			return GetClassNodeMapping(SuperClass);
		}
	}

	if (ShouldSpatialize(ActorCDO))
	{
		return EClassRepNodeMapping::Spatialize_Dynamic;
	}
	else if (ActorCDO->bAlwaysRelevant && !ActorCDO->bOnlyRelevantToOwner)
	{
		return EClassRepNodeMapping::RelevantAllConnections;
	}

	return EClassRepNodeMapping::NotRouted;
}

// 注册类的复制节点映射。
void UTenkaichiReplicationGraph::RegisterClassRepNodeMapping(UClass* Class)
{
	EClassRepNodeMapping Mapping = GetClassNodeMapping(Class);
	ClassRepNodePolicies.Set(Class, Mapping);
}

// 初始化类的复制信息。
void UTenkaichiReplicationGraph::InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool Spatialize) const
{
	AActor* CDO = Class->GetDefaultObject<AActor>();
	if (Spatialize)
	{
		Info.SetCullDistanceSquared(CDO->GetNetCullDistanceSquared());
		UE_LOG(LogTenkaichiRepGraph, Log, TEXT("Setting cull distance for %s to %f (%f)"), *Class->GetName(), Info.GetCullDistanceSquared(), Info.GetCullDistance());
	}

	Info.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(CDO->GetNetUpdateFrequency());

	UClass* NativeClass = Class;
	while (!NativeClass->IsNative() && NativeClass->GetSuperClass() && NativeClass->GetSuperClass() != AActor::StaticClass())
	{
		NativeClass = NativeClass->GetSuperClass();
	}

	UE_LOG(LogTenkaichiRepGraph, Log, TEXT("Setting replication period for %s (%s) to %d frames (%.2f)"), *Class->GetName(), *NativeClass->GetName(), Info.ReplicationPeriodFrame, CDO->GetNetUpdateFrequency());
}

// 有条件地初始化类的复制信息（显式设置过的类跳过）。
bool UTenkaichiReplicationGraph::ConditionalInitClassReplicationInfo(UClass* ReplicatedClass, FClassReplicationInfo& ClassInfo)
{
	if (ExplicitlySetClasses.FindByPredicate([&](const UClass* SetClass) { return ReplicatedClass->IsChildOf(SetClass); }) != nullptr)
	{
		return false;
	}

	bool ClassIsSpatialized = IsSpatialized(ClassRepNodePolicies.GetChecked(ReplicatedClass));
	InitClassReplicationInfo(ClassInfo, ReplicatedClass, ClassIsSpatialized);
	return true;
}

// 添加类的复制信息。
void UTenkaichiReplicationGraph::AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping)
{
	if (IsSpatialized(Mapping))
	{
		if (Class->GetDefaultObject<AActor>()->bAlwaysRelevant)
		{
			UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("Replicated Class %s is AlwaysRelevant but is initialized into a spatialized node (%s)"), *Class->GetName(), *StaticEnum<EClassRepNodeMapping>()->GetNameStringByValue((int64)Mapping));
		}
	}

	ClassRepNodePolicies.Set(Class, Mapping);
}

// 注册类的复制信息。
void UTenkaichiReplicationGraph::RegisterClassReplicationInfo(UClass* ReplicatedClass)
{
	FClassReplicationInfo ClassInfo;
	if (ConditionalInitClassReplicationInfo(ReplicatedClass, ClassInfo))
	{
		GlobalActorReplicationInfoMap.SetClassInfo(ReplicatedClass, ClassInfo);
		UE_LOG(LogTenkaichiRepGraph, Log, TEXT("Setting %s - %.2f"), *GetNameSafe(ReplicatedClass), ClassInfo.GetCullDistance());
	}
}

// 初始化全局 Actor 类设置（核心：把每类 Actor 映射到节点 + 设置复制信息）。
void UTenkaichiReplicationGraph::InitGlobalActorClassSettings()
{
	// 为当前还没加载的类设置惰性初始化函数。
	GlobalActorReplicationInfoMap.SetInitClassInfoFunc(
		[this](UClass* Class, FClassReplicationInfo& ClassInfo)
		{
			RegisterClassRepNodeMapping(Class); // 这需要在 RegisterClassReplicationInfo 之前跑。

			const bool bHandled = ConditionalInitClassReplicationInfo(Class, ClassInfo);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (Tenkaichi::RepGraph::LogLazyInitClasses != 0)
			{
				if (bHandled)
				{
					EClassRepNodeMapping Mapping = ClassRepNodePolicies.GetChecked(Class);
					UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("%s was Lazy Initialized. (Parent: %s) %d."), *GetNameSafe(Class), *GetNameSafe(Class->GetSuperClass()), (int32)Mapping);

					FClassReplicationInfo& ParentRepInfo = GlobalActorReplicationInfoMap.GetClassInfo(Class->GetSuperClass());
					if (ClassInfo.BuildDebugStringDelta() != ParentRepInfo.BuildDebugStringDelta())
					{
						UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("Differences Found!"));
						FString DebugStr = ParentRepInfo.BuildDebugStringDelta();
						UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("  Parent: %s"), *DebugStr);

						DebugStr = ClassInfo.BuildDebugStringDelta();
						UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("  Class : %s"), *DebugStr);
					}
				}
				else
				{
					UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("%s skipped Lazy Initialization because it does not differ from its parent. (Parent: %s)"), *GetNameSafe(Class), *GetNameSafe(Class->GetSuperClass()));

				}
			}
#endif

			return bHandled;
		});

	ClassRepNodePolicies.InitNewElement = [this](UClass* Class, EClassRepNodeMapping& NodeMapping)
	{
		NodeMapping = GetClassNodeMapping(Class);
		return true;
	};

	const UTenkaichiReplicationGraphSettings* TenkaichiRepGraphSettings = GetDefault<UTenkaichiReplicationGraphSettings>();
	check(TenkaichiRepGraphSettings);

	// 设置类的节点映射
	for (const FRepGraphActorClassSettings& ActorClassSettings : TenkaichiRepGraphSettings->ClassSettings)
	{
		if (ActorClassSettings.bAddClassRepInfoToMap)
		{
			if (UClass* StaticActorClass = ActorClassSettings.GetStaticActorClass())
			{
				UE_LOG(LogTenkaichiRepGraph, Log, TEXT("ActorClassSettings -- AddClassRepInfo - %s :: %i"), *StaticActorClass->GetName(), int(ActorClassSettings.ClassNodeMapping));
				AddClassRepInfo(StaticActorClass, ActorClassSettings.ClassNodeMapping);
			}
		}
	}

#if WITH_GAMEPLAY_DEBUGGER
	AddClassRepInfo(AGameplayDebuggerCategoryReplicator::StaticClass(), EClassRepNodeMapping::NotRouted);				// 通过 UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection 复制
#endif

	TArray<UClass*> AllReplicatedClasses;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}

		// 跳过 SKEL 和 REINST 类。我不知道更好的办法。
		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		// --------------------------------------------------------------------
		// 这是一个复制类。先存下来，下面第二轮处理
		// --------------------------------------------------------------------

		AllReplicatedClasses.Add(Class);

		RegisterClassRepNodeMapping(Class);
	}

	// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 设置 FClassReplicationInfo。本质上是每个类的复制设置。有些我们显式设置，其余通过看 AActor 上的旧版设置来设置。
	// -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	auto SetClassInfo = [&](UClass* Class, const FClassReplicationInfo& Info) { GlobalActorReplicationInfoMap.SetClassInfo(Class, Info); ExplicitlySetClasses.Add(Class); };
	ExplicitlySetClasses.Reset();

	FClassReplicationInfo CharacterClassRepInfo;
	CharacterClassRepInfo.DistancePriorityScale = 1.f;
	CharacterClassRepInfo.StarvationPriorityScale = 1.f;
	CharacterClassRepInfo.ActorChannelFrameTimeout = 4;
	CharacterClassRepInfo.SetCullDistanceSquared(ATenkaichiCharacter::StaticClass()->GetDefaultObject<ATenkaichiCharacter>()->GetNetCullDistanceSquared());

	SetClassInfo(ACharacter::StaticClass(), CharacterClassRepInfo);

	{
		// 检查我们的 FSharedRepMovement 类型和默认角色有相同的量化设置。
		FRepMovement DefaultRepMovement = ATenkaichiCharacter::StaticClass()->GetDefaultObject<ATenkaichiCharacter>()->GetReplicatedMovement(); // 用和我们默认 replicatedmovement 相同的量化设置
		FSharedRepMovement SharedRepMovement;
		ensureMsgf(SharedRepMovement.RepMovement.LocationQuantizationLevel == DefaultRepMovement.LocationQuantizationLevel, TEXT("LocationQuantizationLevel mismatch. %d != %d"), (uint8)SharedRepMovement.RepMovement.LocationQuantizationLevel, (uint8)DefaultRepMovement.LocationQuantizationLevel);
		ensureMsgf(SharedRepMovement.RepMovement.VelocityQuantizationLevel == DefaultRepMovement.VelocityQuantizationLevel, TEXT("VelocityQuantizationLevel mismatch. %d != %d"), (uint8)SharedRepMovement.RepMovement.VelocityQuantizationLevel, (uint8)DefaultRepMovement.VelocityQuantizationLevel);
		ensureMsgf(SharedRepMovement.RepMovement.RotationQuantizationLevel == DefaultRepMovement.RotationQuantizationLevel, TEXT("RotationQuantizationLevel mismatch. %d != %d"), (uint8)SharedRepMovement.RepMovement.RotationQuantizationLevel, (uint8)DefaultRepMovement.RotationQuantizationLevel);
	}

	// ------------------------------------------------------------------------------------------------------
	//	为 Pawn 设置 FastShared 复制。每帧每个 Pawn 最多调用一次，看它是否想给所有相关连接发一个 FastShared 更新。
	// ------------------------------------------------------------------------------------------------------
	CharacterClassRepInfo.FastSharedReplicationFunc = [](AActor* Actor)
	{
		bool bSuccess = false;
		if (ATenkaichiCharacter* Character = Cast<ATenkaichiCharacter>(Actor))
		{
			bSuccess = Character->UpdateSharedReplication();
		}
		return bSuccess;
	};

	CharacterClassRepInfo.FastSharedReplicationFuncName = FName(TEXT("FastSharedReplication"));

	FastSharedPathConstants.MaxBitsPerFrame = (int32)((float)(Tenkaichi::RepGraph::TargetKBytesSecFastSharedPath * 1024 * 8) / NetDriver->GetNetServerMaxTickRate());
	FastSharedPathConstants.DistanceRequirementPct = Tenkaichi::RepGraph::FastSharedPathCullDistPct;

	SetClassInfo(ATenkaichiCharacter::StaticClass(), CharacterClassRepInfo);

	// ---------------------------------------------------------------------
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.ListSize = 12;
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.NumBuckets = Tenkaichi::RepGraph::DynamicActorFrequencyBuckets;
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.BucketThresholds.Reset();
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.EnableFastPath = (Tenkaichi::RepGraph::EnableFastSharedPath > 0);
	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.FastPathFrameModulo = 1;

	RPCSendPolicyMap.Reset();

	// 根据所有复制类的旧版设置来设置 FClassReplicationInfo
	for (UClass* ReplicatedClass : AllReplicatedClasses)
	{
		RegisterClassReplicationInfo(ReplicatedClass);
	}

	// 打印我们得出的结果
	UE_LOG(LogTenkaichiRepGraph, Log, TEXT(""));
	UE_LOG(LogTenkaichiRepGraph, Log, TEXT("Class Routing Map: "));
	for (auto ClassMapIt = ClassRepNodePolicies.CreateIterator(); ClassMapIt; ++ClassMapIt)
	{
		UClass* Class = CastChecked<UClass>(ClassMapIt.Key().ResolveObjectPtr());
		EClassRepNodeMapping Mapping = ClassMapIt.Value();

		// 只打印和原生类不同的
		UClass* ParentNativeClass = GetParentNativeClass(Class);

		EClassRepNodeMapping* ParentMapping = ClassRepNodePolicies.Get(ParentNativeClass);
		if (ParentMapping && Class != ParentNativeClass && Mapping == *ParentMapping)
		{
			continue;
		}

		UE_LOG(LogTenkaichiRepGraph, Log, TEXT("  %s (%s) -> %s"), *Class->GetName(), *GetNameSafe(ParentNativeClass), *StaticEnum<EClassRepNodeMapping>()->GetNameStringByValue((int64)Mapping));
	}

	UE_LOG(LogTenkaichiRepGraph, Log, TEXT(""));
	UE_LOG(LogTenkaichiRepGraph, Log, TEXT("Class Settings Map: "));
	FClassReplicationInfo DefaultValues;
	for (auto ClassRepInfoIt = GlobalActorReplicationInfoMap.CreateClassMapIterator(); ClassRepInfoIt; ++ClassRepInfoIt)
	{
		UClass* Class = CastChecked<UClass>(ClassRepInfoIt.Key().ResolveObjectPtr());
		const FClassReplicationInfo& ClassInfo = ClassRepInfoIt.Value();
		UE_LOG(LogTenkaichiRepGraph, Log, TEXT("  %s (%s) -> %s"), *Class->GetName(), *GetNameSafe(GetParentNativeClass(Class)), *ClassInfo.BuildDebugStringDelta());
	}


	// 根据 CVar 值复制破坏信息
	DestructInfoMaxDistanceSquared = Tenkaichi::RepGraph::DestructionInfoMaxDist * Tenkaichi::RepGraph::DestructionInfoMaxDist;

#if WITH_GAMEPLAY_DEBUGGER
	AGameplayDebuggerCategoryReplicator::NotifyDebuggerOwnerChange.AddUObject(this, &ThisClass::OnGameplayDebuggerOwnerChange);
#endif

	// 加入 RPC_Multicast_OpenChannelForClass map
	RPC_Multicast_OpenChannelForClass.Reset();
	RPC_Multicast_OpenChannelForClass.Set(AActor::StaticClass(), true); // 默认给多播 RPC 打开通道
	RPC_Multicast_OpenChannelForClass.Set(AController::StaticClass(), false); // 多播绝不应该在 Controller 上打开通道，因为在非拥有者上打开通道会破坏 Controller 的复制。
	RPC_Multicast_OpenChannelForClass.Set(AServerStatReplicator::StaticClass(), false);

	for (const FRepGraphActorClassSettings& ActorClassSettings : TenkaichiRepGraphSettings->ClassSettings)
	{
		if (ActorClassSettings.bAddToRPC_Multicast_OpenChannelForClassMap)
		{
			if (UClass* StaticActorClass = ActorClassSettings.GetStaticActorClass())
			{
				UE_LOG(LogTenkaichiRepGraph, Log, TEXT("ActorClassSettings -- RPC_Multicast_OpenChannelForClass - %s"), *StaticActorClass->GetName());
				RPC_Multicast_OpenChannelForClass.Set(StaticActorClass, ActorClassSettings.bRPC_Multicast_OpenChannelForClass);
			}
		}
	}
}

// 初始化全局图节点（空间化网格 + 始终相关 + PlayerState 频率限制）。
void UTenkaichiReplicationGraph::InitGlobalGraphNodes()
{
	// -----------------------------------------------
	//	空间化 Actor
	// -----------------------------------------------

	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = Tenkaichi::RepGraph::CellSize;
	GridNode->SpatialBias = FVector2D(Tenkaichi::RepGraph::SpatialBiasX, Tenkaichi::RepGraph::SpatialBiasY);

	if (Tenkaichi::RepGraph::DisableSpatialRebuilds)
	{
		GridNode->AddToClassRebuildDenyList(AActor::StaticClass()); // 禁用所有空间重建
	}

	AddGlobalGraphNode(GridNode);

	// -----------------------------------------------
	//	始终相关（对所有人）Actor
	// -----------------------------------------------
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);

	// -----------------------------------------------
	//	Player State 特化。这会返回一个滚动的 PlayerState 子集来复制。
	// -----------------------------------------------
	UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter* PlayerStateNode = CreateNewNode<UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter>();
	AddGlobalGraphNode(PlayerStateNode);
}

// 初始化连接的图节点（连接级始终相关节点）。
void UTenkaichiReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
	Super::InitConnectionGraphNodes(RepGraphConnection);

	UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = CreateNewNode<UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection>();

	// 这个节点需要知道客户端关卡什么时候进入/退出可见性
	RepGraphConnection->OnClientVisibleLevelNameAdd.AddUObject(AlwaysRelevantConnectionNode, &UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd);
	RepGraphConnection->OnClientVisibleLevelNameRemove.AddUObject(AlwaysRelevantConnectionNode, &UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove);

	AddConnectionGraphNode(AlwaysRelevantConnectionNode, RepGraphConnection);
}

// 获取类的映射策略。
EClassRepNodeMapping UTenkaichiReplicationGraph::GetMappingPolicy(UClass* Class)
{
	EClassRepNodeMapping* PolicyPtr = ClassRepNodePolicies.Get(Class);
	EClassRepNodeMapping Policy = PolicyPtr ? *PolicyPtr : EClassRepNodeMapping::NotRouted;
	return Policy;
}

// 把网络 Actor 路由到节点（按映射策略分发）。
void UTenkaichiReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	EClassRepNodeMapping Policy = GetMappingPolicy(ActorInfo.Class);
	switch(Policy)
	{
		case EClassRepNodeMapping::NotRouted:
		{
			break;
		}

		case EClassRepNodeMapping::RelevantAllConnections:
		{
			if (ActorInfo.StreamingLevelName == NAME_None)
			{
				AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
			}
			else
			{
				FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindOrAdd(ActorInfo.StreamingLevelName);
				RepList.ConditionalAdd(ActorInfo.Actor);
			}
			break;
		}

		case EClassRepNodeMapping::Spatialize_Static:
		{
			GridNode->AddActor_Static(ActorInfo, GlobalInfo);
			break;
		}

		case EClassRepNodeMapping::Spatialize_Dynamic:
		{
			GridNode->AddActor_Dynamic(ActorInfo, GlobalInfo);
			break;
		}

		case EClassRepNodeMapping::Spatialize_Dormancy:
		{
			GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
			break;
		}
	};
}

// 把网络 Actor 从节点移除（按映射策略分发）。
void UTenkaichiReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	EClassRepNodeMapping Policy = GetMappingPolicy(ActorInfo.Class);
	switch(Policy)
	{
		case EClassRepNodeMapping::NotRouted:
		{
			break;
		}

		case EClassRepNodeMapping::RelevantAllConnections:
		{
			if (ActorInfo.StreamingLevelName == NAME_None)
			{
				AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
			}
			else
			{
				FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindChecked(ActorInfo.StreamingLevelName);
				if (RepList.RemoveFast(ActorInfo.Actor) == false)
				{
					UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("Actor %s was not found in AlwaysRelevantStreamingLevelActors list. LevelName: %s"), *GetActorRepListTypeDebugString(ActorInfo.Actor), *ActorInfo.StreamingLevelName.ToString());
				}
			}

			SetActorDestructionInfoToIgnoreDistanceCulling(ActorInfo.GetActor());

			break;
		}

		case EClassRepNodeMapping::Spatialize_Static:
		{
			GridNode->RemoveActor_Static(ActorInfo);
			break;
		}

		case EClassRepNodeMapping::Spatialize_Dynamic:
		{
			GridNode->RemoveActor_Dynamic(ActorInfo);
			break;
		}

		case EClassRepNodeMapping::Spatialize_Dormancy:
		{
			GridNode->RemoveActor_Dormancy(ActorInfo);
			break;
		}
	};
}

// 因为我们监听全局（静态）事件，需要小心跨 world 广播（PIE）
#if WITH_EDITOR
#define CHECK_WORLDS(X) if(X->GetWorld() != GetWorld()) return;
#else
#define CHECK_WORLDS(X)
#endif

#if WITH_GAMEPLAY_DEBUGGER
// 玩法调试器拥有者变化回调。
void UTenkaichiReplicationGraph::OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner)
{
	CHECK_WORLDS(Debugger);

	auto GetAlwaysRelevantForConnectionNode = [this](APlayerController* Controller) -> UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection*
	{
		if (Controller)
		{
			if (UNetConnection* NetConnection = Controller->GetNetConnection())
			{
				if (NetConnection->GetDriver() == NetDriver)
				{
					if (UNetReplicationGraphConnection* GraphConnection = FindOrAddConnectionManager(NetConnection))
					{
						for (UReplicationGraphNode* ConnectionNode : GraphConnection->GetConnectionGraphNodes())
						{
							if (UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = Cast<UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
							{
								return AlwaysRelevantConnectionNode;
							}
						}
					}
				}
			}
		}

		return nullptr;
	};

	if (UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(OldOwner))
	{
		AlwaysRelevantConnectionNode->GameplayDebugger = nullptr;
	}

	if (UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(Debugger->GetReplicationOwner()))
	{
		AlwaysRelevantConnectionNode->GameplayDebugger = Debugger;
	}
}
#endif

#undef CHECK_WORLDS

// 为连接收集 Actor 列表（每帧重建：自己/视图目标/Pawn/PlayerState/流式关卡）。
void UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	UTenkaichiReplicationGraph* TenkaichiGraph = CastChecked<UTenkaichiReplicationGraph>(GetOuter());

	ReplicationActorList.Reset();

	for (const FNetViewer& CurViewer : Params.Viewers)
	{
		ReplicationActorList.ConditionalAdd(CurViewer.InViewer);
		ReplicationActorList.ConditionalAdd(CurViewer.ViewTarget);

		if (ATenkaichiPlayerController* PC = Cast<ATenkaichiPlayerController>(CurViewer.InViewer))
		{
			// 对 PlayerState 做 50% 节流。
			const bool bReplicatePS = (Params.ConnectionManager.ConnectionOrderNum % 2) == (Params.ReplicationFrameNum % 2);
			if (bReplicatePS)
			{
				// 总是把玩家状态返回给拥有玩家。模拟代理 PlayerState 由 UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter 处理
				if (APlayerState* PS = PC->PlayerState)
				{
					if (!bInitializedPlayerState)
					{
						bInitializedPlayerState = true;
						FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(PS);
						ConnectionActorInfo.ReplicationPeriodFrame = 1;
					}

					ReplicationActorList.ConditionalAdd(PS);
				}
			}

			FCachedAlwaysRelevantActorInfo& LastData = PastRelevantActorMap.FindOrAdd(CurViewer.Connection);

			if (ATenkaichiCharacter* Pawn = Cast<ATenkaichiCharacter>(PC->GetPawn()))
			{
				UpdateCachedRelevantActor(Params, Pawn, LastData.LastViewer);

				if (Pawn != CurViewer.ViewTarget)
				{
					ReplicationActorList.ConditionalAdd(Pawn);
				}
			}

			if (ATenkaichiCharacter* ViewTargetPawn = Cast<ATenkaichiCharacter>(CurViewer.ViewTarget))
			{
				UpdateCachedRelevantActor(Params, ViewTargetPawn, LastData.LastViewTarget);
			}
		}
	}

	CleanupCachedRelevantActors(PastRelevantActorMap);

	// 始终相关的流式关卡 Actor。
	FPerConnectionActorInfoMap& ConnectionActorInfoMap = Params.ConnectionManager.ActorInfoMap;

	TMap<FName, FActorRepListRefView>& AlwaysRelevantStreamingLevelActors = TenkaichiGraph->AlwaysRelevantStreamingLevelActors;

	for (int32 Idx=AlwaysRelevantStreamingLevelsNeedingReplication.Num()-1; Idx >= 0; --Idx)
	{
		const FName& StreamingLevel = AlwaysRelevantStreamingLevelsNeedingReplication[Idx];

		FActorRepListRefView* Ptr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel);
		if (Ptr == nullptr)
		{
			// 那个关卡没有始终相关列表
			UE_CLOG(Tenkaichi::RepGraph::DisplayClientLevelStreaming > 0, LogTenkaichiRepGraph, Display, TEXT("CLIENTSTREAMING Removing %s from AlwaysRelevantStreamingLevelActors because FActorRepListRefView is null. %s "), *StreamingLevel.ToString(),  *Params.ConnectionManager.GetName());
			AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
			continue;
		}

		FActorRepListRefView& RepList = *Ptr;

		if (RepList.Num() > 0)
		{
			bool bAllDormant = true;
			for (FActorRepListType Actor : RepList)
			{
				FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);
				if (ConnectionActorInfo.bDormantOnConnection == false)
				{
					bAllDormant = false;
					break;
				}
			}

			if (bAllDormant)
			{
				UE_CLOG(Tenkaichi::RepGraph::DisplayClientLevelStreaming > 0, LogTenkaichiRepGraph, Display, TEXT("CLIENTSTREAMING All AlwaysRelevant Actors Dormant on StreamingLevel %s for %s. Removing list."), *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
				AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
			}
			else
			{
				UE_CLOG(Tenkaichi::RepGraph::DisplayClientLevelStreaming > 0, LogTenkaichiRepGraph, Display, TEXT("CLIENTSTREAMING Adding always Actors on StreamingLevel %s for %s because it has at least one non dormant actor"), *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
				Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
			}
		}
		else
		{
			UE_LOG(LogTenkaichiRepGraph, Warning, TEXT("UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection - empty RepList %s"), *Params.ConnectionManager.GetName());
		}

	}

#if WITH_GAMEPLAY_DEBUGGER
	if (GameplayDebugger)
	{
		ReplicationActorList.ConditionalAdd(GameplayDebugger);
	}
#endif

	Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorList);
}

// 客户端关卡可见性添加回调。
void UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld)
{
	UE_CLOG(Tenkaichi::RepGraph::DisplayClientLevelStreaming > 0, LogTenkaichiRepGraph, Display, TEXT("CLIENTSTREAMING ::OnClientLevelVisibilityAdd - %s"), *LevelName.ToString());
	AlwaysRelevantStreamingLevelsNeedingReplication.Add(LevelName);
}

// 客户端关卡可见性移除回调。
void UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove(FName LevelName)
{
	UE_CLOG(Tenkaichi::RepGraph::DisplayClientLevelStreaming > 0, LogTenkaichiRepGraph, Display, TEXT("CLIENTSTREAMING ::OnClientLevelVisibilityRemove - %s"), *LevelName.ToString());
	AlwaysRelevantStreamingLevelsNeedingReplication.Remove(LevelName);
}

// 打印节点日志。
void UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
{
	DebugInfo.Log(NodeName);
	DebugInfo.PushIndent();
	LogActorRepList(DebugInfo, NodeName, ReplicationActorList);

	for (const FName& LevelName : AlwaysRelevantStreamingLevelsNeedingReplication)
	{
		UTenkaichiReplicationGraph* TenkaichiGraph = CastChecked<UTenkaichiReplicationGraph>(GetOuter());
		if (FActorRepListRefView* RepList = TenkaichiGraph->AlwaysRelevantStreamingLevelActors.Find(LevelName))
		{
			LogActorRepList(DebugInfo, FString::Printf(TEXT("AlwaysRelevant StreamingLevel List: %s"), *LevelName.ToString()), *RepList);
		}
	}

	DebugInfo.PopIndent();
}