// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "TenkaichiPickupDefinition.generated.h"

class UTenkaichiInventoryItemDefinition;
class UNiagaraSystem;
class UObject;
class USoundBase;
class UStaticMesh;

/**
 * 拾取物定义：配置"地上的战利品"是什么、长什么样、什么音效特效
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const, Meta = (DisplayName = "Tenkaichi Pickup Data", ShortTooltip = "用于配置一个拾取物的数据资产。"))
class UTenkaichiPickupDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	// 捡起来得到什么物品（P08 的物品定义）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup|Equipment")
	TSubclassOf<UTenkaichiInventoryItemDefinition> InventoryItemDefinition;

	// 地上显示的模型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup|Mesh")
	TObjectPtr<UStaticMesh> DisplayMesh;

	// 拾取物重生冷却时间（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup")
	int32 SpawnCoolDownSeconds;

	// 捡起时播放的音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup")
	TObjectPtr<USoundBase> PickedUpSound;

	// 重生时播放的音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup")
	TObjectPtr<USoundBase> RespawnedSound;

	// 捡起时播放的特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup")
	TObjectPtr<UNiagaraSystem> PickedUpEffect;

	// 重生时播放的特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup")
	TObjectPtr<UNiagaraSystem> RespawnedEffect;
};


/**
 * 武器拾取物定义：在基础拾取定义上，多加武器模型的偏移和缩放
 */
UCLASS(MinimalAPI, Blueprintable, BlueprintType, Const, Meta = (DisplayName = "Tenkaichi Weapon Pickup Data", ShortTooltip = "用于配置一个武器拾取物的数据资产。"))
class UTenkaichiWeaponPickupDefinition : public UTenkaichiPickupDefinition
{
	GENERATED_BODY()

public:

	// 武器显示模型相对生成点的高度偏移
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup|Mesh")
	FVector WeaponMeshOffset;

	// 武器显示模型的缩放（默认 1,1,1）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pickup|Mesh")
	FVector WeaponMeshScale = FVector(1.0f, 1.0f, 1.0f);
};