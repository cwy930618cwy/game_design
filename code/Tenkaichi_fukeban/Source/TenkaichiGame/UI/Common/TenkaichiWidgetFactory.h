// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "UObject/Object.h"

#include "TenkaichiWidgetFactory.generated.h"

#define UE_API TENKAICHIGAME_API

template <class TClass> class TSubclassOf;

class UUserWidget;
struct FFrame;

// 控件工厂接口：按数据找到对应的控件类（抽象基类）。
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType, EditInlineNew)
class UTenkaichiWidgetFactory : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UTenkaichiWidgetFactory() { }

	// 找到这条数据对应的控件类（蓝图原生事件，可覆写）。
	UFUNCTION(BlueprintNativeEvent)
	UE_API TSubclassOf<UUserWidget> FindWidgetClassForData(const UObject* Data) const;
};

#undef UE_API