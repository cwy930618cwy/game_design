// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiButtonBase.h"
#include "CommonActionWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiButtonBase)

// 蓝图预构造时（编辑器里预览 + 运行时构造）刷新样式和文字。
void UTenkaichiButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateButtonStyle();
	RefreshButtonText();
}

// 输入动作控件更新时（刷新样式和文字）。
void UTenkaichiButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	UpdateButtonStyle();
	RefreshButtonText();
}

// 设置按钮文字（蓝图可调）。
void UTenkaichiButtonBase::SetButtonText(const FText& InText)
{
	// 传空字符串 = 取消覆盖（改回用输入动作提示）
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

// 刷新按钮文字（决定用输入动作文字还是手动文字）。
void UTenkaichiButtonBase::RefreshButtonText()
{
	// 如果要覆盖，或手动文字是空的，就优先用输入动作的显示文字
	if (bOverride_ButtonText || ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();	
			if (!ActionDisplayText.IsEmpty())
			{
				UpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}
	
	UpdateButtonText(ButtonText);	
}

// 输入方式变化时（刷新样式）。
void UTenkaichiButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	UpdateButtonStyle();
}