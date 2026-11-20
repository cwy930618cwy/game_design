// 版权声明（可保留或省略，不影响编译）
#include "UI/TenkaichiJoystickWidget.h"

#include "CommonHardwareVisibilityBorder.h"
#include "Components/Image.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiJoystickWidget)

#define LOCTEXT_NAMESPACE "TenkaichiJoystick"

// 构造函数：开启"消费指针输入"。
UTenkaichiJoystickWidget::UTenkaichiJoystickWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetConsumePointerInput(true);
}

// 触摸开始（记录原点 + 捕获鼠标）。
FReply UTenkaichiJoystickWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
	
	TouchOrigin = InGestureEvent.GetScreenSpacePosition();

	FReply Reply = FReply::Handled();
	if (!HasMouseCaptureByUser(InGestureEvent.GetUserIndex(), InGestureEvent.GetPointerIndex()))
	{
		Reply.CaptureMouse(GetCachedWidget().ToSharedRef());
	}
	return Reply;
}

// 触摸移动（算偏移 + 捕获鼠标）。
FReply UTenkaichiJoystickWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
	HandleTouchDelta(InGeometry, InGestureEvent);

	FReply Reply = FReply::Handled();
	if (!HasMouseCaptureByUser(InGestureEvent.GetUserIndex(), InGestureEvent.GetPointerIndex()))
	{
		Reply.CaptureMouse(GetCachedWidget().ToSharedRef());
	}
	return Reply;
}

// 触摸结束（停止模拟 + 释放鼠标）。
FReply UTenkaichiJoystickWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	StopInputSimulation();
	return FReply::Handled().ReleaseMouseCapture();
}

// 鼠标离开（停止模拟）。
void UTenkaichiJoystickWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	StopInputSimulation();
}

// 每帧 Tick（移动前景图 + 注入输入）。
void UTenkaichiJoystickWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CommonVisibilityBorder || CommonVisibilityBorder->IsVisible())
	{
		// 让内部摇杆图标跟着向量移动
		if (JoystickForeground && JoystickBackground)
		{
			JoystickForeground->SetRenderTranslation(
				(bNegateYAxis ? FVector2D(1.0f, -1.0f) : FVector2D(1.0f)) *
				StickVector *
				(JoystickBackground->GetDesiredSize() * 0.5f)
			);
		}
		InputKeyValue2D(StickVector);
	}
}

// 计算当前触摸相对原点的偏移位置。
void UTenkaichiJoystickWidget::HandleTouchDelta(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	const FVector2D& ScreenSpacePos = InGestureEvent.GetScreenSpacePosition();
	
	// 几何的本地中心就是它的大小
	FVector2D LocalStickCenter = InGeometry.GetAbsoluteSize();

	FVector2D ScreenSpaceStickCenter = InGeometry.LocalToAbsolute(LocalStickCenter);
	// 取相对原点的偏移
	FVector2D MoveStickOffset = (ScreenSpacePos - ScreenSpaceStickCenter);
	if (bNegateYAxis)
	{
		MoveStickOffset *= FVector2D(1.0f, -1.0f);
	}
	
	FVector2D MoveStickDir = FVector2D::ZeroVector;
	float MoveStickLength = 0.0f;
	MoveStickOffset.ToDirectionAndLength(MoveStickDir, MoveStickLength);

	MoveStickLength = FMath::Min(MoveStickLength, StickRange);
	MoveStickOffset = MoveStickDir * MoveStickLength;

	StickVector = MoveStickOffset / StickRange;
}

// 清空已注入的玩家输入，并禁用这个模拟摇杆。
void UTenkaichiJoystickWidget::StopInputSimulation()
{
	TouchOrigin = FVector2D::ZeroVector;
	StickVector = FVector2D::ZeroVector;
}

#undef LOCTEXT_NAMESPACE