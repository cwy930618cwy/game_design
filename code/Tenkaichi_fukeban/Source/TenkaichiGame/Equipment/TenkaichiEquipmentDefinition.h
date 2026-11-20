// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"

#include "TenkaichiEquipmentDefinition.generated.h"

class AActor;
class UTenkaichiAbilitySet;
class UTenkaichiEquipmentInstance;

// 装备要生成的 Actor 描述：生成什么、挂哪个插槽、什么偏移
USTRUCT()
struct FTenkaichiEquipmentActorToSpawn
{
	GENERATED_BODY()

	// 默认构造（空）
	FTenkaichiEquipmentActorToSpawn()
	{}

	// 要生成的 Actor 类
	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	// 挂到角色身上的哪个插槽（如 "hand_r"）
	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	// 挂载的偏移（位置/旋转/缩放）
	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UTenkaichiEquipmentDefinition
 *
 * 一件"可以穿到角色身上"的装备的定义
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UTenkaichiEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数（ObjectInitializer 有默认值，可无参构造）
	UTenkaichiEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 用什么实例类来承载这件装备（默认 UTenkaichiEquipmentInstance）
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UTenkaichiEquipmentInstance> InstanceType;

	// 穿上这件装备时要授予的技能集（GAS AbilitySet）
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UTenkaichiAbilitySet>> AbilitySetsToGrant;

	// 穿上时要在角色身上生成的 Actor（如武器模型）
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FTenkaichiEquipmentActorToSpawn> ActorsToSpawn;
};