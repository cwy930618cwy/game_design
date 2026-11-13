// 版权声明（可保留或省略，不影响编译）
#include "SActorCanvas.h"

#include "Engine/GameViewportClient.h"
#include "IActorIndicatorWidget.h"
#include "Layout/ArrangedChildren.h"
#include "TenkaichiIndicatorManagerComponent.h"
#include "SceneView.h"
#include "UI/IndicatorSystem/IndicatorDescriptor.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SLeafWidget.h"

class FSlateRect;

// 箭头方向枚举。
namespace EArrowDirection
{
	enum Type
	{
		Left,
		Top,
		Right,
		Bottom,
		MAX
	};
}

// 每个方向对应的箭头旋转角度。
const float ArrowRotations[EArrowDirection::MAX] =
{
	270.0f,
	0.0f,
	90.0f,
	180.0f
};

// 每个方向对应的箭头偏移。
const FVector2D ArrowOffsets[EArrowDirection::MAX] =
{
	FVector2D(-1.0f, 0.0f),
	FVector2D(0.0f, -1.0f),
	FVector2D(1.0f, 0.0f),
	FVector2D(0.0f, 1.0f)
};

// 内部箭头控件：画一个可旋转的箭头图片。
class SActorCanvasArrowWidget : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SActorCanvasArrowWidget)
	{}
	/** 这个永远放最后 */
	SLATE_END_ARGS()

	/** 构造函数 */
	SActorCanvasArrowWidget()
	: Rotation(0.0f)
	, Arrow(nullptr)
	{

	}

	/** 每个控件都需要一个这个 */
	void Construct(const FArguments& InArgs, const FSlateBrush* ActorCanvasArrowBrush)
	{
		Arrow = ActorCanvasArrowBrush;
		SetCanTick(false);
	}

	// 绘制：画旋转后的箭头。
	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		int32 MaxLayerId = LayerId;

		if (Arrow)
		{
			const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
			const FColor FinalColorAndOpacity = (InWidgetStyle.GetColorAndOpacityTint() * Arrow->GetTint(InWidgetStyle)).ToFColor(true);

			// 画旋转盒（箭头）
			FSlateDrawElement::MakeRotatedBox(
				OutDrawElements,
				MaxLayerId++,
				AllottedGeometry.ToPaintGeometry(Arrow->ImageSize, FSlateLayoutTransform()),
				Arrow,
				DrawEffects,
				FMath::DegreesToRadians(GetRotation()),
				TOptional<FVector2D>(),
				FSlateDrawElement::RelativeToElement,
				FinalColorAndOpacity
			);
		}

		return MaxLayerId;
	}

	// 设置旋转角度（取模 360）。
	FORCEINLINE void SetRotation(float InRotation)
	{
		Rotation = FMath::Fmod(InRotation, 360.0f);
	}

	// 取旋转角度。
	FORCEINLINE float GetRotation() const
	{
		return Rotation;
	}

	// 期望尺寸：箭头图片大小。
	virtual FVector2D ComputeDesiredSize(float) const override
	{
		if (Arrow)
		{
			return Arrow->ImageSize;
		}
		else
		{
			return FVector2D::ZeroVector;
		}
	}

private:
	// 当前旋转角度。
	float Rotation;
	
	// 箭头画刷。
	const FSlateBrush* Arrow;
};

// 构造（Slate 初始化入口：绑管理器事件 + 存箭头画刷）。
void SActorCanvas::Construct(const FArguments& InArgs, const FLocalPlayerContext& InLocalPlayerContext, const FSlateBrush* InActorCanvasArrowBrush)
{
	LocalPlayerContext = InLocalPlayerContext;
	ActorCanvasArrowBrush = InActorCanvasArrowBrush;

	IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

	SetCanTick(false);
	SetVisibility(EVisibility::SelfHitTestInvisible);

	// 先创建 10 个箭头备用
	for (int32 i = 0; i < 10; ++i)
	{
		TSharedRef<SActorCanvasArrowWidget> ArrowWidget = SNew(SActorCanvasArrowWidget, ActorCanvasArrowBrush);
		ArrowWidget->SetVisibility(EVisibility::Collapsed);
		
		ArrowChildren.AddSlot(MoveTemp(
			FArrowSlot::FSlotArguments(MakeUnique<FArrowSlot>())
			[
				ArrowWidget
			]
		));
	}

	UpdateActiveTimer();
}

