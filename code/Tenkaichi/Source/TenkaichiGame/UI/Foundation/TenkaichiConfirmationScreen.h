// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataTable.h"
#include "Messaging/CommonGameDialog.h"

#include "Messaging/CommonMessagingSubsystem.h"
#include "TenkaichiConfirmationScreen.generated.h"

class IWidgetCompilerLog;

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

/**
 *	确认屏：把对话框描述装配成实际的 UI。
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class UTenkaichiConfirmationScreen : public UCommonGameDialog
{
	GENERATED_BODY()
public:
	// 装配对话框（填标题/正文，按动作创建按钮）。
	virtual void SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback) override;
	// 关闭对话框。
	virtual void KillDialog() override;

protected:
	// 控件初始化（绑"点击空白关闭"事件）。
	virtual void NativeOnInitialized() override;
	// 关闭确认窗口（触发回调）。
	virtual void CloseConfirmationWindow(ECommonMessagingResult Result);

#if WITH_EDITOR
	// 编辑器默认值校验。
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

private:

	// 处理"点击空白区域关闭"的鼠标事件。
	UFUNCTION()
	FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	// 结果回调（对话框关闭时通知调用方）。
	FCommonMessagingResultDelegate OnResultCallback;

private:
	// 标题文本控件。
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	// 正文富文本控件。
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	// 按钮容器（动态创建按钮）。
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

	// "点击关闭"区域边框。
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonBorder> Border_TapToCloseZone;

	// 取消动作的数据表行（编辑器里配）。
	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle CancelAction;
};