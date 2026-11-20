// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Fonts/SlateFontInfo.h"

#include "CommonPlayerInputKey.generated.h"

#define UE_API COMMONGAME_API

enum class ECommonInputType : uint8;

class APlayerController;
class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UCommonLocalPlayer;
class UMaterialInstanceDynamic;
class UObject;
struct FFrame;
struct FGeometry;

UENUM(BlueprintType)
enum class ECommonKeybindForcedHoldStatus : uint8
{
	NoForcedHold,
	ForcedHold,
	NeverShowHold
};

USTRUCT()
struct FMeasuredText
{
	GENERATED_BODY()

public:
	FText GetText() const { return CachedText; }
	void SetText(const FText& InText);

	FVector2D GetTextSize() const { return CachedTextSize; }
	FVector2D UpdateTextSize(const FSlateFontInfo &InFontInfo, float FontScale = 1.0f) const;

private:

	FText CachedText;
	mutable FVector2D CachedTextSize;
	mutable bool bTextDirty = true;
};

UCLASS(MinimalAPI, Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class UCommonPlayerInputKey : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UE_API UCommonPlayerInputKey(const FObjectInitializer& ObjectInitializer);

	/** 根据我们当前的 Boundaction 更新按键及其关联的显示 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void UpdateKeybindWidget();

	/** 为我们的按键绑定设置绑定键 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundKey(FKey NewBoundAction);

	/** 为我们的按键绑定设置绑定动作 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetBoundAction(FName NewBoundAction);

	/** 强制这个按键绑定成为按住（hold）按键绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetForcedHoldKeybindStatus(ECommonKeybindForcedHoldStatus InForcedHoldKeybindStatus);

	/** 强制这个按键绑定成为按住（hold）按键绑定 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	UE_API void SetShowProgressCountDown(bool bShow);

	/** 为这个按键绑定设置轴缩放值 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetAxisScale(const float NewValue) { AxisScale = NewValue; }

	/** 为这个按键绑定设置预设名称覆盖值。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetPresetNameOverride(const FName NewValue) { PresetNameOverride = NewValue; }

	/** 我们当前的 BoundAction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FName BoundAction;

	/** 使用轴映射时要读取的缩放值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	float AxisScale;

	/** 这个控件绑定的按键，可直接在蓝图里设置。当我们要引用某个特定按键而不是动作时使用。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKeyFallback;

	/** 允许我们为按键绑定控件显式设置输入类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	ECommonInputType InputTypeOverride;

	/** 允许我们为按键绑定控件显式设置预设名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	FName PresetNameOverride;

	/** 设置可以让这个按键绑定显示为"按住"，或者永远不显示为"按住"（即使它确实是） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	ECommonKeybindForcedHoldStatus ForcedHoldKeybindStatus;

	/** 当我们开始按住进度时通过委托调用 */
	UFUNCTION()
	UE_API void StartHoldProgress(FName HoldActionName, float HoldDuration);

	/** 当我们停止按住进度时通过委托调用 */
	UFUNCTION()
	UE_API void StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully);

	/** 获取这个按键绑定是否是一个按住动作。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	bool IsHoldKeybind() const { return bIsHoldKeybind; }

	UFUNCTION()
	bool IsBoundKeyValid() const { return BoundKey.IsValid(); }

protected:
	UE_API virtual void NativePreConstruct() override;
	UE_API virtual void NativeConstruct() override;
	UE_API virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	UE_API void RecalculateDesiredSize();

	/** 被重写以销毁我们的 MID */
	UE_API virtual void NativeDestruct() override;

	/** 这个按键绑定控件当前是否被设置为按住按键绑定 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keybind Widget", meta=(ScriptName = "IsHoldKeybindValue"))
	bool bIsHoldKeybind;

	/**  */
	UPROPERTY(Transient)
	bool bShowKeybindBorder;

	UPROPERTY(Transient)
	FVector2D FrameSize;

	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	bool bShowTimeCountDown;

	/** 这个控件绑定到的派生按键 */
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKey;

	/** 用于显示进度的材质 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush HoldProgressBrush;

	/** 按键绑定文本的边框。 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush KeyBindTextBorder;

	/** 这个按键绑定控件是否应显示它当前未绑定的信息？ */
	UPROPERTY(EditAnywhere, Category = "Keybind Widget")
	bool bShowUnboundStatus = false;

	/** 每个尺寸下要应用的字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo KeyBindTextFont;

	/** 每个尺寸下要应用的字体 */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo CountdownTextFont;

	UPROPERTY(Transient)
	FMeasuredText CountdownText;

	UPROPERTY(Transient)
	FMeasuredText KeybindText;

	UPROPERTY(Transient)
	FMargin KeybindTextPadding;

	UPROPERTY(Transient)
	FVector2D KeybindFrameMinimumSize;

	/** HoldKeybindImage 中按住百分比的材质参数名 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FName PercentageMaterialParameterName;	

	/** 用于进度百分比的 MID */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressPercentageMID;

	UE_API virtual void NativeOnInitialized() override;

private:
	/**
	 * 将按住进度同步为所属玩家控制器当前设置的值。
	 */
	UE_API void SyncHoldProgress();

	/** 在按住按键绑定期间更新 HoldKeybindImage 时调用 */
	UE_API void UpdateHoldProgress();

	/** 当我们要把这个按键绑定控件设置为按住按键绑定时调用 */
	UE_API void SetupHoldKeybind();

	UE_API void ShowHoldBackPlate();

	UE_API void HandlePlayerControllerSet(UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);

	/** 我们开始使用按住按键绑定的时间 */
	float HoldKeybindStartTime = 0;

	/** 我们要执行按住按键绑定的时长（秒） */
	float HoldKeybindDuration = 0;

	bool bDrawProgress = false;
	bool bDrawBrushForKey = false;
	bool bDrawCountdownText = false;
	bool bWaitingForPlayerController = false;

	UPROPERTY(Transient)
	FSlateBrush CachedKeyBrush;
};

#undef UE_API
