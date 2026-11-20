// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "TenkaichiInputConfig.h"

#include "TenkaichiInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;


/**
 * UTenkaichiInputComponent
 *
 *	用于通过输入配置数据资产来管理输入映射与绑定的组件。
 */
UCLASS(Config = Input)
class UTenkaichiInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer);

	// 把输入配置里的映射上下文登记进玩家的增强输入子系统
	void AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	// 把之前登记进去的映射上下文从增强输入子系统里移除
	void RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	// 按标签把一条「原生输入」绑定到回调函数上
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	// 遍历「技能输入」表，按下和抬起各绑一次，产生的绑定句柄存进 BindHandles 供后续解绑
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UTenkaichiInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	// 按句柄批量解绑，然后清空句柄数组
	void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
void UTenkaichiInputComponent::BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UTenkaichiInputComponent::BindAbilityActions(const UTenkaichiInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FTenkaichiInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}