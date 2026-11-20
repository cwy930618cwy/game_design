// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "TenkaichiPawnExtensionComponent.generated.h"

#define UE_API TENKAICHIGAME_API

namespace EEndPlayReason { enum Type : int; }

class UGameFrameworkComponentManager;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiPawnData;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;

/**
 * 给所有 Pawn 类（角色/载具等）增加统一功能的组件，协调其他组件的初始化顺序
 */
UCLASS(MinimalAPI)
class UTenkaichiPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	/** 本特性的名字，其他命名组件特性都依赖它 */
	static UE_API const FName NAME_ActorFeatureName;

	//~IGameFrameworkInitStateInterface 接口开始
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~IGameFrameworkInitStateInterface 接口结束

	/** 如果指定 Actor 上有这个组件，则返回它 */
	UFUNCTION(BlueprintPure, Category = "Tenkaichi|Pawn")
	static UTenkaichiPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UTenkaichiPawnExtensionComponent>() : nullptr); }

	/** 获取角色数据（用来指定角色属性的数据资产） */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	/** 设置当前角色数据 */
	UE_API void SetPawnData(const UTenkaichiPawnData* InPawnData);

	/** 获取当前能力系统组件（可能归别的 Actor 所有） */
	UFUNCTION(BlueprintPure, Category = "Tenkaichi|Pawn")
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** 由所属 Pawn 调用：让这个 Pawn 成为能力系统的 avatar */
	UE_API void InitializeAbilitySystem(UTenkaichiAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** 由所属 Pawn 调用：让这个 Pawn 卸下能力系统 avatar 的身份 */
	UE_API void UninitializeAbilitySystem();

	/** 由所属 Pawn 在控制器改变时调用 */
	UE_API void HandleControllerChanged();

	/** 由所属 Pawn 在 PlayerState 被复制（同步）时调用 */
	UE_API void HandlePlayerStateReplicated();

	/** 由所属 Pawn 在输入组件设置好时调用 */
	UE_API void SetupPlayerInputComponent();

	/** 注册到"能力系统已初始化"委托；若 pawn 已注册过能力系统则立即广播 */
	UE_API void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** 注册到"能力系统已反初始化"委托（pawn 被卸下 avatar 身份时触发） */
	UE_API void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	UE_API void OnRep_PawnData();

	/** 当我们的 pawn 成为能力系统 avatar 时触发的委托 */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** 当我们的 pawn 被卸下能力系统 avatar 身份时触发的委托 */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	/** 创建 pawn 用的角色数据（来自生成函数或摆放实例） */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Tenkaichi|Pawn")
	TObjectPtr<const UTenkaichiPawnData> PawnData;

	/** 缓存的能力系统组件指针，方便使用 */
	UPROPERTY(Transient)
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;
};

#undef UE_API