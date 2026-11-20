// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "ReplicationGraphTypes.h"
#include "TenkaichiReplicationGraphTypes.generated.h"

// 这是我们用来把 Actor 路由到正确复制节点的主枚举。每个类映射到一个枚举值。
UENUM()
enum class EClassRepNodeMapping : uint32
{
	NotRouted,						// 不映射到任何节点。用于由特殊节点（UTenkaichiReplicationGraphNode_PlayerStateFrequencyLimiter）处理的特殊情况 Actor。
	RelevantAllConnections,			// 路由到 AlwaysRelevantNode 或 AlwaysRelevantStreamingLevelNode 节点。

	// 下面这些是"空间化"枚举！见 UTenkaichiReplicationGraph::IsSpatialized

	Spatialize_Static,				// 路由到 GridNode：这些 Actor 不移动，不需要每帧更新。
	Spatialize_Dynamic,				// 路由到 GridNode：这些 Actor 频繁移动，每帧更新一次。
	Spatialize_Dormancy,			// 路由到 GridNode：休眠时当静态处理，唤醒/非休眠时当动态处理。注意这是给"非休眠时会移动"的东西用的。
};

// Actor 类设置，可以直接赋给一个类，也可以映射到 FRepGraphActorTemplateSettings。
USTRUCT()
struct FRepGraphActorClassSettings
{
	GENERATED_BODY()

	// 默认构造。
	FRepGraphActorClassSettings() = default;

	// 这个设置要应用的类的名字。
	UPROPERTY(EditAnywhere)
	FSoftClassPath ActorClass;

	// 是否把这个类的 RepInfo 加入 ClassRepNodePolicies Map。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddClassRepInfoToMap  = true;

	// 把类加入 ClassRepNodePolicies Map 时，用哪个 ClassNodeMapping。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddClassRepInfoToMap"))
	EClassRepNodeMapping ClassNodeMapping = EClassRepNodeMapping::NotRouted;

	// 是否把这个加入 RPC_Multicast_OpenChannelForClass map。
	UPROPERTY(EditAnywhere, meta = (InlineEditConditionToggle))
	bool bAddToRPC_Multicast_OpenChannelForClassMap = false;

	// 如果加入了 RPC_Multicast_OpenChannelForClass map，是否真的打开一个通道。
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bAddToRPC_Multicast_OpenChannelForClassMap"))
	bool bRPC_Multicast_OpenChannelForClass = true;

	// 获取静态 Actor 类（从 FSoftClassPath 解析出 UClass*）。
	UClass* GetStaticActorClass() const
	{
		UClass* StaticActorClass = nullptr;
		const FString ActorClassNameString = ActorClass.ToString();

		if (FPackageName::IsScriptPackage(ActorClassNameString))
		{
			StaticActorClass = FindObject<UClass>(nullptr, *ActorClassNameString, true);

			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Find Static Class for %s"), *ActorClassNameString);
			}
		}
		else
		{
			// 允许蓝图用于自定义类设置
			StaticActorClass = (UClass*)StaticLoadObject(UClass::StaticClass(), nullptr, *ActorClassNameString);
			if (!StaticActorClass)
			{
				UE_LOG(LogTemp, Error, TEXT("FRepGraphActorClassSettings: Cannot Load Static Class for %s"), *ActorClassNameString);
			}
		}

		return StaticActorClass;
	}
};