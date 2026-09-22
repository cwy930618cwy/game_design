#pragma once

#include "AttributeSet.h"
#include "TenkaichiAttributeSet.generated.h"

// 前向声明（对应 Lyra LyraAttributeSet.h 第 11-15 行）：
// .h 里只用到这两个类型的指针做返回值，不需要完整定义，前向声明即可，避免循环 include
class UWorld;
class UTenkaichiAbilitySystemComponent;

// 便利宏：每个子类都要用它生成访问函数，所以放在基类里
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UTenkaichiAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTenkaichiAttributeSet();

	// 两个工具函数声明（下一文件 .cpp 里实现，这里必须先声明）
	UWorld* GetWorld() const override;
	UTenkaichiAbilitySystemComponent* GetTenkaichiAbilitySystemComponent() const;
};