// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "IndicatorLibrary.generated.h"

#define UE_API TENKAICHIGAME_API

class AController;
class UTenkaichiIndicatorManagerComponent;
class UObject;
struct FFrame;

// 指示器蓝图函数库：给蓝图提供取管理器的入口。
UCLASS(MinimalAPI)
class UIndicatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 构造函数（空实现）。
	UE_API UIndicatorLibrary();
	
	/** 从控制器上取指示器管理器组件（蓝图可调）。 */
	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UE_API UTenkaichiIndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);
};

#undef UE_API