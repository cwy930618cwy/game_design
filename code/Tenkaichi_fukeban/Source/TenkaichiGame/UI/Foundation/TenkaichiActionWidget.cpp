// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiActionWidget.h"

#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"

// 返回图标（按增强输入的实际键位查）。
FSlateBrush UTenkaichiActionWidget::GetIcon() const
{
	// 如果有增强输入动作关联到这个控件，就搜索绑定到该动作的键，
	// 用那些键的图标代替默认数据表设置。这覆盖了玩家重绑按键的情况。
	if (AssociatedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			// 查询这个动作绑定了哪些键
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(AssociatedInputAction);
			FSlateBrush SlateBrush;

			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
			// 用第一个绑定键的图标
			if (!BoundKeys.IsEmpty() && CommonInputSubsystem && UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, BoundKeys[0], CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
			{
				return SlateBrush;
			}
		}
	}
	
	// 否则回退到父类默认（数据表）
	return Super::GetIcon();
}

// 获取本控件的增强输入本地玩家子系统。
UEnhancedInputLocalPlayerSubsystem* UTenkaichiActionWidget::GetEnhancedInputSubsystem() const
{
	// 优先用"绑定控件"的本地玩家（如果这个控件被绑到了别的控件上）
	const UWidget* BoundWidget = DisplayedBindingHandle.GetBoundWidget();
	if (const ULocalPlayer* BindingOwner = BoundWidget ? BoundWidget->GetOwningLocalPlayer() : GetOwningLocalPlayer())
	{
		return BindingOwner->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}
	return nullptr;
}