// 设置"是否显示任何指示器"（关掉时把所有子控件折叠）。
void SActorCanvas::SetShowAnyIndicators(bool bIndicators)
{
	if (bShowAnyIndicators != bIndicators)
	{
		bShowAnyIndicators = bIndicators;

		if (!bShowAnyIndicators)
		{
			for (int32 ChildIndex = 0; ChildIndex < AllChildren.Num(); ChildIndex++)
			{
				AllChildren.GetChildAt(ChildIndex)->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
}

// 指示器添加回调。
void SActorCanvas::OnIndicatorAdded(UIndicatorDescriptor* Indicator)
{
	AllIndicators.Add(Indicator);
	InactiveIndicators.Add(Indicator);
	
	AddIndicatorForEntry(Indicator);
}

// 指示器移除回调。
void SActorCanvas::OnIndicatorRemoved(UIndicatorDescriptor* Indicator)
{
	RemoveIndicatorForEntry(Indicator);
	
	AllIndicators.Remove(Indicator);
	InactiveIndicators.Remove(Indicator);
}

// 为一条指示器创建控件 + 槽位（异步加载控件类）。
void SActorCanvas::AddIndicatorForEntry(UIndicatorDescriptor* Indicator)
{
	// 异步加载指示器控件，并池化结果，方便使用和复用控件。
	TSoftClassPtr<UUserWidget> IndicatorClass = Indicator->GetIndicatorClass();
	if (!IndicatorClass.IsNull())
	{
		TWeakObjectPtr<UIndicatorDescriptor> IndicatorPtr(Indicator);
		AsyncLoad(IndicatorClass, [this, IndicatorPtr, IndicatorClass]() {
			if (UIndicatorDescriptor* Indicator = IndicatorPtr.Get())
			{
				// 异步加载期间可能已经被移除了。
				if (!AllIndicators.Contains(Indicator))
				{
					return;
				}

				// 从池里创建（或复用）控件。
				if (UUserWidget* IndicatorWidget = IndicatorPool.GetOrCreateInstance(TSubclassOf<UUserWidget>(IndicatorClass.Get())))
				{
					if (IndicatorWidget->GetClass()->ImplementsInterface(UIndicatorWidgetInterface::StaticClass()))
					{
						IIndicatorWidgetInterface::Execute_BindIndicator(IndicatorWidget, Indicator);
					}

					Indicator->IndicatorWidget = IndicatorWidget;

					InactiveIndicators.Remove(Indicator);

					AddActorSlot(Indicator)
					[
						SAssignNew(Indicator->CanvasHost, SBox)
						[
							IndicatorWidget->TakeWidget()
						]
					];
				}
			}
		});
		StartAsyncLoading();
	}
}

// 移除一条指示器的控件 + 槽位。
void SActorCanvas::RemoveIndicatorForEntry(UIndicatorDescriptor* Indicator)
{
	if (UUserWidget* IndicatorWidget = Indicator->IndicatorWidget.Get())
	{
		if (IndicatorWidget->GetClass()->ImplementsInterface(UIndicatorWidgetInterface::StaticClass()))
		{
			IIndicatorWidgetInterface::Execute_UnbindIndicator(IndicatorWidget, Indicator);
		}

		Indicator->IndicatorWidget = nullptr;
		
		IndicatorPool.Release(IndicatorWidget);
	}

	TSharedPtr<SWidget> CanvasHost = Indicator->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		RemoveActorSlot(CanvasHost.ToSharedRef());
		Indicator->CanvasHost.Reset();
	}
}

// 添加一个指示器槽位。
SActorCanvas::FScopedWidgetSlotArguments SActorCanvas::AddActorSlot(UIndicatorDescriptor* Indicator)
{
	TWeakPtr<SActorCanvas> WeakCanvas = SharedThis(this);
	return FScopedWidgetSlotArguments{ MakeUnique<FSlot>(Indicator), this->CanvasChildren, INDEX_NONE
		, [WeakCanvas](const FSlot*, int32)
		{
			if (TSharedPtr<SActorCanvas> Canvas = WeakCanvas.Pin())
			{
				Canvas->UpdateActiveTimer();
			}
		}};
}

// 移除一个槽位。
int32 SActorCanvas::RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < CanvasChildren.Num(); ++SlotIdx)
	{
		if ( SlotWidget == CanvasChildren[SlotIdx].GetWidget() )
		{
			CanvasChildren.RemoveAt(SlotIdx);

			UpdateActiveTimer();

			return SlotIdx;
		}
	}

	return -1;
}

