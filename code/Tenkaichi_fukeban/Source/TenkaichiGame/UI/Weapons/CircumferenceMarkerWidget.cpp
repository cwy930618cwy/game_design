// 版权声明（可保留或省略，不影响编译）
#include "CircumferenceMarkerWidget.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CircumferenceMarkerWidget)

class SWidget;

// 构造函数：设默认可见性 + 易变标记。
UCircumferenceMarkerWidget::UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsVolatile = true;
}

// 释放 Slate 资源（控件销毁时重置指针）。
void UCircumferenceMarkerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyMarkerWidget.Reset();
}

// 重建 Slate 控件（创建 S 层实例）。
TSharedRef<SWidget> UCircumferenceMarkerWidget::RebuildWidget()
{
	MyMarkerWidget = SNew(SCircumferenceMarkerWidget)
		.MarkerBrush(&MarkerImage)
		.Radius(this->Radius)
		.MarkerList(this->MarkerList);

	return MyMarkerWidget.ToSharedRef();
}

// 属性同步（把参数转发给 Slate 控件）。
void UCircumferenceMarkerWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyMarkerWidget->SetRadius(Radius);
	MyMarkerWidget->SetMarkerList(MarkerList);
}

// 设置圆的半径。
void UCircumferenceMarkerWidget::SetRadius(float InRadius)
{
	Radius = InRadius;
	if (MyMarkerWidget.IsValid())
	{
		MyMarkerWidget->SetRadius(InRadius);
	}
}