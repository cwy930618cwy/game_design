// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiPlayerInput.h"
#include "Performance/LatencyMarkerModule.h"
#include "Settings/TenkaichiSettingsLocal.h"

#if 0 // 依赖 P18 TenkaichiSettingsLocal，暂挂起，就位后解开（见注释备忘录 ⑲）

UTenkaichiPlayerInput::UTenkaichiPlayerInput()
	: Super()
{
	// 不要在 CDO（类默认对象）上绑定任何设置委托，否则会有一个一直挂着的监听，
	// 而且它不会被 tick、也不处理输入，等于白挂。
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}
	
	// 绑定延迟标记设置变更
	BindToLatencyMarkerSettingChange();
}

UTenkaichiPlayerInput::~UTenkaichiPlayerInput()
{
	// 析构时解绑监听，避免悬空委托
	UnbindLatencyMarkerSettingChangeListener();
}

bool UTenkaichiPlayerInput::InputKey(const FInputKeyEventArgs& Params)
{
	const bool bResult = Super::InputKey(Params);

	// 注：既然 Tenkaichi 只打算用 "Reflex" 插件来管延迟标记，
	// 理论上可以用 #if PLATFORM_DESKTOP 把这段裁掉以省其他平台的开销。
	// 但为了本项目的可扩展性，这里不做这个裁剪。
	ProcessInputEventForLatencyMarker(Params);

	return bResult;
}

void UTenkaichiPlayerInput::ProcessInputEventForLatencyMarker(const FInputKeyEventArgs& Params)
{
	// 设置没开延迟闪光就直接返回
	if (!bShouldTriggerLatencyFlash)
	{
		return;
	}
	
	// 鼠标左键按下时，闪一下延迟标记
	if (Params.Key == EKeys::LeftMouseButton)
	{
		// 拿到所有注册的延迟标记模块
		TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());

		for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
		{
			// TRIGGER_FLASH 是 7
			LatencyMarkerModule->SetCustomLatencyMarker(7, GFrameCounter);
		}
	}
}

void UTenkaichiPlayerInput::BindToLatencyMarkerSettingChange()
{
	// 平台不支持延迟标记就不绑
	if (!UTenkaichiSettingsLocal::DoesPlatformSupportLatencyMarkers())
	{
		return;
	}
	
	UTenkaichiSettingsLocal* Settings = UTenkaichiSettingsLocal::Get();
	if (!Settings)
	{
		return;
	}

	// 订阅延迟闪光指示器设置变更事件，并立即初始化一次
	Settings->OnLatencyFlashInidicatorSettingsChangedEvent().AddUObject(this, &ThisClass::HandleLatencyMarkerSettingChanged);

	// 初始化设置，并确保输入延迟模块已启用
	HandleLatencyMarkerSettingChanged();
}

void UTenkaichiPlayerInput::UnbindLatencyMarkerSettingChangeListener()
{
	UTenkaichiSettingsLocal* Settings = UTenkaichiSettingsLocal::Get();
	if (!Settings)
	{
		return;
	}

	// 解绑本对象的所有监听
	Settings->OnLatencyFlashInidicatorSettingsChangedEvent().RemoveAll(this);
}

void UTenkaichiPlayerInput::HandleLatencyMarkerSettingChanged()
{
	// 确保我们只会在支持延迟标记的平台上收到这个回调
	ensure(UTenkaichiSettingsLocal::DoesPlatformSupportLatencyMarkers());
	
	const UTenkaichiSettingsLocal* Settings = UTenkaichiSettingsLocal::Get();
	if (!Settings)
	{
		return;
	}

	// 按设置变更，开启或关闭所有标记模块上的延迟闪光
	bShouldTriggerLatencyFlash = Settings->GetEnableLatencyFlashIndicators();
	
	TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());
	for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
	{
		LatencyMarkerModule->SetFlashIndicatorEnabled(bShouldTriggerLatencyFlash);
	}
}

#endif // 依赖 P18 的挂起结束