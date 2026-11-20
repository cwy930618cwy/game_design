// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonActivatableWidget.h"

#include "TenkaichiActivatableWidget.generated.h"

struct FUIInputConfig;

// 控件激活时的输入模式。
UENUM(BlueprintType)
enum class ETenkaichiWidgetInputMode : uint8
{
	Default,          // 用父类默认行为
	GameAndMenu,      // 游戏和菜单都能接收输入
	Game,             // 只有游戏能接收输入
	Menu              // 只有菜单能接收输入
};

// 一个"激活时自动配置期望输入模式"的可激活控件。
UCLASS(Abstract, Blueprintable)
class UTenkaichiActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// 构造函数：空实现（父类处理初始化）。
	UTenkaichiActivatableWidget(const FObjectInitializer& ObjectInitializer);
	
public:
	
	//~UCommonActivatableWidget 接口开始
	// 返回激活时期望的输入配置（按 InputConfig 决定）。
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~UCommonActivatableWidget 接口结束

#if WITH_EDITOR
	// 编辑器校验：检查蓝图是否实现了 BP_GetDesiredFocusTarget。
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	/** 这个 UI 激活时期望使用的输入模式，比如你希望按键仍然能传到游戏/玩家控制器吗？ */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	ETenkaichiWidgetInputMode InputConfig = ETenkaichiWidgetInputMode::Default;

	/** 游戏获得输入时期望的鼠标行为。 */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};