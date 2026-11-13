// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiIndicatorManagerComponent.h"

#include "IndicatorDescriptor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiIndicatorManagerComponent)

// 构造函数：自动注册 + 自动激活。
UTenkaichiIndicatorManagerComponent::UTenkaichiIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegister = true;
	bAutoActivate = true;
}

// 从控制器上找本组件（便捷静态方法）。
/*static*/ UTenkaichiIndicatorManagerComponent* UTenkaichiIndicatorManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<UTenkaichiIndicatorManagerComponent>();
	}

	return nullptr;
}

// 添加一条指示器。
void UTenkaichiIndicatorManagerComponent::AddIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	IndicatorDescriptor->SetIndicatorManagerComponent(this);
	OnIndicatorAdded.Broadcast(IndicatorDescriptor);
	Indicators.Add(IndicatorDescriptor);
}

// 移除一条指示器。
void UTenkaichiIndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	if (IndicatorDescriptor)
	{
		ensure(IndicatorDescriptor->GetIndicatorManagerComponent() == this);
	
		OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
		Indicators.Remove(IndicatorDescriptor);
	}
}