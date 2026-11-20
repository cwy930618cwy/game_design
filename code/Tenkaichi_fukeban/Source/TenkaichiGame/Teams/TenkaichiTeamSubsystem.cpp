// Copyright Epic Games, Inc. All Rights Reserved.

#include "Teams/TenkaichiTeamSubsystem.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TenkaichiLogChannels.h"
#include "TenkaichiTeamAgentInterface.h"
#include "TenkaichiTeamCheats.h"
#include "TenkaichiTeamPrivateInfo.h"
#include "TenkaichiTeamPublicInfo.h"
#include "Player/TenkaichiPlayerState.h"
#include "Teams/TenkaichiTeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTeamSubsystem)

class FSubsystemCollectionBase;

//////////////////////////////////////////////////////////////////////
// FTenkaichiTeamTrackingInfo

// 把队伍信息挂到这个追踪记录上（按类型分发到 Public/Private）
void FTenkaichiTeamTrackingInfo::SetTeamInfo(ATenkaichiTeamInfoBase* Info)
{
	// 如果是公开信息：记录 PublicInfo，并把它的外观资产同步过来
	if (ATenkaichiTeamPublicInfo* NewPublicInfo = Cast<ATenkaichiTeamPublicInfo>(Info))
	{
		ensure((PublicInfo == nullptr) || (PublicInfo == NewPublicInfo));
		PublicInfo = NewPublicInfo;

		UTenkaichiTeamDisplayAsset* OldDisplayAsset = DisplayAsset;
		DisplayAsset = NewPublicInfo->GetTeamDisplayAsset();

		// 外观变了就广播通知所有观察者
		if (OldDisplayAsset != DisplayAsset)
		{
			OnTeamDisplayAssetChanged.Broadcast(DisplayAsset);
		}
	}
	// 如果是私有信息：记录 PrivateInfo
	else if (ATenkaichiTeamPrivateInfo* NewPrivateInfo = Cast<ATenkaichiTeamPrivateInfo>(Info))
	{
		ensure((PrivateInfo == nullptr) || (PrivateInfo == NewPrivateInfo));
		PrivateInfo = NewPrivateInfo;
	}
	// 既不是公开也不是私有 → 报错
	else
	{
		checkf(false, TEXT("Expected a public or private team info but got %s"), *GetPathNameSafe(Info))
	}
}

// 从这个追踪记录移除队伍信息
void FTenkaichiTeamTrackingInfo::RemoveTeamInfo(ATenkaichiTeamInfoBase* Info)
{
	// 移除公开或私有信息，都不匹配就报错
	if (PublicInfo == Info)
	{
		PublicInfo = nullptr;
	}
	else if (PrivateInfo == Info)
	{
		PrivateInfo = nullptr;
	}
	else
	{
		ensureMsgf(false, TEXT("Expected a previously registered team info but got %s"), *GetPathNameSafe(Info));
	}
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiTeamSubsystem

UTenkaichiTeamSubsystem::UTenkaichiTeamSubsystem()
{
}

// 子系统初始化：注册队伍作弊控制台扩展
void UTenkaichiTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// 每创建一个作弊管理器，就给队伍加一个作弊扩展
	auto AddTeamCheats = [](UCheatManager* CheatManager)
	{
		CheatManager->AddCheatManagerExtension(NewObject<UTenkaichiTeamCheats>(CheatManager));
	};

	// 注册"作弊管理器已创建"的委托
	CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(AddTeamCheats));
}

// 子系统反初始化：注销作弊管理器委托
void UTenkaichiTeamSubsystem::Deinitialize()
{
	UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);

	Super::Deinitialize();
}

// 尝试注册一个新队伍：按队号登记进 TeamMap
bool UTenkaichiTeamSubsystem::RegisterTeamInfo(ATenkaichiTeamInfoBase* TeamInfo)
{
	if (!ensure(TeamInfo))
	{
		return false;
	}

	const int32 TeamId = TeamInfo->GetTeamId();
	if (ensure(TeamId != INDEX_NONE))
	{
		// 按队号找到（或新建）追踪记录，把队伍信息挂上去
		FTenkaichiTeamTrackingInfo& Entry = TeamMap.FindOrAdd(TeamId);
		Entry.SetTeamInfo(TeamInfo);

		return true;
	}

	return false;
}

