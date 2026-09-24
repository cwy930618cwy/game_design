#pragma once

#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TenkaichiCharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class UTenkaichiAbilitySystemComponent;
class UTenkaichiHealthSet;

UCLASS()
class ATenkaichiCharacterWithAbilities : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	// 一比一还原 Lyra：实现 IAbilitySystemInterface 的纯虚函数，必须写 override
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	// 自带 ASC（对应 LyraCharacterWithAbilities.h 第 32-33 行）
	UPROPERTY(VisibleAnywhere, Category = "Tenkaichi|Character")
	TObjectPtr<UTenkaichiAbilitySystemComponent> AbilitySystemComponent;

	// 血量属性集（对应第 36-37 行；我们不含 CombatSet）
	UPROPERTY()
	TObjectPtr<UTenkaichiHealthSet> HealthSet;
};