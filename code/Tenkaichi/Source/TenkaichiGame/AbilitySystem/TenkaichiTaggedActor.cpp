// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiTaggedActor.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTaggedActor)

// 构造函数：仅把对象初始化器转发给父类，无额外初始化
ATenkaichiTaggedActor::ATenkaichiTaggedActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 把本 Actor 静态配置的 GameplayTag 追加进调用方传入的容器
void ATenkaichiTaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(StaticGameplayTags);
}

#if WITH_EDITOR
// 判断指定属性能否在编辑器细节面板里被修改
bool ATenkaichiTaggedActor::CanEditChange(const FProperty* InProperty) const
{
	// 禁止编辑另一个标签属性（引擎 Actor 自带的 FName 数组 Tags）
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AActor, Tags))
	{
		return false;
	}

	return Super::CanEditChange(InProperty);
}
#endif