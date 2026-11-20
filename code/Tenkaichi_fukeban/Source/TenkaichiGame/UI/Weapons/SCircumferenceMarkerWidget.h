// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Styling/CoreStyle.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

#include "SCircumferenceMarkerWidget.generated.h"

class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
struct FGeometry;
struct FSlateBrush;

// 一个圆周标记：位置角度 + 图片旋转角度。
USTRUCT(BlueprintType)
struct FCircumferenceMarkerEntry
{
	GENERATED_BODY()

	// 这个标记在圆周上的位置角度（度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float PositionAngle = 0.0f;

	// 标记图片的旋转角度（度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float ImageRotationAngle = 0.0f;
};

// Slate 控件：在圆周上绘制多个标记。
class SCircumferenceMarkerWidget : public SLeafWidget
{
	// Slate 控件的"参数声明"块（类似构造函数参数表）。
	SLATE_BEGIN_ARGS(SCircumferenceMarkerWidget)
		: _MarkerBrush(FCoreStyle::Get().GetBrush("Throbber.CircleChunk"))
		, _Radius(48.0f)
	{
	}
		/** 圆周上每个标记的样子 */
		SLATE_ARGUMENT(const FSlateBrush*, MarkerBrush)
		/** 在哪些角度画标记 */
		SLATE_ARGUMENT(TArray<FCircumferenceMarkerEntry>, MarkerList)
		/** 圆的半径 */
		SLATE_ATTRIBUTE(float, Radius)
		/** 标记的颜色和不透明度 */
		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)
	SLATE_END_ARGS()

public:
	// 构造（Slate 控件的初始化入口）。
	void Construct(const FArguments& InArgs);

	// 构造函数（默认构造，供 Slate 新建控件用）。
	SCircumferenceMarkerWidget();

	//~SWidget 接口开始
	// 绘制：在圆周上画所有标记。
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// 期望尺寸：控件要多大的布局空间。
	virtual FVector2D ComputeDesiredSize(float) const override;
	// 是否易变（true = 每帧都可能变，强制重绘）。
	virtual bool ComputeVolatility() const override { return true; }
	//~SWidget 接口结束

	// 设置半径。
	void SetRadius(float NewRadius);
	// 设置标记列表。
	void SetMarkerList(TArray<FCircumferenceMarkerEntry>& NewMarkerList);

private:
	// 算某个标记的渲染变换（旋转 + 平移到圆周位置）。
	FSlateRenderTransform GetMarkerRenderTransform(const FCircumferenceMarkerEntry& Marker, const float BaseRadius, const float HUDScale) const;

private:
	/** 圆周上每个标记的样子 */
	const FSlateBrush* MarkerBrush;

	/** 用来在准星中心周围放置角标图标的角度列表 */
	TArray<FCircumferenceMarkerEntry> MarkerList;

	/** 圆的半径 */
	TAttribute<float> Radius;

	/** 标记的颜色和不透明度 */
	TAttribute<FSlateColor> ColorAndOpacity;
	bool bColorAndOpacitySet;

	/** 准星角标是否放在散布半径之外 */
	//@TODO: 把它改成一个 0~1 的浮点对齐值？(比如：在内/在圆上/在外)
	uint8 bReticleCornerOutsideSpreadRadius : 1;
};