// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiTabButtonBase.h"

#include "CommonLazyImage.h"
#include "UI/Common/TenkaichiTabListWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTabButtonBase)

class UObject;
struct FSlateBrush;

// 从懒加载对象设置图标（软引用，按需加载）。
void UTenkaichiTabButtonBase::SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

// 直接设置图标画刷。
void UTenkaichiTabButtonBase::SetIconBrush(const FSlateBrush& Brush)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrush(Brush);
	}
}

// 实现接口：接收标签描述（填文字 + 图标）。
void UTenkaichiTabButtonBase::SetTabLabelInfo_Implementation(const FTenkaichiTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}