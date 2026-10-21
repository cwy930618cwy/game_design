// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiPlayerState.generated.h"

#define UE_API TENKAICHIGAME_API

struct FTenkaichiVerbMessage;

class AController;
class ATenkaichiPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiExperienceDefinition;
class UTenkaichiPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/** 定义连进来的客户端类型 */
UENUM()
enum class ELyraPlayerConnectionType : uint8
{
	// 正常在玩的玩家
	Player = 0,

	// 连进正在运行游戏的旁观者
	LiveSpectator,

	// 离线旁观（播放 demo 录像）
	ReplaySpectator,

	// 已断开连接的离线玩家
	InactivePlayer
};

/**
 * ATenkaichiPlayerState
 *
 *	本工程使用的玩家档案基类。
 */
UCLASS(MinimalAPI, Config = Game)
class ATenkaichiPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:
	UE_API ATenkaichiPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取玩家控制器（ATenkaichiPlayerController），用于访问玩家侧的输入/相机等。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerState")
	UE_API ATenkaichiPlayerController* GetTenkaichiPlayerController() const;

	// 获取玩家身上的技能系统组件（ASC）。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|PlayerState")
	UE_API UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const { return AbilitySystemComponent; }
	// 实现 IAbilitySystemInterface 接口：返回 ASC（供 GAS 系统内部调用）。
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 泛型获取 PawnData：调用方指定类型 T 来转型。
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 设置 PawnData（服务器权威，设置后授予能力集）。
	UE_API void SetPawnData(const UTenkaichiPawnData* InPawnData);

	//~AActor 接口开始
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void PostInitializeComponents() override;
	//~AActor 接口结束

	//~APlayerState 接口开始
	UE_API virtual void Reset() override;
	UE_API virtual void ClientInitialize(AController* C) override;
	UE_API virtual void CopyProperties(APlayerState* PlayerState) override;
	UE_API virtual void OnDeactivated() override;
	UE_API virtual void OnReactivated() override;
	//~APlayerState 接口结束

	//~ITenkaichiTeamAgentInterface 接口开始
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~ITenkaichiTeamAgentInterface 接口结束

	// 能力系统就绪事件名（静态常量）。
	static UE_API const FName NAME_TenkaichiAbilityReady;

	// 设置玩家连接类型。
	UE_API void SetPlayerConnectionType(ELyraPlayerConnectionType NewType);
	// 获取玩家连接类型（内联）。
	ELyraPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	// 返回玩家所属小队的 ID。
	UFUNCTION(BlueprintCallable)
	int32 GetSquadId() const
	{
		return MySquadID;
	}

	// 返回玩家所属队伍的 ID。
	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const
	{
		return GenericTeamIdToInteger(MyTeamID);
	}

	// 设置玩家所属小队 ID。
	UE_API void SetSquadID(int32 NewSquadID);

	// 给指定标签加上 N 层（只在服务器上有权限调用）。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 给指定标签减掉 N 层（只在服务器上有权限调用）。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Teams)
	UE_API void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 查询指定标签的层数（标签不在则返回 0）。
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// 判断指定标签是否至少叠了 1 层。
	UFUNCTION(BlueprintCallable, Category=Teams)
	UE_API bool HasStatTag(FGameplayTag Tag) const;

	// 给单个玩家发一条消息（只用于客户端通知，如成就/任务提示这类可容忍偶尔丢失的）。
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Tenkaichi|PlayerState")
	UE_API void ClientBroadcastMessage(const FTenkaichiVerbMessage Message);

	// 获取这个玩家的复制观察旋转（用于旁观/看录像）。
	UE_API FRotator GetReplicatedViewRotation() const;

	// 设置复制观察旋转（只在服务器上有效）。
	UE_API void SetReplicatedViewRotation(const FRotator& NewRotation);

private:
	// 经验（Experience）加载完成后回调：初始化 PawnData。
	UE_API void OnExperienceLoaded(const UTenkaichiExperienceDefinition* CurrentExperience);

protected:
	// PawnData 被复制到客户端后触发（网络同步）。
	UFUNCTION()
	UE_API void OnRep_PawnData();

protected:

	// PawnData 被复制到客户端后触发（网络同步）。
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UTenkaichiPawnData> PawnData;

private:

	// 玩家角色用的技能系统组件（子对象）。
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|PlayerState")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 生命值属性集（GAS 断点类，先挂着）。
	UPROPERTY()
	TObjectPtr<const class UTenkaichiHealthSet> HealthSet;
	// 战斗属性集（GAS 断点类，先挂着）。
	UPROPERTY()
	TObjectPtr<const class UTenkaichiCombatSet> CombatSet;

	// 玩家连接类型（复制到所有客户端）。
	UPROPERTY(Replicated)
	ELyraPlayerConnectionType MyPlayerConnectionType;

	// 队伍变更委托实例。
	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 玩家所属队伍（复制，变更时触发 OnRep_MyTeamID）。
	UPROPERTY(ReplicatedUsing=OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// 玩家所属小队（复制，变更时触发 OnRep_MySquadID）。
	UPROPERTY(ReplicatedUsing=OnRep_MySquadID)
	int32 MySquadID;

	// 统计标签容器（复制）。
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	// 复制的观察旋转。
	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;

private:
	// 队伍被复制到客户端后触发。
	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	// 小队被复制到客户端后触发。
	UFUNCTION()
	UE_API void OnRep_MySquadID();
};

#undef UE_API