// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataTable.h"
#include "Widgets/GameSettingScreen.h"

#include "TenkaichiSettingScreen.generated.h"

class UGameSettingRegistry;
class UTenkaichiTabListWidgetBase;
class UObject;

// 设置屏：带标签页 + 返回/应用/取消快捷键。
UCLASS(Abstract, meta = (Category = "Settings", DisableNativeTick))
class UTenkaichiSettingScreen : public UGameSettingScreen
{
	GENERATED_BODY()

public:

protected:
	// 控件初始化（注册三个输入动作绑定）。
	virtual void NativeOnInitialized() override;
	// 创建设置注册表（用 Tenkaichi 的版本）。
	virtual UGameSettingRegistry* CreateRegistry() override;

	// 返回：先尝试弹出导航，否则应用并关闭。
	void HandleBackAction();
	// 应用：应用所有改动。
	void HandleApplyAction();
	// 取消：撤销所有改动。
	void HandleCancelChangesAction();

	// 脏状态变化（决定是否显示"应用/取消"绑定）。
	virtual void OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty) override;
	
protected:
	// 顶部设置标签页（可选绑定）。
	UPROPERTY(BlueprintReadOnly, Category = Input, meta = (BindWidget, OptionalWidget = true, AllowPrivateAccess = true))
	TObjectPtr<UTenkaichiTabListWidgetBase> TopSettingsTabs;
	
	// 返回动作的数据表行。
	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle BackInputActionData;

	// 应用动作的数据表行。
	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle ApplyInputActionData;

	// 取消动作的数据表行。
	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle CancelChangesInputActionData;

	// 返回动作的绑定句柄。
	FUIActionBindingHandle BackHandle;
	// 应用动作的绑定句柄。
	FUIActionBindingHandle ApplyHandle;
	// 取消动作的绑定句柄。
	FUIActionBindingHandle CancelChangesHandle;
};