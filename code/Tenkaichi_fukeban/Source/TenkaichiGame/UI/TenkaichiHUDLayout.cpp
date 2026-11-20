// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiHUDLayout.h"

#include "CommonUIExtensions.h"
#include "CommonUISettings.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Input/CommonUIInputTypes.h"
#include "ICommonUIModule.h"
#include "TenkaichiLogChannels.h"
#include "NativeGameplayTags.h"
#include "UI/Foundation/TenkaichiControllerDisconnectedScreen.h"
#include "UI/TenkaichiActivatableWidget.h"

#if WITH_EDITOR
#include "CommonUIVisibilitySubsystem.h"
#endif	// WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHUDLayout)

// 定义 3 个本地静态 GameplayTag。
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_ACTION_ESCAPE, "UI.Action.Escape");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_PrimarlyController, "Platform.Trait.Input.PrimarlyController");

// 构造函数：加默认的平台 trait tag。
UTenkaichiHUDLayout::UTenkaichiHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpawnedControllerDisconnectScreen(nullptr)
{
	// 默认只有"主要以手柄输入"的平台才需要断线屏。
	PlatformRequiresControllerDisconnectScreen.AddTag(TAG_Platform_Trait_Input_PrimarlyController);
}

// 控件初始化（注册 Esc 动作 + 绑手柄事件）。
void UTenkaichiHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 注册 Esc 输入动作 → HandleEscapeAction
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));

	// 如果这个平台能显示控制器断线屏，就监听控制器状态变化委托
	if (ShouldPlatformDisplayControllerDisconnectScreen())
	{
		// 绑到输入设备连接变化
		IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
		DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);
		DeviceMapper.GetOnInputDevicePairingChange().AddUObject(this, &ThisClass::HandleInputDevicePairingChanged);	
	}
}

// 控件销毁（解绑手柄事件 + 移除 Ticker）。
void UTenkaichiHUDLayout::NativeDestruct()
{
	Super::NativeDestruct();

	// 移除输入设备连接变化的绑定
	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);
	DeviceMapper.GetOnInputDevicePairingChange().RemoveAll(this);

	// 如果还有排队的 Ticker，移除它
	if (RequestProcessControllerStateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RequestProcessControllerStateHandle);
		RequestProcessControllerStateHandle.Reset();
	}
}

// 处理 Esc 动作：推暂停菜单。
void UTenkaichiHUDLayout::HandleEscapeAction()
{
	// 确保配置了暂停菜单类
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass);
	}
}

// 手柄连接变化回调：检查断线。
void UTenkaichiHUDLayout::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();

	ensure(OwningLocalPlayerId.IsValid());

	// 这个设备变化是别的玩家的，跟我们无关，忽略。
	if (PlatformUserId != OwningLocalPlayerId)
	{
		return;
	}

	NotifyControllerStateChangeForDisconnectScreen();
}

// 手柄配对变化回调：如果和本玩家相关就检查断线。
void UTenkaichiHUDLayout::HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();

	ensure(OwningLocalPlayerId.IsValid());

	// 如果这次配对变化和我们的本地玩家相关，就通知有变化。
	if (NewUserPlatformId == OwningLocalPlayerId || OldUserPlatformId == OwningLocalPlayerId)
	{
		NotifyControllerStateChangeForDisconnectScreen();	
	}
}

// 平台是否应该显示"控制器已断开"屏。
bool UTenkaichiHUDLayout::ShouldPlatformDisplayControllerDisconnectScreen() const
{
	// 我们只想在主要以手柄输入的平台上有这个菜单
	bool bHasAllRequiredTags = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll(PlatformRequiresControllerDisconnectScreen);

	// 也检查我们在编辑器里可能模拟的 tag
#if WITH_EDITOR
	const FGameplayTagContainer& PlatformEmulationTags = UCommonUIVisibilitySubsystem::Get(GetOwningLocalPlayer())->GetVisibilityTags();
	bHasAllRequiredTags |= PlatformEmulationTags.HasAll(PlatformRequiresControllerDisconnectScreen);
#endif	// WITH_EDITOR

	return bHasAllRequiredTags;
}

// 通知控制器状态变化：下一帧处理（用 FTSTicker 排队一次）。
void UTenkaichiHUDLayout::NotifyControllerStateChangeForDisconnectScreen()
{
	// 我们只应该在绑定了控制器状态变化委托时才会走到这里
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());

	// 如果还没排队，就给下一帧排队处理设备状态。
	if (!RequestProcessControllerStateHandle.IsValid())
	{
		RequestProcessControllerStateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
		{
			RequestProcessControllerStateHandle.Reset();
			ProcessControllerDevicesHavingChangedForDisconnectScreen();
			return false;   // 返回 false = 只执行一次
		}));
	}
}

// 检查玩家的控制器状态，决定显示还是隐藏断线屏。
void UTenkaichiHUDLayout::ProcessControllerDevicesHavingChangedForDisconnectScreen()
{
	// 我们只应该在绑定了控制器状态变化委托时才会走到这里
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());
	
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	
	ensure(OwningLocalPlayerId.IsValid());

	// 拿到映射到本玩家的所有输入设备
	const IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
    TArray<FInputDeviceId> MappedInputDevices;
    const int32 NumDevicesMappedToUser = DeviceMapper.GetAllInputDevicesForUser(OwningLocalPlayerId, OUT MappedInputDevices);

    // 检查映射到这个平台用户的所有已连接手柄设备里，还有没有别的
    bool bHasConnectedController = false;

    for (const FInputDeviceId MappedDevice : MappedInputDevices)
    {
    	if (DeviceMapper.GetInputDeviceConnectionState(MappedDevice) == EInputDeviceConnectionState::Connected)
    	{
    		const FHardwareDeviceIdentifier HardwareInfo = UInputDeviceSubsystem::Get()->GetInputDeviceHardwareIdentifier(MappedDevice);
    		if (HardwareInfo.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad)
    		{
    			bHasConnectedController = true;
    		}
    	}			
    }

    // 如果没有手柄设备映射到这个用户，我们想弹出一个提示让他们重新连上
    if (!bHasConnectedController)
    {
    	DisplayControllerDisconnectedMenu();
    }
	// 否则如果当前正在显示断线屏，就隐藏它
	else if (SpawnedControllerDisconnectScreen)
	{
		HideControllerDisconnectedMenu();
	}
}

// 显示控制器断线菜单（C++ 默认实现，蓝图可覆写）。
void UTenkaichiHUDLayout::DisplayControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogTenkaichi, Log, TEXT("[%hs] Display controller disconnected menu!"), __func__);

	if (ControllerDisconnectedScreen)
	{
		// 把"控制器已断开"控件推到菜单层
		SpawnedControllerDisconnectScreen = UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, ControllerDisconnectedScreen);
	}
}

// 隐藏控制器断线菜单（C++ 默认实现，蓝图可覆写）。
void UTenkaichiHUDLayout::HideControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogTenkaichi, Log, TEXT("[%hs] Hide controller disconnected menu!"), __func__);
	
	UCommonUIExtensions::PopContentFromLayer(SpawnedControllerDisconnectScreen);
	SpawnedControllerDisconnectScreen = nullptr;
}