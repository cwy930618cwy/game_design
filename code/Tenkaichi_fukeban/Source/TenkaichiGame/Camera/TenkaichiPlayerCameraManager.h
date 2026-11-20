#pragma once

#include "Camera/PlayerCameraManager.h"

#include "TenkaichiPlayerCameraManager.generated.h"

class FDebugDisplayInfo;
class UCanvas;
class UObject;


#define TENKAICHI_CAMERA_DEFAULT_FOV		(80.0f)
#define TENKAICHI_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define TENKAICHI_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

class UTenkaichiUICameraManagerComponent;

/**
 * ATenkaichiPlayerCameraManager
 *
 *	本项目使用的基础玩家相机管理器类。
 */
UCLASS(notplaceable, MinimalAPI)
class ATenkaichiPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	// 构造函数。
	ATenkaichiPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	// 返回 UI 相机组件。
	UTenkaichiUICameraManagerComponent* GetUICameraComponent() const;

protected:

	// 覆盖引擎：更新视图目标（UI 相机要接管时优先让给它）。
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	// 覆盖引擎：showdebug 时打印相机调试信息。
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

private:
	// UI 相机组件，当 UI 在做某些比玩法更重要的、需要控制相机的事情时接管相机。
	UPROPERTY(Transient)
	TObjectPtr<UTenkaichiUICameraManagerComponent> UICamera;
};