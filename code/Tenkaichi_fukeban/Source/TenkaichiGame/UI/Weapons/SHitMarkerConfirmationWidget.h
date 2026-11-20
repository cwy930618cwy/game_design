// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/LocalPlayer.h"
#include "GameplayTagContainer.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/SLeafWidget.h"

class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
struct FGameplayTag;
struct FGeometry;
struct FSlateBrush;

// 命中标记 Slate 控件：按命中位置绘制标记 + 淡出动画。
class SHitMarkerConfirmationWidget : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SHitMarkerConfirmationWidget)
		: _PerHitMarkerImage(FCoreStyle::Get().GetBrush("Throbber.CircleChunk"))
		, _AnyHitsMarkerImage(nullptr)
		, _HitNotifyDuration(0.4f)
	{
	}
		/** 单次命中要绘制的标记图片 */
		SLATE_ARGUMENT(const FSlateBrush*, PerHitMarkerImage)
		/** 只要有命中就绘制的标记图片（屏幕中心） */
		SLATE_ARGUMENT(const FSlateBrush*, AnyHitsMarkerImage)
		/** 命中通知的显示时长（秒），期间淡出到透明 */
		SLATE_ATTRIBUTE(float, HitNotifyDuration)
		/** 标记的颜色和不透明度 */
		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)
	SLATE_END_ARGS()

public:
	// Construct 多两个自定义参数（本地玩家上下文 + 部位覆盖图）。
	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext, const TMap<FGameplayTag, FSlateBrush>& ZoneOverrideImages);

	// 默认构造。
	SHitMarkerConfirmationWidget();

	//~SWidget 接口开始
	// 绘制：按命中位置画标记。
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// 每帧更新淡出透明度。
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// 期望尺寸。
	virtual FVector2D ComputeDesiredSize(float) const override;
	// 易变（每帧重绘）。
	virtual bool ComputeVolatility() const override { return true; }
	//~SWidget 接口结束

private:
	/** 单次命中要绘制的标记图片 */
	const FSlateBrush* PerHitMarkerImage = nullptr;

	/** 部位 tag（比如弱点）→ 覆盖用标记图片 的映射 */
	TMap<FGameplayTag, FSlateBrush> PerHitMarkerZoneOverrideImages;

	/** 只要有命中就绘制的标记图片 */
	const FSlateBrush* AnyHitsMarkerImage = nullptr;

	/** 命中标记的不透明度 */
	float HitNotifyOpacity = 0.0f;

	/** 命中通知的显示时长（秒），期间淡出到透明 */
	float HitNotifyDuration = 0.4f;

	/** 标记的颜色和不透明度 */
	TAttribute<FSlateColor> ColorAndOpacity;
	bool bColorAndOpacitySet;

	/** 拥有 HUD 的玩家上下文 */
	FLocalPlayerContext MyContext;
};