// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"

#include "TenkaichiControllerDisconnectedScreen.generated.h"

class UHorizontalBox;
class UObject;
class UCommonButtonBase;
struct FPlatformUserSelectionCompleteParams;

/**
 * 当用户所有手柄都断开、需要重新连接才能继续游戏时显示的屏。
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class UTenkaichiControllerDisconnectedScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
public:
	// 构造函数：加默认要求的平台 trait tag。
	UTenkaichiControllerDisconnectedScreen(const FObjectInitializer& ObjectInitializer);
	
protected:
	// 控件激活时的回调（设置"切换用户"按钮的可见性）。
	virtual void NativeOnActivated() override;

	// 点击"切换用户"按钮时的回调。
	virtual void HandleChangeUserClicked();

	/**
	 * 当用户在选择了"切换平台用户"提示之后完成切换时调用。
	 */
	virtual void HandleChangeUserCompleted(const FPlatformUserSelectionCompleteParams& Params);

	/**
	 * 如果应该显示"切换用户"按钮，返回 true。
	 * 这会在运行时检查 ICommonUIModule 的平台 trait tag。
	 */
	virtual bool ShouldDisplayChangeUserButton() const;

	/**
	 * 要求的平台 trait——满足时显示"切换用户"按钮，
	 * 允许玩家改变当前映射到输入设备的已登录用户。
	 */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer PlatformSupportsUserChangeTags;

	/**
	 * 有"严格"用户配对要求的平台，可能想允许你直接从游戏内 UI 里切换用户。
	 * 这些平台在 CommonUI 里被打上 "Platform.Trait.Input.HasStrictControllerPairing" 标记。
	 *
	 * 如果所在平台没有该 trait，这个 HBox 会被设成不可见。
	 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_SwitchUser;

	/**
	* 一个按钮，用于在有严格用户配对要求的平台上切换用户。
	* 
	* @see HBox_SwitchUser
	*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_ChangeUser;
};