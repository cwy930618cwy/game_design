// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonActionWidget.h"
#include "TenkaichiActionWidget.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/** 一个动作控件，会获取当前分配给这个控件上通用输入动作的按键图标。 */
UCLASS(BlueprintType, Blueprintable)
class UTenkaichiActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	//~UCommonActionWidget 接口开始
	// 返回图标（按增强输入的实际键位查）。
	virtual FSlateBrush GetIcon() const override;
	//~UCommonActionWidget 接口结束

	/** 和这个通用输入动作关联的增强输入动作。 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	const TObjectPtr<UInputAction> AssociatedInputAction;

private:

	// 获取本控件的增强输入本地玩家子系统。
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
	
};