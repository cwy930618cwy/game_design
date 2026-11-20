// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"

#include "TenkaichiTaggedActor.generated.h"

// 实现了 GameplayTag 资产接口的 Actor
UCLASS()
class ATenkaichiTaggedActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	// 构造函数：仅转发对象初始化器，无额外初始化
	ATenkaichiTaggedActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~IGameplayTagAssetInterface 接口开始
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~IGameplayTagAssetInterface 接口结束

	//~UObject 接口开始
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~UObject 接口结束

protected:
	// 与本 Actor 关联的、游戏相关的标签
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	FGameplayTagContainer StaticGameplayTags;
};