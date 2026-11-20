// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/World.h"

#include "TenkaichiEquipmentInstance.generated.h"

class AActor;
class APawn;
struct FFrame;
struct FTenkaichiEquipmentActorToSpawn;

/**
 * UTenkaichiEquipmentInstance
 *
 * 一件"生成并穿到角色身上"的装备
 */
UCLASS(BlueprintType, Blueprintable)
class UTenkaichiEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数
	UTenkaichiEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject 接口
	// 支持网络复制（返回 true）
	virtual bool IsSupportedForNetworking() const override { return true; }
	// 取世界：返回穿戴 Pawn 的世界
	virtual UWorld* GetWorld() const override final;
	//~UObject 接口结束

	// 取穿戴者（谁穿的这件装备）
	UFUNCTION(BlueprintPure, Category=Equipment)
	UObject* GetInstigator() const { return Instigator; }

	// 设置穿戴者
	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	// 取穿戴的 Pawn
	UFUNCTION(BlueprintPure, Category=Equipment)
	APawn* GetPawn() const;

	// 取穿戴的 Pawn（带类型检查，返回指定类型或空）
	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	// 取生成的 Actor 列表
	UFUNCTION(BlueprintPure, Category=Equipment)
	TArray<AActor*> GetSpawnedActors() const { return SpawnedActors; }

	// 按定义生成装备 Actor（武器模型等），挂到角色身上
	virtual void SpawnEquipmentActors(const TArray<FTenkaichiEquipmentActorToSpawn>& ActorsToSpawn);
	// 销毁生成的装备 Actor
	virtual void DestroyEquipmentActors();

	// 装备时的钩子（可覆写）
	virtual void OnEquipped();
	// 卸下时的钩子（可覆写）
	virtual void OnUnequipped();

protected:
#if UE_WITH_IRIS
	// 注册所有复制片段（Iris 网络复制系统）
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	// 蓝图可覆写事件：装备时触发
	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnEquipped"))
	void K2_OnEquipped();

	// 蓝图可覆写事件：卸下时触发
	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnUnequipped"))
	void K2_OnUnequipped();

private:
	// Instigator 复制回调
	UFUNCTION()
	void OnRep_Instigator();

private:
	// 穿戴者（谁穿的），复制时触发 OnRep_Instigator
	UPROPERTY(ReplicatedUsing=OnRep_Instigator)
	TObjectPtr<UObject> Instigator;

	// 生成的 Actor 列表（武器模型等），随网络复制
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AActor>> SpawnedActors;
};