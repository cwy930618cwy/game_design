// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"
#include "TenkaichiSimulatedInputWidget.generated.h"

#define UE_API TENKAICHIGAME_API

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UCommonHardwareVisibilityBorder;
class UEnhancedPlayerInput;

/**
 *  一个带"向增强输入子系统注入输入（按键或输入动作）"基础功能的 UMG 控件。
 */
UCLASS(MinimalAPI, meta=( DisplayName="Tenkaichi Simulated Input Widget" ))
class UTenkaichiSimulatedInputWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	
	// 构造函数：开启"消费指针输入"。
	UE_API UTenkaichiSimulatedInputWidget(const FObjectInitializer& ObjectInitializer);
	
	//~UWidget 接口开始
#if WITH_EDITOR
	// 编辑器调色板分类。
	UE_API virtual const FText GetPaletteCategory() override;
#endif
	//~UWidget 接口结束

	//~UUserWidget 接口开始
	// 控件构造完成（查初始键 + 监听控制映射变化）。
	UE_API virtual void NativeConstruct() override;
	// 控件销毁（解绑控制映射变化）。
	UE_API virtual void NativeDestruct() override;
	// 触摸结束（清空模拟输入）。
	UE_API virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	//~UUserWidget 接口结束
	
	/** 基于本控件的拥有本地玩家取增强输入子系统。没有拥有玩家就返回 null */
	UFUNCTION(BlueprintCallable)
	UE_API UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** 从当前输入子系统取玩家输入 */
	UE_API UEnhancedPlayerInput* GetPlayerInput() const;

	/** 取关联的输入动作 */
	UFUNCTION(BlueprintCallable)
	const UInputAction* GetAssociatedAction() const { return AssociatedAction; }

	/** 返回当前用来输入值的键。 */
	UFUNCTION(BlueprintCallable)
	FKey GetSimulatedKey() const { return KeyToSimulate; }

	/**
	 * 把给定的向量作为输入注入到当前模拟键。
	 * 这会调用当前玩家的 "InputKey"。
	 */
	UFUNCTION(BlueprintCallable)
	UE_API void InputKeyValue(const FVector& Value);

	/**
	 * 把给定的二维向量作为输入注入到当前模拟键。
	 * 这会调用当前玩家的 "InputKey"。
	 */
	UFUNCTION(BlueprintCallable)
	UE_API void InputKeyValue2D(const FVector2D& Value);

	// 清空所有模拟输入（松开所有键）。
	UFUNCTION(BlueprintCallable)
	UE_API void FlushSimulatedInput();
	
protected:

	/** 根据增强输入查询"关联动作绑了哪些键"来设置 KeyToSimulate */
	UE_API void QueryKeyToSimulate();

	/** 控制映射变化时调用，让我们有机会适配自己的键 */
	UFUNCTION()
	UE_API void OnControlMappingsRebuilt();

	/** CommonUI 的可见性边框，允许你按平台指定 UI */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonHardwareVisibilityBorder> CommonVisibilityBorder = nullptr;
	
	/** 要模拟输入的关联输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UInputAction> AssociatedAction = nullptr;

	/** 当关联动作没绑任何键时，用来模拟输入的备用键 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FKey FallbackBindingKey = EKeys::Gamepad_Right2D;

	/** 通过 InputKey 输入到玩家输入的那个键 */
	FKey KeyToSimulate;
};

#undef UE_API