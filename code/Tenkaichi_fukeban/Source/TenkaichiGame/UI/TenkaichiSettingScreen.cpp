// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiSettingScreen.h"

#include "Input/CommonUIInputTypes.h"
#include "Player/TenkaichiLocalPlayer.h"
//#include "Settings/TenkaichiGameSettingRegistry.h"   // ⚠️ 注释挂起：依赖 P18 Settings

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiSettingScreen)

class UGameSettingRegistry;

// 控件初始化（注册三个输入动作绑定）。
void UTenkaichiSettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction)));
	ApplyHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	CancelChangesHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelChangesInputActionData, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCancelChangesAction)));
}

#if 0
// ⚠️ 注释挂起：依赖 P18 UTenkaichiGameSettingRegistry
// 创建设置注册表（用 Tenkaichi 的版本）。
UGameSettingRegistry* UTenkaichiSettingScreen::CreateRegistry()
{
	UTenkaichiGameSettingRegistry* NewRegistry = NewObject<UTenkaichiGameSettingRegistry>();

	if (UTenkaichiLocalPlayer* LocalPlayer = CastChecked<UTenkaichiLocalPlayer>(GetOwningLocalPlayer()))
	{
		NewRegistry->Initialize(LocalPlayer);
	}

	return NewRegistry;
}
#endif

// 返回：先尝试弹出导航，否则应用并关闭。
void UTenkaichiSettingScreen::HandleBackAction()
{
	if (AttemptToPopNavigation())
	{
		return;
	}

	ApplyChanges();

	DeactivateWidget();
}

// 应用：应用所有改动。
void UTenkaichiSettingScreen::HandleApplyAction()
{
	ApplyChanges();
}

// 取消：撤销所有改动。
void UTenkaichiSettingScreen::HandleCancelChangesAction()
{
	CancelChanges();
}

// 脏状态变化（决定是否显示"应用/取消"绑定）。
void UTenkaichiSettingScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyHandle))
		{
			AddActionBinding(ApplyHandle);
		}
		if (!GetActionBindings().Contains(CancelChangesHandle))
		{
			AddActionBinding(CancelChangesHandle);
		}
	}
	else
	{
		RemoveActionBinding(ApplyHandle);
		RemoveActionBinding(CancelChangesHandle);
	}
}