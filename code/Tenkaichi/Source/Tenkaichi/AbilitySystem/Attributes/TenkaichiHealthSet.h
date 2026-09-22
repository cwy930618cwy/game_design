#pragma once

#include "AbilitySystem/Attributes/TenkaichiAttributeSet.h"
#include "TenkaichiHealthSet.generated.h"

UCLASS()
class UTenkaichiHealthSet : public UTenkaichiAttributeSet
{
	GENERATED_BODY()

public:
	UTenkaichiHealthSet();

	// 用基类里的宏生成 GetHealth/GetHealthAttribute/SetHealth/InitHealth
	ATTRIBUTE_ACCESSORS(UTenkaichiHealthSet, Health);

protected:
	// 网络复制回调声明
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

private:
	// 属性本体
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Lyra|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;
};