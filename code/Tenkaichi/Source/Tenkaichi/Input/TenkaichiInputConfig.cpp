#include "TenkaichiInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputConfig)


UTenkaichiInputConfig::UTenkaichiInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UTenkaichiInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// 遍历「原生输入动作」列表，逐个比对 Tag
	for (const FTenkaichiInputAction& Action : NativeInputActions)
	{
		// 找到匹配的条目（动作存在 且 Tag 相等），就返回它对应的 InputAction
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	// 没找到时，如果允许打日志，就报一条错误，方便排查
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}