// 尝试注销一个队伍：从 TeamMap 移除，失败返回 false
bool UTenkaichiTeamSubsystem::UnregisterTeamInfo(ATenkaichiTeamInfoBase* TeamInfo)
{
	if (!ensure(TeamInfo))
	{
		return false;
	}

	const int32 TeamId = TeamInfo->GetTeamId();
	if (ensure(TeamId != INDEX_NONE))
	{
		FTenkaichiTeamTrackingInfo* Entry = TeamMap.Find(TeamId);

		// 如果找不到该队，可能是上个世界残留的 Actor，忽略它
		if (Entry)
		{
			Entry->RemoveTeamInfo(TeamInfo);

			return true;
		}
	}

	return false;
}

// 尽可能改变与这个 Actor 关联的队伍（只能权威端调用）
bool UTenkaichiTeamSubsystem::ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamIndex)
{
	// 先尝试走 PlayerState（有玩家状态就用它改队）
	const FGenericTeamId NewTeamID = IntegerToGenericTeamId(NewTeamIndex);
	if (ATenkaichiPlayerState* TenkaichiPS = const_cast<ATenkaichiPlayerState*>(FindPlayerStateFromActor(ActorToChange)))
	{
		TenkaichiPS->SetGenericTeamId(NewTeamID);
		return true;
	}
	// 否则走队伍接口（实现 ITenkaichiTeamAgentInterface 的 Actor）
	else if (ITenkaichiTeamAgentInterface* TeamActor = Cast<ITenkaichiTeamAgentInterface>(ActorToChange))
	{
		TeamActor->SetGenericTeamId(NewTeamID);
		return true;
	}
	else
	{
		return false;
	}
}

// 返回这个对象所属的队伍，如果不是任何队伍则返回 INDEX_NONE
int32 UTenkaichiTeamSubsystem::FindTeamFromObject(const UObject* TestObject) const
{
	// 情况 1：对象本身实现了队伍接口
	if (const ITenkaichiTeamAgentInterface* ObjectWithTeamInterface = Cast<ITenkaichiTeamAgentInterface>(TestObject))
	{
		return GenericTeamIdToInteger(ObjectWithTeamInterface->GetGenericTeamId());
	}

	// 情况 2：对象是 Actor，看它是不是队伍相关
	if (const AActor* TestActor = Cast<const AActor>(TestObject))
	{
		// 情况 2a：Actor 的 instigator（发起者）实现了队伍接口
		if (const ITenkaichiTeamAgentInterface* InstigatorWithTeamInterface = Cast<ITenkaichiTeamAgentInterface>(TestActor->GetInstigator()))
		{
			return GenericTeamIdToInteger(InstigatorWithTeamInterface->GetGenericTeamId());
		}

		// 情况 2b：队伍信息 Actor 没有队伍接口，特殊处理——直接返回队号
		if (const ATenkaichiTeamInfoBase* TeamInfo = Cast<ATenkaichiTeamInfoBase>(TestActor))
		{
			return TeamInfo->GetTeamId();
		}

		// 情况 2c：回退到找关联的 PlayerState
		if (const ATenkaichiPlayerState* TenkaichiPS = FindPlayerStateFromActor(TestActor))
		{
			return TenkaichiPS->GetTeamId();
		}
	}

	return INDEX_NONE;
}

// 返回与这个 Actor 关联的 PlayerState，没关联玩家则返回 nullptr
const ATenkaichiPlayerState* UTenkaichiTeamSubsystem::FindPlayerStateFromActor(const AActor* PossibleTeamActor) const
{
	if (PossibleTeamActor != nullptr)
	{
		// 是 Pawn：从 Pawn 拿 PlayerState
		if (const APawn* Pawn = Cast<const APawn>(PossibleTeamActor))
		{
			//@TODO: 考虑用接口，或让队伍 Actor 在子系统里注册并维护一个 Map
			if (ATenkaichiPlayerState* TenkaichiPS = Pawn->GetPlayerState<ATenkaichiPlayerState>())
			{
				return TenkaichiPS;
			}
		}
		// 是 Controller：从 Controller 的 PlayerState 拿
		else if (const AController* PC = Cast<const AController>(PossibleTeamActor))
		{
			if (ATenkaichiPlayerState* TenkaichiPS = Cast<ATenkaichiPlayerState>(PC->PlayerState))
			{
				return TenkaichiPS;
			}
		}
		// 本身就是 PlayerState
		else if (const ATenkaichiPlayerState* TenkaichiPS = Cast<const ATenkaichiPlayerState>(PossibleTeamActor))
		{
			return TenkaichiPS;
		}

		// 尝试用 instigator（被 Lyra 注释掉了，保留原样）
// 		if (AActor* Instigator = PossibleTeamActor->GetInstigator())
// 		{
// 			if (ensure(Instigator != PossibleTeamActor))
// 			{
// 				return FindPlayerStateFromActor(Instigator);
// 			}
// 		}
	}

	return nullptr;
}

