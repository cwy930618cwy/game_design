// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Input/CommonBoundActionButton.h"

#include "TenkaichiBoundActionButton.generated.h"

#define UE_API TENKAICHIGAME_API

class UCommonButtonStyle;
class UObject;

// 绑定动作按钮：按输入方式（键鼠/手柄/触摸）自动切换按钮样式。
UCLASS(MinimalAPI, Abstract, meta = (DisableNativeTick))
class UTenkaichiBoundActionButton : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
protected:
	// 控件构造完成（绑输入方式变化事件 + 立刻应用一次样式）。
	UE_API virtual void NativeConstruct() override;

private:
	// 输入方式变化回调（切换样式）。
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);

	// 键鼠样式。
	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> KeyboardStyle;

	// 手柄样式。
	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> GamepadStyle;

	// 触摸样式。
	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> TouchStyle;
};

#undef UE_API