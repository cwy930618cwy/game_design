// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if 0	// ⚠️ 挂起：依赖 GameFeatureAction_AddInputContextMapping.h + GameplayAbilitySpecHandle.h（硬 include），GameFeatures + GAS 就位后放开

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "GameplayAbilitySpecHandle.h"
#include "TenkaichiHeroComponent.generated.h"

#define UE_API TENKAICHIGAME_API

namespace EEndPlayReason { enum Type : int; }
struct FLoadedMappableConfigPair;
struct FMappableConfigPair;

class UGameFrameworkComponentManager;
class UInputComponent;
class UTenkaichiCameraMode;
class UTenkaichiInputConfig;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;

/**
 * 负责为玩家控制的 Pawn（或模拟玩家的 bot）设置输入和相机处理的组件。
 * 它依赖 PawnExtensionComponent 来协调初始化。
 */
UCLASS(MinimalAPI, Blueprintable, Meta=(BlueprintSpawnableComponent))
class UTenkaichiHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiHeroComponent(const FObjectInitializer& ObjectInitializer);

	/** 如果指定 Actor 上有这个组件，则返回它。 */
	UFUNCTION(BlueprintPure, Category = "Tenkaichi|Hero")
	static UTenkaichiHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UTenkaichiHeroComponent>() : nullptr); }

	/** 用一个活动中的 gameplay ability 来覆盖相机模式 */
	UE_API void SetAbilityCameraMode(TSubclassOf<UTenkaichiCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** 如果覆盖已设置，则清除相机覆盖 */
	UE_API void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** 添加模式专属的输入配置 */
	UE_API void AddAdditionalInputConfig(const UTenkaichiInputConfig* InputConfig);

	/** 如果模式专属的输入配置已被添加，则移除它 */
	UE_API void RemoveAdditionalInputConfig(const UTenkaichiInputConfig* InputConfig);

	/** 如果这是由真实玩家控制，且初始化已推进到可以添加额外输入绑定的程度，则为 true */
	UE_API bool IsReadyToBindInputs() const;
	
	/** 通过 UGameFrameworkComponentManager 发送的"能力输入已可绑定"扩展事件的名字 */
	static UE_API const FName NAME_BindInputsNow;

	/** 本组件所实现特性的名字 */
	static UE_API const FName NAME_ActorFeatureName;

	//~IGameFrameworkInitStateInterface 接口开始
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~IGameFrameworkInitStateInterface 接口结束

protected:

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UE_API virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	UE_API void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	UE_API void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	UE_API void Input_Move(const FInputActionValue& InputActionValue);
	UE_API void Input_LookMouse(const FInputActionValue& InputActionValue);
	UE_API void Input_LookStick(const FInputActionValue& InputActionValue);
	UE_API void Input_Crouch(const FInputActionValue& InputActionValue);
	UE_API void Input_AutoRun(const FInputActionValue& InputActionValue);

	UE_API TSubclassOf<UTenkaichiCameraMode> DetermineCameraMode() const;

protected:
	
	UPROPERTY(EditAnywhere)
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	/** 由能力设置的相机模式。 */
	UPROPERTY()
	TSubclassOf<UTenkaichiCameraMode> AbilityCameraMode;

	/** 最后设置相机模式的那个能力的 Spec 句柄。 */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	/** 当玩家输入绑定已应用时为 true，对非玩家永远为 false */
	bool bReadyToBindInputs;
};

#undef UE_API

#endif	// ⚠️ GameFeatures（GameFeatureAction_AddInputContextMapping）+ GAS（GameplayAbilitySpecHandle）就位后放开