// 比较两个 Actor 的队伍，返回同队/异队/无效（带输出参数）
ETenkaichiTeamComparison UTenkaichiTeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const
{
	TeamIdA = FindTeamFromObject(Cast<const AActor>(A));
	TeamIdB = FindTeamFromObject(Cast<const AActor>(B));

	// 任何一个不是队伍成员 → 无效
	if ((TeamIdA == INDEX_NONE) || (TeamIdB == INDEX_NONE))
	{
		return ETenkaichiTeamComparison::InvalidArgument;
	}
	else
	{
		// 队号相同则同队，否则异队
		return (TeamIdA == TeamIdB) ? ETenkaichiTeamComparison::OnSameTeam : ETenkaichiTeamComparison::DifferentTeams;
	}
}

// 比较两个 Actor 的队伍（不带输出参数的版本）
ETenkaichiTeamComparison UTenkaichiTeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
	int32 TeamIdA;
	int32 TeamIdB;
	return CompareTeams(A, B, /*out*/ TeamIdA, /*out*/ TeamIdB);
}

// 蓝图版查询：返回是否属于队伍 + 队号
void UTenkaichiTeamSubsystem::FindTeamFromActor(const UObject* TestObject, bool& bIsPartOfTeam, int32& TeamId) const
{
	TeamId = FindTeamFromObject(TestObject);
	bIsPartOfTeam = TeamId != INDEX_NONE;
}

// 给队伍标签增加层数（只能在权威端调用）
void UTenkaichiTeamSubsystem::AddTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
	// 失败时的统一日志处理
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogTenkaichiTeams, Error, TEXT("AddTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
	};

	if (FTenkaichiTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (Entry->PublicInfo)
		{
			// 只在权威端（服务器）才能改队伍标签
			if (Entry->PublicInfo->HasAuthority())
			{
				Entry->PublicInfo->TeamTags.AddStack(Tag, StackCount);
			}
			else
			{
				FailureHandler(TEXT("failed because it was called on a client"));
			}
		}
		else
		{
			FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
		}
	}
	else
	{
		FailureHandler(TEXT("failed because it was passed an unknown team id"));
	}
}

// 从队伍标签移除层数（只能在权威端调用）
void UTenkaichiTeamSubsystem::RemoveTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount)
{
	// 失败时的统一日志处理
	auto FailureHandler = [&](const FString& ErrorMessage)
	{
		UE_LOG(LogTenkaichiTeams, Error, TEXT("RemoveTeamTagStack(TeamId: %d, Tag: %s, StackCount: %d) %s"), TeamId, *Tag.ToString(), StackCount, *ErrorMessage);
	};

	if (FTenkaichiTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		if (Entry->PublicInfo)
		{
			if (Entry->PublicInfo->HasAuthority())
			{
				Entry->PublicInfo->TeamTags.RemoveStack(Tag, StackCount);
			}
			else
			{
				FailureHandler(TEXT("failed because it was called on a client"));
			}
		}
		else
		{
			FailureHandler(TEXT("failed because there is no team info spawned yet (called too early, before the experience was ready)"));
		}
	}
	else
	{
		FailureHandler(TEXT("failed because it was passed an unknown team id"));
	}
}

