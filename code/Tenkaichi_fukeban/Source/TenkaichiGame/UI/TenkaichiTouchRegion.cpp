// 版权声明（可保留或省略，不影响编译）
#include "UI/TenkaichiTouchRegion.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTouchRegion)

struct FGeometry;
struct FPointerEvent;

// 触摸开始（开始模拟输入）。
FReply UTenkaichiTouchRegion::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	bShouldSimulateInput = true;
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

// 触摸移动（继续模拟输入）。
FReply UTenkaichiTouchRegion::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	// 只要玩家在边界内触摸，就输入我们关联的键
	//InputKeyValue(FVector::OneVector);
	bShouldSimulateInput = true;
	// 如果应该的话，持续触发输入
	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

// 触摸结束（停止模拟输入）。
FReply UTenkaichiTouchRegion::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	bShouldSimulateInput = false;
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

// 每帧 Tick（持续注入输入）。
void UTenkaichiTouchRegion::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if(bShouldSimulateInput)
	{
		InputKeyValue(FVector::OneVector);
	}
}