// 按需启停活跃定时器。
void SActorCanvas::UpdateActiveTimer()
{
	const bool NeedsTicks = AllIndicators.Num() > 0 || !IndicatorComponentPtr.IsValid();

	if (NeedsTicks && !TickHandle.IsValid())
	{
		TickHandle = RegisterActiveTimer(0, FWidgetActiveTimerDelegate::CreateSP(this, &SActorCanvas::UpdateCanvas));
	}
}

// 活跃定时器回调：每帧更新画布。
EActiveTimerReturnType SActorCanvas::UpdateCanvas(double InCurrentTime, float InDeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorCanvas_UpdateCanvas);

	// ① 还没绘制过（没有几何信息），等下一帧
	if (!OptionalPaintGeometry.IsSet())
	{
		return EActiveTimerReturnType::Continue;
	}

	// ② 拿本地玩家
	ULocalPlayer* LocalPlayer = LocalPlayerContext.GetLocalPlayer();
	UTenkaichiIndicatorManagerComponent* IndicatorComponent = IndicatorComponentPtr.Get();
	if (IndicatorComponent == nullptr)
	{
		// ③ 懒获取管理器组件（第一次）
		IndicatorComponent = UTenkaichiIndicatorManagerComponent::GetComponent(LocalPlayerContext.GetPlayerController());
		if (IndicatorComponent)
		{
			// World 可能变了
			IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

			IndicatorComponentPtr = IndicatorComponent;
			// 绑增删事件
			IndicatorComponent->OnIndicatorAdded.AddSP(this, &SActorCanvas::OnIndicatorAdded);
			IndicatorComponent->OnIndicatorRemoved.AddSP(this, &SActorCanvas::OnIndicatorRemoved);
			// 补加"已经存在"的指示器
			for (UIndicatorDescriptor* Indicator : IndicatorComponent->GetIndicators())
			{
				OnIndicatorAdded(Indicator);
			}
		}
		else
		{
			//TODO HIDE EVERYTHING
			return EActiveTimerReturnType::Continue;
		}
	}

	// ④ 必须有玩家才能投影
	if (LocalPlayer)
	{
		const FGeometry PaintGeometry = OptionalPaintGeometry.GetValue();

		FSceneViewProjectionData ProjectionData;
		if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, /*out*/ ProjectionData))
		{
			SetShowAnyIndicators(true);

			bool IndicatorsChanged = false;

			// ⑤ 遍历所有槽位
			for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
			{
				SActorCanvas::FSlot& CurChild = CanvasChildren[ChildIndex];
				UIndicatorDescriptor* Indicator = CurChild.Indicator;

				// ⑥ 槽位内容失效且允许自动移除 → 移除
				if (Indicator->CanAutomaticallyRemove())
				{
					IndicatorsChanged = true;

					RemoveIndicatorForEntry(Indicator);
					// 索引回退（因为删了一个）
					--ChildIndex;
					continue;
				}

				// ⑦ 同步可见性
				CurChild.SetIsIndicatorVisible(Indicator->GetIsVisible());

				if (!CurChild.GetIsIndicatorVisible())
				{
					IndicatorsChanged |= CurChild.bIsDirty();
					CurChild.ClearDirtyFlag();
					continue;
				}

				// ⑧ 钳制状态变化 → 标记需要重绘
				if (CurChild.WasIndicatorClampedStatusChanged())
				{
					//Indicator->OnIndicatorClampedStatusChanged(CurChild.WasIndicatorClamped());
					CurChild.ClearIndicatorClampedStatusChangedFlag();
					IndicatorsChanged = true;
				}

				FVector ScreenPositionWithDepth;

				// ⑨ 投影：世界位置 → 屏幕位置 + 深度
				FIndicatorProjection Projector;
				const bool Success = Projector.Project(*Indicator, ProjectionData, PaintGeometry.Size, OUT ScreenPositionWithDepth);

				if (!Success)
				{
					CurChild.SetHasValidScreenPosition(false);
					CurChild.SetInFrontOfCamera(false);

					IndicatorsChanged |= CurChild.bIsDirty();
					CurChild.ClearDirtyFlag();
					continue;
				}

				// ⑩ 投影成功：更新"在相机前"和"有有效位置"
				CurChild.SetInFrontOfCamera(Success);
				CurChild.SetHasValidScreenPosition(CurChild.GetInFrontOfCamera() || Indicator->GetClampToScreen());

				if (CurChild.HasValidScreenPosition())
				{
					// 只有真能显示时才更新屏幕位置（避免脏标记误触发）
					CurChild.SetScreenPosition(FVector2D(ScreenPositionWithDepth));
					CurChild.SetDepth(ScreenPositionWithDepth.X);
				}

				// ⑪ 同步优先级
				CurChild.SetPriority(Indicator->GetPriority());

				IndicatorsChanged |= CurChild.bIsDirty();
				CurChild.ClearDirtyFlag();
			}

			// ⑫ 有变化就请求重绘
			if (IndicatorsChanged)
			{
				Invalidate(EInvalidateWidget::Paint);
			}
		}
		else
		{
			SetShowAnyIndicators(false);
		}
	}
	else
	{
		SetShowAnyIndicators(false);
	}

	// ⑬ 没指示器了就停定时器
	if (AllIndicators.Num() == 0)
	{
		TickHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}
	else
	{
		return EActiveTimerReturnType::Continue;
	}
}

