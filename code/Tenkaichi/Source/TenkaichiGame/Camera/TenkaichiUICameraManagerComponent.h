#pragma once

#include "Camera/PlayerCameraManager.h"

#include "TenkaichiUICameraManagerComponent.generated.h"

class ATenkaichiPlayerCameraManager;

class AActor;
class AHUD;
class APlayerController;
class FDebugDisplayInfo;
class UCanvas;
class UObject;

UCLASS( Transient, Within=TenkaichiPlayerCameraManager )
class UTenkaichiUICameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 静态：从玩家控制器找到 UI 相机组件。
	static UTenkaichiUICameraManagerComponent* GetComponent(APlayerController* PC);

public:
	// 构造函数。
	UTenkaichiUICameraManagerComponent();
	// 组件初始化钩子（覆盖引擎）。
	virtual void InitializeComponent() override;

	// 是否正在设置视图目标。
	bool IsSettingViewTarget() const { return bUpdatingViewTarget; }
	// 返回当前视图目标。
	AActor* GetViewTarget() const { return ViewTarget; }
	// 设置视图目标（转发给主相机管理器）。
	void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	// 是否需要更新视图目标（UI 相机要不要接管）。
	bool NeedsToUpdateViewTarget() const;
	// UI 相机更新视图目标。
	void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

	// showdebug 时显示调试信息。
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

private:
	// 当前 UI 相机看向的目标。
	UPROPERTY(Transient)
	TObjectPtr<AActor> ViewTarget;

	// 是否正在更新视图目标（防重入标记）。
	UPROPERTY(Transient)
	bool bUpdatingViewTarget;
};