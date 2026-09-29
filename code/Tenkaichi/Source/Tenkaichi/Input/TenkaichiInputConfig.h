#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "TenkaichiInputConfig.generated.h"

class UInputAction;
class UObject;
struct FFrame;

/**
 * FTenkaichiInputAction
 *
 *	用于把「一个输入动作（InputAction）」映射到「一个 GameplayTag」的结构体。
 */
USTRUCT(BlueprintType)
struct FTenkaichiInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * UTenkaichiInputConfig
 *
 *	不可变的数据资产，用来存放输入配置。
 */
UCLASS(BlueprintType, Const)
class UTenkaichiInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	// 宿主使用的输入动作列表。这些输入动作映射到 GameplayTag，需要手动绑定。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTenkaichiInputAction> NativeInputActions;
};