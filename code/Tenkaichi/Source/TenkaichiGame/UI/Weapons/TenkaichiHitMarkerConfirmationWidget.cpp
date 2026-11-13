// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiHitMarkerConfirmationWidget.h"

#include "Blueprint/UserWidget.h"
#include "SHitMarkerConfirmationWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHitMarkerConfirmationWidget)

class SWidget;

// 构造函数：设默认可见性 + 易变标记 + 默认不画"任意命中"图。
UTenkaichiHitMarkerConfirmationWidget::UTenkaichiHitMarkerConfirmationWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsVolatile = true;
	AnyHitsMarkerImage.DrawAs = ESlateBrushDrawType::NoDrawType;
}

// 释放 Slate 资源。
void UTenkaichiHitMarkerConfirmationWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyMarkerWidget.Reset();
}

// 重建 Slate 控件（创建 S 层实例，并传自定义参数）。
TSharedRef<SWidget> UTenkaichiHitMarkerConfirmationWidget::RebuildWidget()
{
	UUserWidget* OuterUserWidget = GetTypedOuter<UUserWidget>();
	FLocalPlayerContext DummyContext;
	const FLocalPlayerContext& PlayerContextRef = (OuterUserWidget != nullptr) ? OuterUserWidget->GetPlayerContext() : DummyContext;

	MyMarkerWidget = SNew(SHitMarkerConfirmationWidget, PlayerContextRef, PerHitMarkerZoneOverrideImages)
		.PerHitMarkerImage(&(this->PerHitMarkerImage))
		.AnyHitsMarkerImage(&(this->AnyHitsMarkerImage))
		.HitNotifyDuration(this->HitNotifyDuration);

	return MyMarkerWidget.ToSharedRef();
}