#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"   // 第3小步会建，先引着

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAttributeSet)

UTenkaichiAttributeSet::UTenkaichiAttributeSet()
{
}

UWorld* UTenkaichiAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);
	return Outer->GetWorld();
}

UTenkaichiAbilitySystemComponent* UTenkaichiAttributeSet::GetTenkaichiAbilitySystemComponent() const
{
	return Cast<UTenkaichiAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}