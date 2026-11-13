// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "UI/TenkaichiSimulatedInputWidget.h"

#include "TenkaichiTouchRegion.generated.h"

#define UE_API TENKAICHIGAME_API

class UObject;
struct FFrame;
struct FGeometry;
struct FPointerEvent;

/**
 * "触摸区域"用来定义屏幕上的一个区域，当用户手指按上去时应该触发某种输入
 */
UCLASS(MinimalAPI, meta=( DisplayName="Tenkaichi Touch Region" ))
class UTenkaichiTouchRegion : public UTenkaichiSimulatedInputWidget
{
	GENERATED_BODY()
	
public:
	
	//~UUserWidget 接口开始
	// 触摸开始（开始模拟输入）。
	UE_API virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 触摸移动（继续模拟输入）。
	UE_API virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 触摸结束（停止模拟输入）。
	UE_API virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 每帧 Tick（持续注入输入）。
	UE_API virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~UUserWidget 接口结束

	// 是否应该模拟输入（正在被触摸）。
	UFUNCTION(BlueprintCallable)
	bool ShouldSimulateInput() const { return bShouldSimulateInput; }

protected:

	/** 这个控件正在被触摸时为 true */
	bool bShouldSimulateInput = false;
};

#undef UE_API