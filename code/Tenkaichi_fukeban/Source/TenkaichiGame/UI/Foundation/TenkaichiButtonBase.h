// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonButtonBase.h"

#include "TenkaichiButtonBase.generated.h"

class UObject;
struct FFrame;

// 按钮基类：自动处理"按钮文字 vs 输入动作提示"的优先级。
UCLASS(Abstract, BlueprintType, Blueprintable)
class UTenkaichiButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	// 设置按钮文字（蓝图可调）。
	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText);
	
protected:
	// UUserWidget 接口开始
	// 蓝图预构造时（编辑器里预览 + 运行时构造）刷新样式和文字。
	virtual void NativePreConstruct() override;
	// UUserWidget 接口结束

	// UCommonButtonBase 接口开始
	// 输入动作控件更新时（刷新样式和文字）。
	virtual void UpdateInputActionWidget() override;
	// 输入方式变化时（刷新样式）。
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// UCommonButtonBase 接口结束

	// 刷新按钮文字（决定用输入动作文字还是手动文字）。
	void RefreshButtonText();
	
	// 蓝图可实现的更新按钮文字事件。
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	// 蓝图可实现的更新按钮样式事件。
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();
	
private:
	// 是否覆盖按钮文字（内联编辑开关）。
	UPROPERTY(EditAnywhere, Category="Button", meta=(InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;
	
	// 按钮文字（勾选覆盖时使用）。
	UPROPERTY(EditAnywhere, Category="Button", meta=( editcondition="bOverride_ButtonText" ))
	FText ButtonText;
};