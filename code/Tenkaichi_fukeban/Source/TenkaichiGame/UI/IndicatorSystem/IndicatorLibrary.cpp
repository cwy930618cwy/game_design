// 版权声明（可保留或省略，不影响编译）
#include "IndicatorLibrary.h"

#include "TenkaichiIndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorLibrary)

class AController;

// 构造函数（空实现）。
UIndicatorLibrary::UIndicatorLibrary()
{
}

// 从控制器上取指示器管理器组件（蓝图可调）。
UTenkaichiIndicatorManagerComponent* UIndicatorLibrary::GetIndicatorManagerComponent(AController* Controller)
{
	return UTenkaichiIndicatorManagerComponent::GetComponent(Controller);
}