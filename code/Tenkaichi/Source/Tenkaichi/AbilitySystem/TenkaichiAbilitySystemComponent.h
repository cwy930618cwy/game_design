#pragma once

#include "AbilitySystemComponent.h"

#include "TenkaichiAbilitySystemComponent.generated.h"

UCLASS(MinimalAPI)
class UTenkaichiAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UTenkaichiAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};