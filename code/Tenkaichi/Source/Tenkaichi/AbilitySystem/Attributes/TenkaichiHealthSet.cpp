#include "AbilitySystem/Attributes/TenkaichiHealthSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHealthSet)

UTenkaichiHealthSet::UTenkaichiHealthSet()
	: Health(100.0f)   // ① 构造函数设初值
{
}

void UTenkaichiHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ② 登记网络复制
	DOREPLIFETIME_CONDITION_NOTIFY(UTenkaichiHealthSet, Health, COND_None, REPNOTIFY_Always);
}

void UTenkaichiHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	// ③ 这一句必须！通知 GAS 属性已复制
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTenkaichiHealthSet, Health, OldValue);
}