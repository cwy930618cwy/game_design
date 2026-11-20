// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "AsyncMixin.h"
#include "Blueprint/UserWidgetPool.h"
#include "Widgets/SPanel.h"

class FActiveTimerHandle;
class FArrangedChildren;
class FChildren;
class FPaintArgs;
class FReferenceCollector;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UIndicatorDescriptor;
class UTenkaichiIndicatorManagerComponent;
struct FSlateBrush;

// 指示器画布：把指示器投影到屏幕并绘制（Slate 面板）。
class SActorCanvas : public SPanel, public FAsyncMixin, public FGCObject
{
public:
	/** 画布专用的槽位类 */
	class FSlot : public TSlotBase<FSlot>
	{
	public: 

		// 构造函数：初始化所有状态。
		FSlot(UIndicatorDescriptor* InIndicator)
			: TSlotBase<FSlot>()
			, Indicator(InIndicator)
			, ScreenPosition(FVector2D::ZeroVector)
			, Depth(0)
			, Priority(0.f)
			, bIsIndicatorVisible(true)
			, bInFrontOfCamera(true)
			, bHasValidScreenPosition(false)
			, bDirty(true)
			, bWasIndicatorClamped(false)
			, bWasIndicatorClampedStatusChanged(false)
		{
		}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_SLOT_END_ARGS()
		using TSlotBase<FSlot>::Construct;

		// 取指示器是否可见。
		bool GetIsIndicatorVisible() const { return bIsIndicatorVisible; }
		// 设置指示器可见性（变了就标脏 + 刷新可见性）。
		void SetIsIndicatorVisible(bool bVisible)
		{
			if (bIsIndicatorVisible != bVisible)
			{
				bIsIndicatorVisible = bVisible;
				bDirty = true;
			}

			RefreshVisibility();
		}

		// 取屏幕位置。
		FVector2D GetScreenPosition() const { return ScreenPosition; }
		// 设置屏幕位置（变了就标脏）。
		void SetScreenPosition(FVector2D InScreenPosition)
		{
			if (ScreenPosition != InScreenPosition)
			{
				ScreenPosition = InScreenPosition;
				bDirty = true;
			}
		}

		// 取深度（到相机距离）。
		double GetDepth() const { return Depth; }
		// 设置深度（变了就标脏）。
		void SetDepth(double InDepth)
		{
			if (Depth != InDepth)
			{
				Depth = InDepth;
				bDirty = true;
			}
		}

		// 取排序优先级。
		int32 GetPriority() const { return Priority; }
		// 设置排序优先级（变了就标脏）。
		void SetPriority(int32 InPriority)
		{
			if (Priority != InPriority)
			{
				Priority = InPriority;
				bDirty = true;
			}
		}

		// 取是否在相机前方。
		bool GetInFrontOfCamera() const { return bInFrontOfCamera; }
		// 设置是否在相机前方（变了就标脏 + 刷新可见性）。
		void SetInFrontOfCamera(bool bInFront)
		{
			if (bInFrontOfCamera != bInFront)
			{
				bInFrontOfCamera = bInFront;
				bDirty = true;
			}

			RefreshVisibility();
		}

		// 是否有有效的屏幕位置。
		bool HasValidScreenPosition() const { return bHasValidScreenPosition; }
		// 设置是否有有效屏幕位置（变了就标脏 + 刷新可见性）。
		void SetHasValidScreenPosition(bool bValidScreenPosition)
		{
			if (bHasValidScreenPosition != bValidScreenPosition)
			{
				bHasValidScreenPosition = bValidScreenPosition;
				bDirty = true;
			}

			RefreshVisibility();
		}

		// 是否脏（需要重新布局/绘制）。
		bool bIsDirty() const { return bDirty; }

		// 清除脏标记。
		void ClearDirtyFlag()
		{
			bDirty = false;
		}

		// 上一帧是否被钳制到屏幕边缘。
		bool WasIndicatorClamped() const { return bWasIndicatorClamped; }
		// 设置钳制状态（变了就标记"状态已变"）。
		void SetWasIndicatorClamped(bool bWasClamped) const
		{
			if (bWasClamped != bWasIndicatorClamped)
			{
				bWasIndicatorClamped = bWasClamped;
				bWasIndicatorClampedStatusChanged = true;
			}
		}

		// 钳制状态是否变化过。
		bool WasIndicatorClampedStatusChanged() const { return bWasIndicatorClampedStatusChanged; }
		// 清除"钳制状态已变"标记。
		void ClearIndicatorClampedStatusChangedFlag()
		{
			bWasIndicatorClampedStatusChanged = false;
		}

	private:
		// 刷新控件可见性（可见 + 有有效位置 才显示）。
		void RefreshVisibility()
		{
			const bool bIsVisible = bIsIndicatorVisible && bHasValidScreenPosition;
			GetWidget()->SetVisibility(bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		}

		// 由 SActorCanvas::AddReferencedObjects 保活
		UIndicatorDescriptor* Indicator;
		FVector2D ScreenPosition;
		double Depth;
		int32 Priority;

		uint8 bIsIndicatorVisible : 1;
		uint8 bInFrontOfCamera : 1;
		uint8 bHasValidScreenPosition : 1;
		uint8 bDirty : 1;
		
		/** 
		 * 缓存并延迟一帧的"上一帧是否被屏幕钳制"值；
		 * 半 hack 的 mutable 实现，因为它在 const 绘制操作期间被缓存
		 */
		mutable uint8 bWasIndicatorClamped : 1;
		mutable uint8 bWasIndicatorClampedStatusChanged : 1;

		friend class SActorCanvas;
	};

