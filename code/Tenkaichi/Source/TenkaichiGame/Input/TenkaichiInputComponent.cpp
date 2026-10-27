// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/TenkaichiLocalPlayer.h"
// #include "Settings/TenkaichiSettingsLocal.h"   // ⚠️ 挂起：P18 Settings 未就位

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInputComponent)

class UTenkaichiInputConfig;

UTenkaichiInputComponent::UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UTenkaichiInputComponent::AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 如果你需要从输入配置里加点自定义的东西，就在这里处理
}

void UTenkaichiInputComponent::RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// 在这里处理上面可能添加过的输入映射的移除逻辑
}

void UTenkaichiInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}