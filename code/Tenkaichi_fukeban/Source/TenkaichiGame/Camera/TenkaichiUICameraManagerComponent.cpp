// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiUICameraManagerComponent.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "TenkaichiPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiUICameraManagerComponent)

class AActor;
class FDebugDisplayInfo;

// 静态：从玩家控制器找到 UI 相机组件。
UTenkaichiUICameraManagerComponent* UTenkaichiUICameraManagerComponent::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (ATenkaichiPlayerCameraManager* PCCamera = Cast<ATenkaichiPlayerCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}

	return nullptr;
}

// 构造函数：注册 showdebug 钩子。
UTenkaichiUICameraManagerComponent::UTenkaichiUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// 注册 showdebug 钩子。
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

// 组件初始化钩子（覆盖引擎）。
void UTenkaichiUICameraManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// 设置视图目标（转发给主相机管理器）。
void UTenkaichiUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<ATenkaichiPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

// 是否需要更新视图目标（UI 相机要不要接管）。
bool UTenkaichiUICameraManagerComponent::NeedsToUpdateViewTarget() const
{
	return false;
}

// UI 相机更新视图目标。
void UTenkaichiUICameraManagerComponent::UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime)
{
}

// showdebug 时显示调试信息。
void UTenkaichiUICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}