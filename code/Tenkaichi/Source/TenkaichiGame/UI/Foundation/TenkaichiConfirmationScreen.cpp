// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiConfirmationScreen.h"

#if WITH_EDITOR
#include "CommonInputSettings.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#include "CommonBorder.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"
#include "TenkaichiButtonBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiConfirmationScreen)

// 装配对话框（填标题/正文，按动作创建按钮）。
void UTenkaichiConfirmationScreen::SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback)
{
	Super::SetupDialog(Descriptor, ResultCallback);

	// 填标题和正文
	Text_Title->SetText(Descriptor->Header);
	RichText_Description->SetText(Descriptor->Body);

	// 先清空按钮容器（并清掉旧按钮的点击委托）
	EntryBox_Buttons->Reset<UTenkaichiButtonBase>([](UTenkaichiButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	// 按每个按钮动作创建按钮
	for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
	{
		FDataTableRowHandle ActionRow;

		// 按动作结果决定用哪个输入动作（确认=点击键 / 拒绝=返回键 / 取消=取消键）
		switch(Action.Result)
		{
			case ECommonMessagingResult::Confirmed:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
				break;
			case ECommonMessagingResult::Declined:
				ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
				break;
			case ECommonMessagingResult::Cancelled:
				ActionRow = CancelAction;
				break;
			default:
				ensure(false);
				continue;
		}

		ULyraButtonBase* Button = EntryBox_Buttons->CreateEntry<UTenkaichiButtonBase>();
		Button->SetTriggeringInputAction(ActionRow);   // 设置触发这个按钮的输入动作
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
		Button->SetButtonText(Action.OptionalDisplayText);   // 设置按钮文字
	}

	OnResultCallback = ResultCallback;
}

// 关闭对话框。
void UTenkaichiConfirmationScreen::KillDialog()
{
	Super::KillDialog();
}

// 控件初始化（绑"点击空白关闭"事件）。
void UTenkaichiConfirmationScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic(this, &UTenkaichiConfirmationScreen::HandleTapToCloseZoneMouseButtonDown);
}

// 关闭确认窗口（触发回调）。
void UTenkaichiConfirmationScreen::CloseConfirmationWindow(ECommonMessagingResult Result)
{
	DeactivateWidget();
	OnResultCallback.ExecuteIfBound(Result);
}

// 处理"点击空白区域关闭"的鼠标事件。
FEventReply UTenkaichiConfirmationScreen::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();

	// 触摸事件 或 左键点击 → 当作"拒绝"关闭
	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CloseConfirmationWindow(ECommonMessagingResult::Declined);
		Reply.NativeReply = FReply::Handled();
	}

	return Reply;
}

#if WITH_EDITOR
// 编辑器默认值校验（CancelAction 必须配）。
void UTenkaichiConfirmationScreen::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	if (CancelAction.IsNull())
	{
		CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} has unset property: CancelAction.")), FText::FromString(GetName())));
	}
}
#endif