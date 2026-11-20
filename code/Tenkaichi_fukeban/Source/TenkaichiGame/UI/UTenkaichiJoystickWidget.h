// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "UI/TenkaichiSimulatedInputWidget.h"

#include "TenkaichiJoystickWidget.generated.h"

#define UE_API TENKAICHIGAME_API

class UImage;
class UObject;
struct FGeometry;
struct FPointerEvent;

/**
 *  Tenkaichi 虚拟摇杆的 UMG 包装。
 *
 *  这会计算一个钳制在 -1 到 1 之间的二维向量，
 *  作为键值输入给玩家，模拟手柄模拟摇杆。
 *
 *  这是为配合增强输入玩家使用而设计的。
 */
UCLASS(MinimalAPI, meta=( DisplayName="Tenkaichi Joystick" ))
class UTenkaichiJoystickWidget : public UTenkaichiSimulatedInputWidget
{
	GENERATED_BODY()
	
public:
	
	// 构造函数：开启"消费指针输入"。
	UE_API UTenkaichiJoystickWidget(const FObjectInitializer& ObjectInitializer);

	//~UUserWidget 接口开始
	// 触摸开始（记录原点 + 捕获鼠标）。
	UE_API virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 触摸移动（算偏移 + 捕获鼠标）。
	UE_API virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 触摸结束（停止模拟 + 释放鼠标）。
	UE_API virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	// 鼠标离开（停止模拟）。
	UE_API virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	// 每帧 Tick（移动前景图 + 注入输入）。
	UE_API virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~UUserWidget 接口结束
	
protected:

	/**
	 * 计算当前触摸相对原点的偏移位置。
	 *
	 * 在玩家身上输入关联的手柄键
	 *
	 * 让前景摇杆图跟着给定输入移动，营造"跟着玩家手指动"的视觉效果
	 */
	UE_API void HandleTouchDelta(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent);
	
	/** 清空已注入的玩家输入，并禁用这个模拟摇杆。 */
	UE_API void StopInputSimulation();

	/** 摇杆内部图片最多能移动多远？ */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float StickRange = 50.0f;

	/** 用作摇杆背景的图片 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> JoystickBackground;

	/** 用作摇杆前景的图片 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> JoystickForeground;

	/** 是否要反转摇杆的 Y 轴值？这对"移动"摇杆很常见 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bNegateYAxis = false;

	/** 触摸的原点。在 NativeOnTouchStarted 里设置 */
	UPROPERTY(Transient)
	FVector2D TouchOrigin = FVector2D::ZeroVector;

	// 摇杆向量（-1~1）。
	UPROPERTY(Transient)
	FVector2D StickVector = FVector2D::ZeroVector;
};

#undef UE_API