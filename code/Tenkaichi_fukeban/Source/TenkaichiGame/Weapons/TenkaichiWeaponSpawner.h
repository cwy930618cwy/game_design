// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "TenkaichiWeaponSpawner.generated.h"

#define UE_API TENKAICHIGAME_API

namespace EEndPlayReason { enum Type : int; }

class APawn;
class UCapsuleComponent;
class UTenkaichiInventoryItemDefinition;
class UTenkaichiWeaponPickupDefinition;
class UObject;
class UPrimitiveComponent;
class UStaticMeshComponent;
class FFrame;
struct FGameplayTag;
struct FHitResult;

UCLASS(MinimalAPI, Blueprintable,BlueprintType)
class ATenkaichiWeaponSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// 设置这个 Actor 的默认属性
	UE_API ATenkaichiWeaponSpawner();

protected:
	// 游戏开始或生成时调用
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// 每一帧调用
	UE_API virtual void Tick(float DeltaTime) override;

	UE_API void OnConstruction(const FTransform& Transform) override;

protected:
	// 用来配置武器生成器的数据资产
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	TObjectPtr<UTenkaichiWeaponPickupDefinition> WeaponDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_WeaponAvailability, Category = "Lyra|WeaponPickup")
	bool bIsWeaponAvailable;

	// 从武器被捡到重新生成之间隔的时间（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	float CoolDownTime;

	// 武器重新可用后，延迟检测"是否有人正站在刷新点上"的时间。
	// 用来给 bIsWeaponAvailable 的 OnRep 留出触发时间，先播完特效再检测。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	float CheckExistingOverlapDelay;

	// 用来驱动"武器重生倒计时"显示条（0~1）
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Lyra|WeaponPickup")
	float CoolDownPercentage;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	TObjectPtr<UCapsuleComponent> CollisionVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	TObjectPtr<UStaticMeshComponent> PadMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Lyra|WeaponPickup")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lyra|WeaponPickup")
	float WeaponMeshRotationSpeed;

	FTimerHandle CoolDownTimerHandle;

	FTimerHandle CheckOverlapsDelayTimerHandle;

	UFUNCTION()
	UE_API void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	// 枪重新生成时，检测是否有人正站在刷新点上
	UE_API void CheckForExistingOverlaps();

	UFUNCTION(BlueprintNativeEvent)
	UE_API void AttemptPickUpWeapon(APawn* Pawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lyra|WeaponPickup")
	UE_API bool GiveWeapon(TSubclassOf<UTenkaichiInventoryItemDefinition> WeaponItemClass, APawn* ReceivingPawn);

	UE_API void StartCoolDown();

	UFUNCTION(BlueprintCallable, Category = "Lyra|WeaponPickup")
	UE_API void ResetCoolDown();

	UFUNCTION()
	UE_API void OnCoolDownTimerComplete();

	UE_API void SetWeaponPickupVisibility(bool bShouldBeVisible);

	UFUNCTION(BlueprintNativeEvent, Category = "Lyra|WeaponPickup")
	UE_API void PlayPickupEffects();

	UFUNCTION(BlueprintNativeEvent, Category = "Lyra|WeaponPickup")
	UE_API void PlayRespawnEffects();

	UFUNCTION()
	UE_API void OnRep_WeaponAvailability();

	/** 在某个物品定义类型里查找匹配的属性并返回它的值，找不到就返回 0 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lyra|WeaponPickup")
	static UE_API int32 GetDefaultStatFromItemDef(const TSubclassOf<UTenkaichiInventoryItemDefinition> WeaponItemClass, FGameplayTag StatTag);
};

#undef UE_API