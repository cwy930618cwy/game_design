// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedPlayerInput.h"

#include "TenkaichiPlayerInput.generated.h"

/**
 * Tenkaichi 的自定义玩家输入类。它在 Enhanced Input 的基础上，额外追踪按键按下时的输入延迟。
 *
 * 如果你有任何想在按键按下或输入刷新时运行的逻辑，就继承这个类。
 */
UCLASS(config = Input, transient)
class UTenkaichiPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()

public:
	UTenkaichiPlayerInput();
	virtual ~UTenkaichiPlayerInput() override;

protected:
	//~ UEnhancedPlayerInput 接口开始
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;
	//~ UEnhancedPlayerInput 接口结束

	// 为延迟标记处理本次按键事件
	void ProcessInputEventForLatencyMarker(const FInputKeyEventArgs& Params);
	// 绑定到延迟标记设置变更事件
	void BindToLatencyMarkerSettingChange();
	// 解绑延迟标记设置变更监听
	void UnbindLatencyMarkerSettingChangeListener();
	// 处理延迟标记设置变更（开关各延迟模块的闪光）
	void HandleLatencyMarkerSettingChanged();

	// 是否应触发延迟闪光
	bool bShouldTriggerLatencyFlash = false;
};