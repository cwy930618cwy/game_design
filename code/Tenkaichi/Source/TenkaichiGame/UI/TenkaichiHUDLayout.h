// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "TenkaichiActivatableWidget.h"
#include "Containers/Ticker.h"
#include "GameplayTagContainer.h"

#include "TenkaichiHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;
class UTenkaichiControllerDisconnectedScreen;

/**
 * UTenkaichiHUDLayout
 *
 *	用来布局玩家 HUD 的控件（通常由经验里的 Add Widgets 动作指定）。
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "Tenkaichi HUD Layout", Category = "Tenkaichi|HUD"))
class UTenkaichiHUDLayout : public UTenkaichiActivatableWidget
{
	GENERATED_BODY()

public:

	// 构造函数：加默认的平台 trait tag。
	UTenkaichiHUDLayout(const FObjectInitializer& ObjectInitializer);

	// 控件初始化（注册 Esc 动作 + 绑手柄事件）。
	virtual void NativeOnInitialized() override;
	// 控件销毁（解绑手柄事件 + 移除 Ticker）。
	virtual void NativeDestruct() override;

protected:
	// 处理 Esc 动作：推暂停菜单。
	void HandleEscapeAction();
	
	/** 
	* 控制器断开时的回调。这会检查玩家现在是否已经没有映射到它的输入设备了，
	* 这意味着他们无法玩游戏。
	* 
	* 如果是这种情况，就调用 DisplayControllerDisconnectedMenu。
	*/
	void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	/**
	* 控制器改变其所属平台用户时的回调。我们用这个来检查
	* 是否不再需要显示"控制器已断开"菜单。
	*/
	void HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId);
	
	/**
	* 通知这个控件：玩家的控制器状态变了。下一帧排队一个计时器来
	* 处理它们，看是否需要显示/隐藏"控制器已断开"控件。
	*/
	void NotifyControllerStateChangeForDisconnectScreen();

	/**
	 * 这会检查连接到玩家的控制器状态。如果它们没有连接任何控制器，
	 * 我们就应该显示断开菜单。如果它们有连接的控制器，我们就可以
	 * 隐藏断开菜单（如果正在显示的话）。
	 */
	virtual void ProcessControllerDevicesHavingChangedForDisconnectScreen();

	/**
     * 如果这个平台支持"控制器已断开"屏，返回 true。
     */
    virtual bool ShouldPlatformDisplayControllerDisconnectScreen() const;
	
	/**
	* 把 ControllerDisconnectedMenuClass 推到菜单层（UI.Layer.Menu）。
	*/
	UFUNCTION(BlueprintNativeEvent, Category="Controller Disconnect Menu")
	void DisplayControllerDisconnectedMenu();

	/**
	* 如果控制器断开菜单是激活的，就隐藏它。
	*/
	UFUNCTION(BlueprintNativeEvent, Category="Controller Disconnect Menu")
	void HideControllerDisconnectedMenu();
	
	/**
	 * 当用户按下"暂停"或"Esc"按钮时要显示的菜单。
	 */
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;

	/** 
	* 如果用户所有控制器都断开了，应该呈现给用户的控件。
	*/
	UPROPERTY(EditDefaultsOnly, Category="Controller Disconnect Menu")
	TSubclassOf<UTenkaichiControllerDisconnectedScreen> ControllerDisconnectedScreen;

	/**
	 * 为了显示"控制器已断开"屏所需的平台 tag。
	 *
	 * 如果这些 tag 没有在这个平台的 INI 文件里设置，那么控制器断开屏
	 * 将永远不会显示。
	 */
	UPROPERTY(EditDefaultsOnly, Category="Controller Disconnect Menu")
	FGameplayTagContainer PlatformRequiresControllerDisconnectScreen;

	/** 指向当前激活的"控制器已断开"菜单（如果有的话）。 */
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> SpawnedControllerDisconnectScreen;

	/** FTSTicker 的句柄，用于我们想要处理玩家控制器状态时。 */
	FTSTicker::FDelegateHandle RequestProcessControllerStateHandle;
};