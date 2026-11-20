// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiActivatableWidget)

#define LOCTEXT_NAMESPACE "Tenkaichi"

// 构造函数：空实现（父类处理初始化）。
UTenkaichiActivatableWidget::UTenkaichiActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 返回激活时期望的输入配置（按 InputConfig 决定）。
TOptional<FUIInputConfig> UTenkaichiActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case ETenkaichiWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case ETenkaichiWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case ETenkaichiWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case ETenkaichiWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

// 编辑器校验：检查蓝图是否实现了 BP_GetDesiredFocusTarget。
void UTenkaichiActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	// 检查蓝图有没有实现 BP_GetDesiredFocusTarget
	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UTenkaichiActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		// 如果直接继承自本类（没有中间原生子类），就报警告
		if (GetParentNativeClass(GetClass()) == UTenkaichiActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			// TODO - 先只记一条 note，因为我们无法保证它没有在原生基类里实现
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE