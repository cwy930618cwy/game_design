// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiBoundActionButton.h"

#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiBoundActionButton)

class UCommonButtonStyle;

// 控件构造完成（绑输入方式变化事件 + 立刻应用一次样式）。
void UTenkaichiBoundActionButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		// 绑"输入方式变化"事件
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		// 立刻按当前输入方式应用一次（否则首次显示样式不对）
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

// 输入方式变化回调（切换样式）。
void UTenkaichiBoundActionButton::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	TSubclassOf<UCommonButtonStyle> NewStyle = nullptr;

	if (NewInputMethod == ECommonInputType::Gamepad)
	{
		NewStyle = GamepadStyle;
	}
	else if (NewInputMethod == ECommonInputType::Touch)
	{
		NewStyle = TouchStyle;
	}
	else
	{
		NewStyle = KeyboardStyle;
	}

	// 只有配了样式才设置（没配就保留当前）
	if (NewStyle)
	{
		SetStyle(NewStyle);
	}
}