// 布局：排序 + 钳制到屏幕边缘 + 算箭头位置。
void SActorCanvas::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorCanvas_OnArrangeChildren);

	NextArrowIndex = 0;

	// 必须有玩家才能投影
	if (bShowAnyIndicators)
	{
		const FVector2D ArrowWidgetSize = ActorCanvasArrowBrush->GetImageSize();
		const FIntPoint FixedPadding = FIntPoint(10.0f, 10.0f) + FIntPoint(ArrowWidgetSize.X, ArrowWidgetSize.Y);
		const FVector Center = FVector(AllottedGeometry.Size * 0.5f, 0.0f);

		// ① 收集所有槽位指针
		TArray<const SActorCanvas::FSlot*> SortedSlots;
		for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
		{
			SortedSlots.Add(&CanvasChildren[ChildIndex]);
		}

		// ② 排序：优先级升序；同优先级时深度降序（远的先画）
		SortedSlots.StableSort([](const SActorCanvas::FSlot& A, const SActorCanvas::FSlot& B)
		{
			return A.GetPriority() == B.GetPriority() ? A.GetDepth() > B.GetDepth() : A.GetPriority() < B.GetPriority();
		});

		// ③ 遍历排序后的槽位
		for (int32 ChildIndex = 0; ChildIndex < SortedSlots.Num(); ++ChildIndex)
		{
			const SActorCanvas::FSlot& CurChild = *SortedSlots[ChildIndex];
			const UIndicatorDescriptor* Indicator = CurChild.Indicator;

			// ④ 不可见的跳过（并清掉钳制标记）
			if (!ArrangedChildren.Accepts(CurChild.GetWidget()->GetVisibility()))
			{
				CurChild.SetWasIndicatorClamped(false);
				continue;
			}

			FVector2D ScreenPosition = CurChild.GetScreenPosition();
			const bool bInFrontOfCamera = CurChild.GetInFrontOfCamera();

			// ⑤ 这条指示器是否允许钳制
			const bool bShouldClamp = Indicator->GetClampToScreen();

			// ⑥ 算槽位尺寸/偏移/内边距
			FVector2D SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax;
			GetOffsetAndSize(Indicator, SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax);

			bool bWasIndicatorClamped = false;

			// ⑦ 需要钳制才做这一堆计算
			if (bShouldClamp)
			{
				EArrowDirection::Type ClampDir = EArrowDirection::MAX;

				// 算"可钳制区域"（屏幕内缩进 padding + 固定边距）
				const FIntPoint RectMin = FIntPoint(SlotPaddingMin.X, SlotPaddingMin.Y) + FixedPadding;
				const FIntPoint RectMax = FIntPoint(AllottedGeometry.Size.X - SlotPaddingMax.X, AllottedGeometry.Size.Y - SlotPaddingMax.Y) - FixedPadding;
				const FIntRect ClampRect(RectMin, RectMax);

				// ⑧ 位置在钳制区外 → 用"线段与平面求交"算贴边点
				if (!ClampRect.Contains(FIntPoint(ScreenPosition.X, ScreenPosition.Y)))
				{
					const FPlane Planes[] =
					{
						FPlane(FVector(1.0f, 0.0f, 0.0f), ClampRect.Min.X),	// 左
						FPlane(FVector(0.0f, 1.0f, 0.0f), ClampRect.Min.Y),	// 上
						FPlane(FVector(-1.0f, 0.0f, 0.0f), -ClampRect.Max.X),	// 右
						FPlane(FVector(0.0f, -1.0f, 0.0f), -ClampRect.Max.Y)	// 下
					};

					for (int32 i = 0; i < EArrowDirection::MAX; ++i)
					{
						FVector NewPoint;
						if (FMath::SegmentPlaneIntersection(Center, FVector(ScreenPosition, 0.0f), Planes[i], NewPoint))
						{
							ClampDir = (EArrowDirection::Type)i;
							ScreenPosition = FVector2D(NewPoint);
						}
					}
				}
				// ⑨ 在相机背后 → 按归一化坐标判断贴哪条边
				else if (!bInFrontOfCamera)
				{
					const float ScreenXNorm = ScreenPosition.X / (RectMax.X - RectMin.X);
					const float ScreenYNorm = ScreenPosition.Y / (RectMax.Y - RectMin.Y);
					// 需要把这个东西钉到屏幕边上
					if (ScreenXNorm < ScreenYNorm)
					{
						if (ScreenXNorm < (-ScreenYNorm + 1.0f))
						{
							ClampDir = EArrowDirection::Left;
							ScreenPosition.X = ClampRect.Min.X;
						}
						else
						{
							ClampDir = EArrowDirection::Bottom;
							ScreenPosition.Y = ClampRect.Max.Y;
						}
					}
					else
					{
						if (ScreenXNorm < (-ScreenYNorm + 1.0f))
						{
							ClampDir = EArrowDirection::Top;
							ScreenPosition.Y = ClampRect.Min.Y;
						}
						else
						{
							ClampDir = EArrowDirection::Right;
							ScreenPosition.X = ClampRect.Max.X;
						}
					}
				}

				bWasIndicatorClamped = (ClampDir != EArrowDirection::MAX);

				// ⑩ 需要显示箭头吗
				if (Indicator->GetShowClampToScreenArrow() &&
					bWasIndicatorClamped &&
					ArrowChildren.IsValidIndex(NextArrowIndex))
				{
					const FVector2D ArrowOffsetDirection = ArrowOffsets[ClampDir];
					const float ArrowRotation = ArrowRotations[ClampDir];

					// 取一个箭头控件
					TSharedRef<SActorCanvasArrowWidget> ArrowWidgetToUse = StaticCastSharedRef<SActorCanvasArrowWidget>(ArrowChildren.GetChildAt(NextArrowIndex));
					NextArrowIndex++;

					// 设置箭头旋转
					ArrowWidgetToUse->SetRotation(ArrowRotation);

					// 算偏移量大小
					const FVector2D OffsetMagnitude = (SlotSize + ArrowWidgetSize) * 0.5f;

					// 用于把箭头居中到位置上
					const FVector2D ArrowCenteringOffset = -(ArrowWidgetSize * 0.5f);

					FVector2D ArrowAlignmentOffset = FVector2D::ZeroVector;
					switch (Indicator->VAlignment)
					{
					case VAlign_Top:
						ArrowAlignmentOffset = SlotSize * FVector2D(0.0f, 0.5f);
						break;
					case VAlign_Bottom:
						ArrowAlignmentOffset = SlotSize * FVector2D(0.0f, -0.5f);
						break;
					}

					// 算箭头偏移
					const FVector2D WidgetOffset = (OffsetMagnitude * ArrowOffsetDirection);

					const FVector2D FinalOffset = (WidgetOffset + ArrowAlignmentOffset + ArrowCenteringOffset);

					// 最终位置
					const FVector2D FinalPosition = (ScreenPosition + FinalOffset);

					ArrowWidgetToUse->SetVisibility(EVisibility::HitTestInvisible);

					// 把箭头注入到指示器上方
					ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
						ArrowWidgetToUse,			// 被布局的子控件
						FinalPosition,				// 子控件的局部位置
						ArrowWidgetSize,			// 子控件尺寸
						1.f							// 子控件缩放
					));
				}
			}

			CurChild.SetWasIndicatorClamped(bWasIndicatorClamped);

			// ⑪ 把指示器控件加进输出列表
			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
				CurChild.GetWidget(),
				ScreenPosition + SlotOffset,
				SlotSize,
				1.f
			));
		}
	}

	// ⑫ 上一帧用了更多箭头 → 把多余的折叠
	if (NextArrowIndex < ArrowIndexLastUpdate)
	{
		for (int32 ArrowRemovedIndex = NextArrowIndex; ArrowRemovedIndex < ArrowIndexLastUpdate; ArrowRemovedIndex++)
		{
			ArrowChildren.GetChildAt(ArrowRemovedIndex)->SetVisibility(EVisibility::Collapsed);
		}
	}

	ArrowIndexLastUpdate = NextArrowIndex;
}

