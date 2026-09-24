#include "Character/TenkaichiCharacterWithAbilities.h"

#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiCharacterWithAbilities)

ATenkaichiCharacterWithAbilities::ATenkaichiCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UTenkaichiAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UTenkaichiHealthSet>(TEXT("HealthSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void ATenkaichiCharacterWithAbilities::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ATenkaichiCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}