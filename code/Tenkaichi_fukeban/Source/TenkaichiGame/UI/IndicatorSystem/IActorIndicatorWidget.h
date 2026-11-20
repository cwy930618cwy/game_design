// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"

#include "IActorIndicatorWidget.generated.h"

class AActor;
class UIndicatorDescriptor;

// 接口的 UObject 壳（蓝图可见）。
UINTERFACE(MinimalAPI, BlueprintType)
class UIndicatorWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

// 接口的 C++ 侧：能作为指示器显示的控件实现它。
class IIndicatorWidgetInterface
{
	GENERATED_BODY()

public:
	// 绑定到某条指示器时调用（蓝图可实现）。
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void BindIndicator(UIndicatorDescriptor* Indicator);

	// 从某条指示器解绑时调用（蓝图可实现）。
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void UnbindIndicator(const UIndicatorDescriptor* Indicator);
};