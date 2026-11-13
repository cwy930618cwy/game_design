// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/ControllerComponent.h"

#include "TenkaichiIndicatorManagerComponent.generated.h"

#define UE_API TENKAICHIGAME_API

class AController;
class UIndicatorDescriptor;
class UObject;
struct FFrame;

/**
 * @class UTenkaichiIndicatorManagerComponent
 */
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UTenkaichiIndicatorManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	// 构造函数：自动注册 + 自动激活。
	UE_API UTenkaichiIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	// 从控制器上找本组件（便捷静态方法）。
	static UE_API UTenkaichiIndicatorManagerComponent* GetComponent(AController* Controller);

	// 添加一条指示器。
	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void AddIndicator(UIndicatorDescriptor* IndicatorDescriptor);
	
	// 移除一条指示器。
	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor);

	// 声明"指示器增删"的 C++ 事件类型（带一个 Descriptor 参数）。
	DECLARE_EVENT_OneParam(UTenkaichiIndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptor* Descriptor)
	// 指示器添加时广播。
	FIndicatorEvent OnIndicatorAdded;
	// 指示器移除时广播。
	FIndicatorEvent OnIndicatorRemoved;

	// 返回所有指示器（只读）。
	const TArray<UIndicatorDescriptor*>& GetIndicators() const { return Indicators; }

private:
	// 所有指示器（UObject 数组，随本组件存续）。
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;
};

#undef UE_API