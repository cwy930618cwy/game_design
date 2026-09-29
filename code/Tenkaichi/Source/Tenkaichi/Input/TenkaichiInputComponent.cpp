#include "TenkaichiInputComponent.h"

#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputComponent)

class UTenkaichiInputConfig;

UTenkaichiInputComponent::UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UTenkaichiInputComponent::AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 如果需要，可以在这里处理「从输入配置里添加映射」的自定义逻辑（Lyra 这里留空）
}

void UTenkaichiInputComponent::RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 如果需要，可以在这里处理「移除上面添加的映射」的自定义逻辑（Lyra 这里留空）
}

void UTenkaichiInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}