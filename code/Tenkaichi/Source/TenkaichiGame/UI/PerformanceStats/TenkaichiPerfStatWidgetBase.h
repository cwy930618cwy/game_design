// 延迟曲线图的 Slate 控件（画折线）。
class STenkaichiLatencyGraph : public SLeafWidget
{
public:
	/** 开始声明这个 Slate 控件的参数 */
	SLATE_BEGIN_ARGS(STenkaichiLatencyGraph)
		: _DesiredSize(150, 50),
		_MaxLatencyToGraph(33.0),
		_LineColor(255, 255, 255, 255),
		_BackgroundColor(0, 0, 0, 128)
	{
		_Clipping = EWidgetClipping::ClipToBounds;
	}

	SLATE_ARGUMENT(FVector2D, DesiredSize)
	SLATE_ARGUMENT(double, MaxLatencyToGraph)
	SLATE_ARGUMENT(FColor, LineColor)
	SLATE_ARGUMENT(FColor, BackgroundColor)
	SLATE_END_ARGS()

	/** 每个控件都需要的构造函数 */
	void Construct(const FArguments& InArgs);

	/** 绘制时调用（传入要画的元素） */
	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	// 易变（每帧重绘）。
	virtual bool ComputeVolatility() const override { return true; }
	
	// 期望尺寸。
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	// 设置线条颜色。
	inline void SetLineColor(const FColor& InColor)
	{
		LineColor = InColor;	
	}

	// 设置 Y 轴最大值。
	inline void SetMaxYValue(const double InValue)
	{
		MaxYAxisOfGraph = InValue;
	}

	// 设置背景色。
	inline void SetBackgroundColor(const FColor& InColor)
	{
		BackgroundColor = InColor;
	}

	// 更新曲线数据（数据源 + 缩放系数）。
	inline void UpdateGraphData(const FSampledStatCache* StatData, const float InScaleFactor)
	{
		GraphData = StatData;
		ScaleFactor = InScaleFactor;
	}
	
private:
	
	// 画总延迟折线。
	void DrawTotalLatency(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/**
	 * 要画的图的大小
	 */
	FVector2D DesiredSize = { 150.0, 50.0 };

	/**
	 * 图的 Y 轴最大值。画出来的值会被钳制到这个值
	 */
	double MaxYAxisOfGraph = 33.0;

	// 缩放系数。
	float ScaleFactor = 1.0f;

	/**
	 * 要画在图上那条线的颜色
	 */
	FColor LineColor = FColor(255, 255, 255, 255);

	/**
	 * 画图时用的背景色
	 */
	FColor BackgroundColor = FColor(0, 0, 0, 128);

	/**
	 * 这个图控件需要画的数据缓存
	 */
	const FSampledStatCache* GraphData = nullptr;
};

/**
 * UTenkaichiPerfStatGraph
 *
 * 显示某个 stat 随时间变化的曲线图的控件基类。
 */
UCLASS(meta = (DisableNativeTick))
class UTenkaichiPerfStatGraph : public UUserWidget
{
	GENERATED_BODY()

public:
	// 构造函数：设置裁剪。
	UTenkaichiPerfStatGraph(const FObjectInitializer& ObjectInitializer);
	
	// 设置线条颜色。
	void SetLineColor(const FColor& InColor);
	
	// 设置 Y 轴最大值。
	void SetMaxYValue(const float InValue);
	
	// 设置背景色。
	void SetBackgroundColor(const FColor& InValue);

	// 更新曲线数据。
	void UpdateGraphData(const FSampledStatCache* StatData, const float ScaleFactor);
	
protected:
	//~UWidget 接口开始
	// 重建 Slate 控件。
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// 释放 Slate 资源。
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~UWidget 接口结束

	// 真正画图的 Slate 控件。在 RebuildWidget 里创建，在 ReleaseSlateResources 里销毁。
	TSharedPtr<STenkaichiLatencyGraph> SlateLatencyGraph;
};