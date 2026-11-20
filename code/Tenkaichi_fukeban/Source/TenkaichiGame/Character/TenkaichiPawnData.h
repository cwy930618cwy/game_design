// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "TenkaichiPawnData.generated.h"

#define UE_API TENKAICHIGAME_API

class APawn;
class UTenkaichiAbilitySet;
class UTenkaichiAbilityTagRelationshipMapping;
class UTenkaichiCameraMode;
class UTenkaichiInputConfig;
class UObject;


/**
 * UTenkaichiPawnData
 *
 *	不可变数据资产，用来定义"一个 Pawn 需要哪些配置"。
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Tenkaichi Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class UTenkaichiPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// 该 Pawn 实例化用的类（通常继承自 ATenkaichiPawn 或 ATenkaichiCharacter）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Pawn")
	TSubclassOf<APawn> PawnClass;

	// 授予该 Pawn 能力系统的一组能力集
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Abilities")
	TArray<TObjectPtr<UTenkaichiAbilitySet>> AbilitySets;

	// 该 Pawn 发起动作时使用的"能力标签关系映射"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Abilities")
	TObjectPtr<UTenkaichiAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 玩家控制的 Pawn 用这个输入配置来创建输入映射并绑定输入动作
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Input")
	TObjectPtr<UTenkaichiInputConfig> InputConfig;

	// 玩家控制的 Pawn 默认使用的相机模式
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tenkaichi|Camera")
	TSubclassOf<UTenkaichiCameraMode> DefaultCameraMode;
};

#undef UE_API