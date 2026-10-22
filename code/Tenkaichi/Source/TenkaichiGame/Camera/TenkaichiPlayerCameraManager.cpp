// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiPlayerCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TenkaichiCameraComponent.h"
#include "TenkaichiUICameraManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerCameraManager)

class FDebugDisplayInfo;

static FName UICameraComponentName(TEXT("UICamera"));

// 构造函数：设置默认相机参数，并创建 UI 相机组件。
ATenkaichiPlayerCameraManager::ATenkaichiPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = TENKAICHI_CAMERA_DEFAULT_FOV;
	ViewPitchMin = TENKAICHI_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = TENKAICHI_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<UTenkaichiUICameraManagerComponent>(UICameraComponentName);
}

// 返回 UI 相机组件。
UTenkaichiUICameraManagerComponent* ATenkaichiPlayerCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

// 覆盖引擎：更新视图目标（如果 UI 相机在观察某物，让它优先接管）。
void ATenkaichiPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// 如果 UI 相机正在观察某物，让它优先接管。
	if (UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

// 覆盖引擎：showdebug 时打印相机调试信息。
void ATenkaichiPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("TenkaichiPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UTenkaichiCameraComponent* CameraComponent = UTenkaichiCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}