// 返回指定标签的总层数（公开+私有栈的和，0 表示没有）
int32 UTenkaichiTeamSubsystem::GetTeamTagStackCount(int32 TeamId, FGameplayTag Tag) const
{
	if (const FTenkaichiTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		const int32 PublicStackCount = (Entry->PublicInfo != nullptr) ? Entry->PublicInfo->TeamTags.GetStackCount(Tag) : 0;
		const int32 PrivateStackCount = (Entry->PrivateInfo != nullptr) ? Entry->PrivateInfo->TeamTags.GetStackCount(Tag) : 0;
		return PublicStackCount + PrivateStackCount;
	}
	else
	{
		UE_LOG(LogTenkaichiTeams, Verbose, TEXT("GetTeamTagStackCount(TeamId: %d, Tag: %s) failed because it was passed an unknown team id"), TeamId, *Tag.ToString());
		return 0;
	}
}

// 返回指定标签是否至少有一层
bool UTenkaichiTeamSubsystem::TeamHasTag(int32 TeamId, FGameplayTag Tag) const
{
	return GetTeamTagStackCount(TeamId, Tag) > 0;
}

// 返回指定队伍是否存在
bool UTenkaichiTeamSubsystem::DoesTeamExist(int32 TeamId) const
{
	return TeamMap.Contains(TeamId);
}

// 返回所有队伍的队号列表（排序后）
TArray<int32> UTenkaichiTeamSubsystem::GetTeamIDs() const
{
	TArray<int32> Result;
	TeamMap.GenerateKeyArray(Result);
	Result.Sort();
	return Result;
}

// 根据友军伤害设置，返回 instigator 是否能伤害 target
bool UTenkaichiTeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf) const
{
	// 允许伤害自己：两个对象相同，或它们的 PlayerState 相同 → 允许
	if (bAllowDamageToSelf)
	{
		if ((Instigator == Target) || (FindPlayerStateFromActor(Cast<AActor>(Instigator)) == FindPlayerStateFromActor(Cast<AActor>(Target))))
		{
			return true;
		}
	}

	int32 InstigatorTeamId;
	int32 TargetTeamId;
	const ETenkaichiTeamComparison Relationship = CompareTeams(Instigator, Target, /*out*/ InstigatorTeamId, /*out*/ TargetTeamId);
	// 异队 → 可以伤害
	if (Relationship == ETenkaichiTeamComparison::DifferentTeams)
	{
		return true;
	}
	// 无效参数但 instigator 有队号：暂时允许伤害无队伍的 Actor，只要它有能力系统组件
	else if ((Relationship == ETenkaichiTeamComparison::InvalidArgument) && (InstigatorTeamId != INDEX_NONE))
	{
		//@TODO: 这是临时的，直到训练假人有队伍归属
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<const AActor>(Target)) != nullptr;
	}

	return false;
}

// 获取指定队伍的外观资产（从"观众队"视角，当前忽略视角）
UTenkaichiTeamDisplayAsset* UTenkaichiTeamSubsystem::GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId)
{
	// 当前忽略 ViewerTeamId

	if (FTenkaichiTeamTrackingInfo* Entry = TeamMap.Find(TeamId))
	{
		return Entry->DisplayAsset;
	}

	return nullptr;
}

// 获取指定队伍的外观资产（从"观众 Agent"视角）
UTenkaichiTeamDisplayAsset* UTenkaichiTeamSubsystem::GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent)
{
	return GetTeamDisplayAsset(TeamId, FindTeamFromObject(ViewerTeamAgent));
}

// 当队伍外观资产被编辑时调用，让所有队伍颜色观察者刷新
void UTenkaichiTeamSubsystem::NotifyTeamDisplayAssetModified(UTenkaichiTeamDisplayAsset* /*ModifiedAsset*/)
{
	// 现在改成编辑时广播给所有观察者，而不是只给被编辑的那一个
	for (const auto& KVP : TeamMap)
	{
		const int32 TeamId = KVP.Key;
		const FTenkaichiTeamTrackingInfo& TrackingInfo = KVP.Value;

		TrackingInfo.OnTeamDisplayAssetChanged.Broadcast(TrackingInfo.DisplayAsset);
	}
}

// 为指定队伍 ID 注册一个"队伍外观改变"通知委托
FOnTenkaichiTeamDisplayAssetChangedDelegate& UTenkaichiTeamSubsystem::GetTeamDisplayAssetChangedDelegate(int32 TeamId)
{
	return TeamMap.FindOrAdd(TeamId).OnTeamDisplayAssetChanged;
}