	/** 画布专用的箭头槽位类 */
	class FArrowSlot : public TSlotBase<FArrowSlot>
	{
	};

	/** 开始声明这个 Slate 控件的参数 */
	SLATE_BEGIN_ARGS(SActorCanvas) {
		_Visibility = EVisibility::HitTestInvisible;
	}

		/** 表示我们有一个这个控件支持的槽位 */
		SLATE_SLOT_ARGUMENT(SActorCanvas::FSlot, Slots)
	
	/** 这个永远放最后 */
	SLATE_END_ARGS()

	// 构造函数：初始化三个子容器。
	SActorCanvas()
		: CanvasChildren(this)
		, ArrowChildren(this)
		, AllChildren(this)
	{
		AllChildren.AddChildren(CanvasChildren);
		AllChildren.AddChildren(ArrowChildren);
	}

	// 构造（Slate 初始化入口：绑管理器事件 + 存箭头画刷）。
	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InCtx, const FSlateBrush* ActorCanvasArrowBrush);

	//~SWidget 接口开始
	// 布局：算每个槽位的位置。
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;
	// 期望尺寸：0（不占布局空间）。
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	// 返回子控件容器。
	virtual FChildren* GetChildren() override { return &AllChildren; }
	// 绘制：画所有指示器 + 箭头。
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	//~SWidget 接口结束

	// 设置是否按添加顺序绘制（关闭合批）。
	void SetDrawElementsInOrder(bool bInDrawElementsInOrder) { bDrawElementsInOrder = bInDrawElementsInOrder; }

	//~FGCObject 接口开始
	// 返回引用收集器里显示的名字。
	virtual FString GetReferencerName() const override;
	// 把 UObject 引用报告给 GC（防回收）。
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
	//~FGCObject 接口结束
	
private:
	// 指示器添加回调。
	void OnIndicatorAdded(UIndicatorDescriptor* Indicator);
	// 指示器移除回调。
	void OnIndicatorRemoved(UIndicatorDescriptor* Indicator);

	// 为一条指示器创建控件 + 槽位。
	void AddIndicatorForEntry(UIndicatorDescriptor* Indicator);
	// 移除一条指示器的控件 + 槽位。
	void RemoveIndicatorForEntry(UIndicatorDescriptor* Indicator);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	// 添加一个指示器槽位。
	FScopedWidgetSlotArguments AddActorSlot(UIndicatorDescriptor* Indicator);
	// 移除一个槽位。
	int32 RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget);

	// 设置"是否显示任何指示器"。
	void SetShowAnyIndicators(bool bIndicators);
	// 活跃定时器回调：每帧更新画布。
	EActiveTimerReturnType UpdateCanvas(double InCurrentTime, float InDeltaTime);

	/** 计算偏移的辅助函数 */
	void GetOffsetAndSize(const UIndicatorDescriptor* Indicator,
		FVector2D& OutSize, 
		FVector2D& OutOffset,
		FVector2D& OutPaddingMin,
		FVector2D& OutPaddingMax) const;

	// 按需启停活跃定时器。
	void UpdateActiveTimer();

private:
	// 所有指示器（UObject 数组，GC 追踪）。
	TArray<TObjectPtr<UIndicatorDescriptor>> AllIndicators;
	// 待移除的指示器。
	TArray<UIndicatorDescriptor*> InactiveIndicators;
	
	// 本地玩家上下文。
	FLocalPlayerContext LocalPlayerContext;
	// 指示器管理器组件（弱引用）。
	TWeakObjectPtr<UTenkaichiIndicatorManagerComponent> IndicatorComponentPtr;

	/** 这个画布里的所有槽位 */
	TPanelChildren<FSlot> CanvasChildren;
	// 箭头槽位（mutable：const 绘制时也要改）。
	mutable TPanelChildren<FArrowSlot> ArrowChildren;
	// 合并的子容器（Canvas + Arrow）。
	FCombinedChildren AllChildren;

	// 指示器控件对象池（复用 UUserWidget）。
	FUserWidgetPool IndicatorPool;

	// 箭头画刷。
	const FSlateBrush* ActorCanvasArrowBrush = nullptr;

	// 下一个箭头索引。
	mutable int32 NextArrowIndex = 0;
	// 上次更新时的箭头索引。
	mutable int32 ArrowIndexLastUpdate = 0;

	/** 是否按添加顺序绘制元素。注意：开启会禁用合批，导致更多 draw call */
	bool bDrawElementsInOrder = false;

	// 是否显示任何指示器。
	bool bShowAnyIndicators = false;

	// 缓存的绘制几何（mutable：const 绘制时写入）。
	mutable TOptional<FGeometry> OptionalPaintGeometry;

	// 活跃定时器句柄。
	TSharedPtr<FActiveTimerHandle> TickHandle;
};