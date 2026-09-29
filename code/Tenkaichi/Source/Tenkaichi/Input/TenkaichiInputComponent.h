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
 *	用「输入配置数据资产」来管理输入映射与绑定的组件。
 */
UCLASS(Config = Input)
class UTenkaichiInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UTenkaichiInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UTenkaichiInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

// ===== 模板函数：因为含模板，必须把实现写在 .h 里（不能放 .cpp） =====
template<class UserClass, typename FuncType>
void UTenkaichiInputComponent::BindNativeAction(const UTenkaichiInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}