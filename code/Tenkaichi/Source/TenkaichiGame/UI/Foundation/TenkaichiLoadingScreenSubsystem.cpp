// 版权声明（可保留或省略，不影响编译）
#include "UI/Foundation/TenkaichiLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiLoadingScreenSubsystem)

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// UTenkaichiLoadingScreenSubsystem

// 构造函数：空实现。
UTenkaichiLoadingScreenSubsystem::UTenkaichiLoadingScreenSubsystem()
{
}

// 设置加载屏内容控件类（变了才广播）。
void UTenkaichiLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

// 返回上次设置的加载屏内容控件类。
TSubclassOf<UUserWidget> UTenkaichiLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}