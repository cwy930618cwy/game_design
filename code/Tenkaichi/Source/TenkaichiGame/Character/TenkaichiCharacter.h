// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "Teams/TenkaichiTeamAgentInterface.h"

#include "TenkaichiCharacter.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class AController;
class ATenkaichiPlayerController;
class ATenkaichiPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiCameraComponent;
class UTenkaichiHealthComponent;
class UTenkaichiPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;

/**
 * FTenkaichiReplicatedAcceleration: 加速度的压缩表示（FastShared 网络优化用）
 */
USTRUCT()
struct FTenkaichiReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// XY 加速度的方向角，量化到 [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	// XY 加速度的大小，量化到 [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Z 轴原始加速度，量化到 [-MaxAcceleration, MaxAcceleration]
};

/** 我们用来发送 FastShared 移动更新的类型。 */
USTRUCT()
struct FTenkaichiSharedRepMovement
{
	GENERATED_BODY()

	FTenkaichiSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FTenkaichiSharedRepMovement& Other, ACharacter* Character) const;

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

template<>
struct TStructOpsTypeTraits<FTenkaichiSharedRepMovement> : public TStructOpsTypeTraitsBase2<FTenkaichiSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/**
 * ATenkaichiCharacter
 *
 *	本项目使用的基础角色 Pawn 类。
 *	负责向 Pawn 组件发送事件。
 *	新行为应尽可能通过 Pawn 组件添加。
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "本项目使用的基础角色 Pawn 类。"))
class ATenkaichiCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ITenkaichiTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API ATenkaichiCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Character")
	UE_API ATenkaichiPlayerController* GetTenkaichiPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Character")
	UE_API ATenkaichiPlayerState* GetTenkaichiPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Character")
	UE_API UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
	UE_API virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	UE_API void ToggleCrouch();
    //~AActor 接口开始
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~AActor 接口结束

	//~APawn 接口开始
	UE_API virtual void NotifyControllerChanged() override;
	//~APawn 接口结束

	//~ITenkaichiTeamAgentInterface 接口开始
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnTenkaichiTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~ITenkaichiTeamAgentInterface 接口结束

	/** 当默认属性复制被跳过的那几帧调用的 RPC。它向所有人复制一次移动更新。 */
	UFUNCTION(NetMulticast, unreliable)
	UE_API void FastSharedReplication(const FTenkaichiSharedRepMovement& SharedRepMovement);

	// 上次发送的 FSharedRepMovement，避免重复发送。
	FTenkaichiSharedRepMovement LastSharedReplication;

	UE_API virtual bool UpdateSharedReplication();

protected:

	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();

	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;

	UE_API virtual void OnRep_Controller() override;
	UE_API virtual void OnRep_PlayerState() override;

	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UE_API void InitializeGameplayTags();

	UE_API virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// 开始角色的死亡流程（关闭碰撞、关闭移动等）
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// 结束角色的死亡流程（断开控制器、销毁 Pawn 等）
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

	UE_API void DisableMovementAndCollision();
	UE_API void DestroyDueToDeath();
	UE_API void UninitAndDestroy();

	// 死亡流程完成后被调用
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	UE_API void K2_OnDeathFinished();

	UE_API virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	UE_API void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	UE_API virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	UE_API virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UE_API virtual bool CanJumpInternal_Implementation() const;
    
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tenkaichi|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTenkaichiPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tenkaichi|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTenkaichiHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tenkaichi|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTenkaichiCameraComponent> CameraComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FTenkaichiReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnTenkaichiTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	// 决定接管结束后队伍 ID 的去向
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// 这里可以改成：想释放后保留旧队就返回 OldTeamID，
		// 或者返回某个中立阵营的 ID 等等……
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	UE_API void OnRep_ReplicatedAcceleration();

	UFUNCTION()
	UE_API void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};

#undef UE_API