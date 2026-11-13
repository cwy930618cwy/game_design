// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiWidgetFactory.h"
#include "Templates/SubclassOf.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWidgetFactory)

class UUserWidget;

// C++ 默认实现：返回空（由子类覆写）。
TSubclassOf<UUserWidget> UTenkaichiWidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}