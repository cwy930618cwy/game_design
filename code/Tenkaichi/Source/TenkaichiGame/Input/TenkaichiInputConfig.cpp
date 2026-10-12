// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInputConfig.h"

#include "TenkaichiLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputConfig)


UTenkaichiInputConfig::UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

// 根据标签，在「原生输入」里找出对应的输入动作；找不到时按 bLogNotFound 决定是否打印错误日志
const UInputAction* UTenkaichiInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FTenkaichiInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

// 根据标签，在「技能输入」里找出对应的输入动作；找不到时按 bLogNotFound 决定是否打印错误日志
const UInputAction* UTenkaichiInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FTenkaichiInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}