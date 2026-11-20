#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

#include "TenkaichiCameraComponent.generated.h"

class UCanvas;
class UTenkaichiCameraMode;
class UTenkaichiCameraModeStack;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass> class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<UTenkaichiCameraMode>, FTenkaichiCameraModeDelegate);


/**
 * UTenkaichiCameraComponent
 *
 *	本项目使用的基础相机组件类。
 */
UCLASS()
class TENKAICHIGAME_API UTenkaichiCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:

	UTenkaichiCameraComponent(const FObjectInitializer& ObjectInitializer);

	// 如果指定 Actor 上存在相机组件，返回它。
	UFUNCTION(BlueprintPure, Category = "Tenkaichi|Camera")
	static UTenkaichiCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UTenkaichiCameraComponent>() : nullptr); }

	// 返回相机正在观察的目标 Actor。
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// 用来查询"当前最佳相机模式"的委托。
	FTenkaichiCameraModeDelegate DetermineCameraModeDelegate;

	// 给视野加一个偏移。偏移只生效一帧，应用后会被清掉。
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	virtual void DrawDebug(UCanvas* Canvas) const;

	// 获取最顶层模式对应的标签及其混合权重。
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

protected:

	// 用来混合相机模式的栈。
	UPROPERTY()
	TObjectPtr<UTenkaichiCameraModeStack> CameraModeStack;

	// 应用到视野的偏移。偏移只生效一帧，应用后会被清掉。
	float FieldOfViewOffset;

};