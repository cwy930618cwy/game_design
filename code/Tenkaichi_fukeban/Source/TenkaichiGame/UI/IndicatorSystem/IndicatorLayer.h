// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/Widget.h"

#include "IndicatorLayer.generated.h"

class SActorCanvas;
class SWidget;
class UObject;

// 指示器层的 UMG 包装（内部是 SActorCanvas）。
UCLASS()
class UIndicatorLayer : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** 当 UI 被钳制到屏幕边缘、需要显示箭头时用的默认箭头画刷。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush ArrowBrush;

protected:
	//~UWidget 接口开始
	// 释放 Slate 资源。
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// 重建 Slate 控件（创建 SActorCanvas）。
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~UWidget 接口结束

protected:
	// 内部 Slate 画布。
	TSharedPtr<SActorCanvas> MyActorCanvas;
};