// 构造（Slate 初始化入口：存参数）。
void STenkaichiLatencyGraph::Construct(const FArguments& InArgs)
{
	DesiredSize = InArgs._DesiredSize;
	MaxYAxisOfGraph = InArgs._MaxLatencyToGraph;
	LineColor = InArgs._LineColor;
	BackgroundColor = InArgs._BackgroundColor;
}

// 绘制：先画背景，再画折线。
int32 STenkaichiLatencyGraph::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MaxLayerId = LayerId;

	// 画背景
	FSlateDrawElement::MakeRotatedBox(
		OutDrawElements,
		MaxLayerId,
		AllottedGeometry.ToPaintGeometry(),
		FCoreStyle::Get().GetBrush("BlackBrush"),
		ESlateDrawEffect::NoPixelSnapping,
		0,
		TOptional<FVector2D>(),
		FSlateDrawElement::RelativeToElement,
		BackgroundColor);

	// 我们需要把曲线图真正画在背景上面
	// 所以把层级加一
	MaxLayerId++;

	// 真正画曲线图
	DrawTotalLatency(AllottedGeometry, OutDrawElements, MaxLayerId);
	
	MaxLayerId++;

	return MaxLayerId;
}

// 期望尺寸。
FVector2D STenkaichiLatencyGraph::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return DesiredSize;
}

#if 0
// ⚠️ 注释挂起：依赖 P18 FSampledStatCache
// 画总延迟折线。
void STenkaichiLatencyGraph::DrawTotalLatency(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	if (!GraphData)
	{
		return;
	}
	
	static TArray<FVector2D> Points;
	Points.Reset(GraphData->GetSampleSize() + 1);
	
	const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();
	const float LineThickness = 1.0f;
	const double XSlice = WidgetSize.X / static_cast<double>(GraphData->GetSampleSize());
	const double Border = 1.0;

	int32 i = 0;
	
	GraphData->ForEachCurrentSample([&](const double Stat)
	{
		double Y = WidgetSize.Y - FMath::Clamp((Stat * ScaleFactor), 0.0, MaxYAxisOfGraph) / MaxYAxisOfGraph * WidgetSize.Y;
		Y = FMath::Clamp(Y, Border, WidgetSize.Y - Border);

		Points.Emplace(XSlice * double(++i), Y);
	});

	// 为什么这个不直接画一条直线？？
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Points,
		ESlateDrawEffect::NoPixelSnapping,
		LineColor,
		false,
		LineThickness);
}
#endif

//////////////////////////////////////////////////////////////////////
// UTenkaichiPerfStatGraph

// 构造函数：设置裁剪。
UTenkaichiPerfStatGraph::UTenkaichiPerfStatGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetClipping(EWidgetClipping::ClipToBounds);
}

// 重建 Slate 控件。
TSharedRef<SWidget> UTenkaichiPerfStatGraph::RebuildWidget()
{
	return SAssignNew(SlateLatencyGraph, STenkaichiLatencyGraph);
}

// 释放 Slate 资源。
void UTenkaichiPerfStatGraph::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateLatencyGraph.Reset();
}

// 设置线条颜色。
void UTenkaichiPerfStatGraph::SetLineColor(const FColor& InColor)
{
	SlateLatencyGraph->SetLineColor(InColor);
}

// 设置 Y 轴最大值。
void UTenkaichiPerfStatGraph::SetMaxYValue(const float InValue)
{
	SlateLatencyGraph->SetMaxYValue(InValue);
}

// 设置背景色。
void UTenkaichiPerfStatGraph::SetBackgroundColor(const FColor& InValue)
{
	SlateLatencyGraph->SetBackgroundColor(InValue);
}

#if 0
// ⚠️ 注释挂起：依赖 P18 FSampledStatCache
// 更新曲线数据。
void UTenkaichiPerfStatGraph::UpdateGraphData(const FSampledStatCache* StatData, const float ScaleFactor)
{
	SlateLatencyGraph->UpdateGraphData(StatData, ScaleFactor);
}
#endif