// 绘制：缓存几何 + 遍历子控件调 Paint。
int32 SActorCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorCanvas_OnPaint);

	// ① 缓存几何（UpdateCanvas 要用）
	OptionalPaintGeometry = AllottedGeometry;

	// ② 先布局（调 OnArrangeChildren）
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	int32 MaxLayerId = LayerId;

	const FPaintArgs NewArgs = Args.WithNewParent(this);
	const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);

	// ③ 遍历布局结果，逐个绘制
	for (const FArrangedWidget& CurWidget : ArrangedChildren.GetInternalArray())
	{
		if (!IsChildWidgetCulled(MyCullingRect, CurWidget))
		{
			SWidget* MutableWidget = const_cast<SWidget*>(&CurWidget.Widget.Get());

			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, MyCullingRect, OutDrawElements, bDrawElementsInOrder ? MaxLayerId : LayerId, InWidgetStyle, bShouldBeEnabled);
			MaxLayerId = FMath::Max(MaxLayerId, CurWidgetsMaxLayerId);
		}
		else
		{
			//SlateGI - RemoveContent
		}
	}

	return MaxLayerId;
}

// 返回引用收集器里显示的名字。
FString SActorCanvas::GetReferencerName() const
{
	return TEXT("SActorCanvas");
}

// 把 UObject 引用报告给 GC（防回收）。
void SActorCanvas::AddReferencedObjects( FReferenceCollector& Collector )
{
	Collector.AddReferencedObjects(AllIndicators);
}

