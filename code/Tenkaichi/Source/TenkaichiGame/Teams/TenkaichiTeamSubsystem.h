// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "TenkaichiTeamSubsystem.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class ATenkaichiPlayerState;
class ATenkaichiTeamInfoBase;
class ATenkaichiTeamPrivateInfo;
class ATenkaichiTeamPublicInfo;
class FSubsystemCollectionBase;
class UTenkaichiTeamDisplayAsset;
struct FFrame;
struct FGameplayTag;

// 队伍外观改变时通知所有观察者（参数：新的外观资产）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTenkaichiTeamDisplayAssetChangedDelegate, const UTenkaichiTeamDisplayAsset*, DisplayAsset);

// 单个队伍在 TeamSubsystem 里追踪的信息
USTRUCT()
struct FTenkaichiTeamTrackingInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<ATenkaichiTeamPublicInfo> PublicInfo = nullptr;

	UPROPERTY()
	TObjectPtr<ATenkaichiTeamPrivateInfo> PrivateInfo = nullptr;

	UPROPERTY()
	TObjectPtr<UTenkaichiTeamDisplayAsset> DisplayAsset = nullptr;

	UPROPERTY()
	FOnTenkaichiTeamDisplayAssetChangedDelegate OnTeamDisplayAssetChanged;

public:
	// 把队伍信息挂到这个追踪记录上（按类型分发到 Public/Private）
	void SetTeamInfo(ATenkaichiTeamInfoBase* Info);
	// 从这个追踪记录移除队伍信息
	void RemoveTeamInfo(ATenkaichiTeamInfoBase* Info);
};

// 比较两个 Actor 队伍归属的结果
UENUM(BlueprintType)
enum class ETenkaichiTeamComparison : uint8
{
	// 两个 Actor 属于同一队
	OnSameTeam,

	// 两个 Actor 属于敌对队伍
	DifferentTeams,

	// 一个（或两个）Actor 无效，或不属于任何队伍
	InvalidArgument
};

/** 一个方便访问队伍信息的子系统（给基于队伍的角色 Actor，如 Pawn 或 PlayerState 用） */
UCLASS(MinimalAPI)
class UTenkaichiTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UTenkaichiTeamSubsystem();

	//~USubsystem 接口
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	//~USubsystem 接口结束

	// 尝试注册一个新队伍
	UE_API bool RegisterTeamInfo(ATenkaichiTeamInfoBase* TeamInfo);

	// 尝试注销一个队伍，注销失败会返回 false
	UE_API bool UnregisterTeamInfo(ATenkaichiTeamInfoBase* TeamInfo);

	// 尽可能改变与这个 Actor 关联的队伍
	// 注意：这个函数只能在权威端（服务器）调用
	UE_API bool ChangeTeamForActor(AActor* ActorToChange, int32 NewTeamId);

	// 返回这个对象所属的队伍，如果不是任何队伍则返回 INDEX_NONE
	UE_API int32 FindTeamFromObject(const UObject* TestObject) const;

	// 返回与这个 Actor 关联的 PlayerState，如果没关联玩家则返回 nullptr
	UE_API const ATenkaichiPlayerState* FindPlayerStateFromActor(const AActor* PossibleTeamActor) const;

	// 返回这个对象所属的队伍，如果不是任何队伍则返回 INDEX_NONE
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Teams, meta=(Keywords="Get"))
	UE_API void FindTeamFromActor(const UObject* TestActor, bool& bIsPartOfTeam, int32& TeamId) const;

	// 比较两个 Actor 的队伍，返回他们同队/异队/一个或两个无效
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Teams, meta=(ExpandEnumAsExecs=ReturnValue))
	UE_API ETenkaichiTeamComparison CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const;

	// 比较两个 Actor 的队伍（不带输出参数的版本）
	UE_API ETenkaichiTeamComparison CompareTeams(const UObject* A, const UObject* B) const;

	// 根据友军伤害设置，返回 instigator 是否能伤害 target
	UE_API bool CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf = true) const;

	// 给标签增加指定数量的层数（StackCount 小于 1 时什么都不做）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void AddTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount);

	// 从标签移除指定数量的层数（StackCount 小于 1 时什么都不做）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void RemoveTeamTagStack(int32 TeamId, FGameplayTag Tag, int32 StackCount);

	// 返回指定标签的层数（如果标签不存在则返回 0）
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API int32 GetTeamTagStackCount(int32 TeamId, FGameplayTag Tag) const;

	// 返回指定标签是否至少有一层
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API bool TeamHasTag(int32 TeamId, FGameplayTag Tag) const;

	// 返回指定队伍是否存在
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API bool DoesTeamExist(int32 TeamId) const;

	// 获取指定队伍的外观资产，从指定"观众队"的视角
	// （必须指定观众，以防游戏模式是"本地玩家永远是蓝队"这种情况）
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API UTenkaichiTeamDisplayAsset* GetTeamDisplayAsset(int32 TeamId, int32 ViewerTeamId);

	// 获取指定队伍的外观资产，从指定"观众 Agent"的视角
	UFUNCTION(BlueprintCallable, Category = Teams)
	UE_API UTenkaichiTeamDisplayAsset* GetEffectiveTeamDisplayAsset(int32 TeamId, UObject* ViewerTeamAgent);

	// 获取所有队伍的列表
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Teams)
	UE_API TArray<int32> GetTeamIDs() const;

	// 当队伍外观资产被编辑时调用，让所有队伍颜色观察者刷新
	UE_API void NotifyTeamDisplayAssetModified(UTenkaichiTeamDisplayAsset* ModifiedAsset);

	// 为指定队伍 ID 注册一个"队伍外观改变"通知委托
	UE_API FOnTenkaichiTeamDisplayAssetChangedDelegate& GetTeamDisplayAssetChangedDelegate(int32 TeamId);

private:
	UPROPERTY()
	TMap<int32, FTenkaichiTeamTrackingInfo> TeamMap;

	FDelegateHandle CheatManagerRegistrationHandle;
};

#undef UE_API