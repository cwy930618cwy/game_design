// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "ReplicationGraph.h"
#include "TenkaichiReplicationGraphTypes.h"
#include "TenkaichiReplicationGraph.generated.h"

class AGameplayDebuggerCategoryReplicator;

// 声明复制图日志通道（Display 级别，All 详细度）。
DECLARE_LOG_CATEGORY_EXTERN(LogTenkaichiRepGraph, Display, All);

// Tenkaichi 复制图实现。更多说明见 TenkaichiReplicationGraph.cpp！
UCLASS(transient, config=Engine)
class UTenkaichiReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	// 构造函数。
	UTenkaichiReplicationGraph();

	// 重置游戏世界状态。
	virtual void ResetGameWorldState() override;

	// 初始化全局 Actor 类设置。
	virtual void InitGlobalActorClassSettings() override;
	// 初始化全局图节点。
	virtual void InitGlobalGraphNodes() override;
	// 初始化连接的图节点。
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	// 把网络 Actor 路由到节点。
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	// 把网络 Actor 从节点移除。
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	// 始终相关（AlwaysRelevant）的类列表。
	UPROPERTY()
	TArray<TObjectPtr<UClass>>	AlwaysRelevantClasses;

	// 空间化网格节点。
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	// 始终相关 Actor 列表节点。
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	// 始终相关的流式关卡 Actor（按关卡名分组）。
	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

#if WITH_GAMEPLAY_DEBUGGER
	// 调试器拥有者变化回调。
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	// 打印复制节点策略。
	void PrintRepNodePolicies();

private:
	// 添加类的复制信息。
	void AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping);
	// 注册类的复制节点映射。
	void RegisterClassRepNodeMapping(UClass* Class);
	// 获取类的节点映射。
	EClassRepNodeMapping GetClassNodeMapping(UClass* Class) const;

	// 注册类的复制信息。
	void RegisterClassReplicationInfo(UClass* Class);
	// 有条件地初始化类的复制信息。
	bool ConditionalInitClassReplicationInfo(UClass* Class, FClassReplicationInfo& ClassInfo);
	// 初始化类的复制信息。
	void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool Spatialize) const;

	// 获取类的映射策略。
	EClassRepNodeMapping GetMappingPolicy(UClass* Class);

	// 是否空间化（映射值 >= Spatialize_Static）。
	bool IsSpatialized(EClassRepNodeMapping Mapping) const { return Mapping >= EClassRepNodeMapping::Spatialize_Static; }

	// 类 → 节点映射策略表。
	TClassMap<EClassRepNodeMapping> ClassRepNodePolicies;

	// 在 UTenkaichiReplicationGraph::InitGlobalActorClassSettings 里显式设置了复制设置的类。
	TArray<UClass*> ExplicitlySetClasses;
};

// 连接级"始终相关"节点。
UCLASS()
class UTenkaichiReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode_AlwaysRelevant_ForConnection
{
	GENERATED_BODY()

public:
	// 通知添加网络 Actor（空实现）。
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	// 通知移除网络 Actor（返回 false）。
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }
	// 通知重置所有网络 Actor（空实现）。
	virtual void NotifyResetAllNetworkActors() override { }

	// 为连接收集 Actor 列表。
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// 打印节点日志。
	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	// 客户端关卡可见性添加回调。
	void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);
	// 客户端关卡可见性移除回调。
	void OnClientLevelVisibilityRemove(FName LevelName);

	// 重置游戏世界状态。
	void ResetGameWorldState();

#if WITH_GAMEPLAY_DEBUGGER
	// 玩法调试器。
	AGameplayDebuggerCategoryReplicator* GameplayDebugger = nullptr;
#endif

private:
	// 需要复制的始终相关流式关卡。
	TArray<FName, TInlineAllocator<64> > AlwaysRelevantStreamingLevelsNeedingReplication;

	// 是否已初始化玩家状态。
	bool bInitializedPlayerState = false;
};

// 这是一个专门用于处理 PlayerState 复制的节点，以频率限制的方式。
// 它追踪所有玩家状态，但每帧只返回一个子集给复制驱动。
// 这是针对大量玩家连接数的优化，不是必须的。
UCLASS()
class UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter : public UReplicationGraphNode
{
	GENERATED_BODY()

	// 构造函数（私有）。
	UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter();

	// 通知添加网络 Actor（空实现）。
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override { }
	// 通知移除网络 Actor（返回 false）。
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound=true) override { return false; }
	// 通知 Actor 重命名（返回 false）。
	virtual bool NotifyActorRenamed(const FRenamedReplicatedActorInfo& Actor, bool bWarnIfNotFound=true) override { return false; }

	// 为连接收集 Actor 列表。
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// 准备复制。
	virtual void PrepareForReplication() override;

	// 打印节点日志。
	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	// 每帧想返回给复制驱动的 Actor 数量。不会抑制 ForceNetUpdate。
	int32 TargetActorsPerFrame = 2;

private:

	// 复制 Actor 列表。
	TArray<FActorRepListRefView> ReplicationActorLists;
	// 强制网络更新的复制 Actor 列表。
	FActorRepListRefView ForceNetUpdateReplicationActorList;
};