// 按对齐方式算控件偏移/尺寸/内边距。
void SActorCanvas::GetOffsetAndSize(const UIndicatorDescriptor* Indicator,
	FVector2D& OutSize, 
	FVector2D& OutOffset,
	FVector2D& OutPaddingMin,
	FVector2D& OutPaddingMax) const
{
	// 这个将来可能会用到
	FVector2D AllottedSize = FVector2D::ZeroVector;

	// 取子控件的期望尺寸
	TSharedPtr<SWidget> CanvasHost = Indicator->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		OutSize = CanvasHost->GetDesiredSize();
	}

	// 处理水平对齐
	switch(Indicator->GetHAlign())
	{
		case HAlign_Left: // 同 Align_Top
			OutOffset.X = 0.0f;
			OutPaddingMin.X = 0.0f;
			OutPaddingMax.X = OutSize.X;
			break;
		
		case HAlign_Center:
			OutOffset.X = (AllottedSize.X - OutSize.X) / 2.0f;
			OutPaddingMin.X = OutSize.X / 2.0f;
			OutPaddingMax.X = OutPaddingMin.X;
			break;
		
		case HAlign_Right: // 同 Align_Bottom
			OutOffset.X = AllottedSize.X - OutSize.X;
			OutPaddingMin.X = OutSize.X;
			OutPaddingMax.X = 0.0f;
			break;
	}

	// 处理垂直对齐
	switch(Indicator->GetVAlign())
	{
		case VAlign_Top:
			OutOffset.Y = 0.0f;
			OutPaddingMin.Y = 0.0f;
			OutPaddingMax.Y = OutSize.Y;
			break;
		
		case VAlign_Center:
			OutOffset.Y = (AllottedSize.Y - OutSize.Y) / 2.0f;
			OutPaddingMin.Y = OutSize.Y / 2.0f;
			OutPaddingMax.Y = OutPaddingMin.Y;
			break;
		
		case VAlign_Bottom:
			OutOffset.Y = AllottedSize.Y - OutSize.Y;
			OutPaddingMin.Y = OutSize.Y;
			OutPaddingMax.Y = 0.0f;
			break;
	}
}