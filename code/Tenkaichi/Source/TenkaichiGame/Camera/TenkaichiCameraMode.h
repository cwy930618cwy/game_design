#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"

#include "TenkaichiCameraMode.generated.h"

#define UE_API TENKAICHIGAME_API

class AActor;
class UCanvas;
class UTenkaichiCameraComponent;

/**
 * ETenkaichiCameraModeBlendFunction
 *
 *	相机模式之间过渡时使用的混合函数。
 */
UENUM(BlueprintType)
enum class ETenkaichiCameraModeBlendFunction : uint8
{
	// 简单的线性插值。
	Linear,

	// 立即加速，但平滑减速到目标。缓动量由指数控制。
	EaseIn,

	// 平滑加速，但不减速到目标。缓动量由指数控制。
	EaseOut,

	// 平滑加速也平滑减速。缓动量由指数控制。
	EaseInOut,

	COUNT	UMETA(Hidden)
};


/**
 * FTenkaichiCameraModeView
 *
 *	相机模式产出的视图数据，用来混合多个相机模式。
 */
struct FTenkaichiCameraModeView
{
public:

	FTenkaichiCameraModeView();

	void Blend(const FTenkaichiCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};


/**
 * UTenkaichiCameraMode
 *
 *	所有相机模式的基类。
 */
UCLASS(MinimalAPI, Abstract, NotBlueprintable)
class UTenkaichiCameraMode : public UObject
{
	GENERATED_BODY()

public:

	UE_API UTenkaichiCameraMode();

	UE_API UTenkaichiCameraComponent* GetTenkaichiCameraComponent() const;

	UE_API virtual UWorld* GetWorld() const override;

	UE_API AActor* GetTargetActor() const;

	const FTenkaichiCameraModeView& GetCameraModeView() const { return View; }

	// 当这个相机模式在模式栈上被激活时调用。
	virtual void OnActivation() {};

	// 当这个相机模式在模式栈上被取消激活时调用。
	virtual void OnDeactivation() {};

	UE_API void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	UE_API void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

	UE_API virtual void DrawDebug(UCanvas* Canvas) const;

protected:

	UE_API virtual FVector GetPivotLocation() const;
	UE_API virtual FRotator GetPivotRotation() const;

	UE_API virtual void UpdateView(float DeltaTime);
	UE_API virtual void UpdateBlending(float DeltaTime);

protected:
	// 一个标签，游戏逻辑可以用它查询"当前是哪种相机模式在起作用"，
	// 而不必问具体是哪个模式（比如瞄准时想要更高精度）。
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// 相机模式产出的视图输出。
	FTenkaichiCameraModeView View;

	// 水平视野（单位：度）。
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// 最小视野俯仰角（单位：度）。
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// 最大视野俯仰角（单位：度）。
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// 混合进入这个模式需要多长时间。
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// 混合使用的函数。
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	ETenkaichiCameraModeBlendFunction BlendFunction;

	// 混合函数用来控制曲线形状的指数。
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// 用于计算混合权重的线性混合 alpha。
	float BlendAlpha;

	// 用混合 alpha 和函数计算出的混合权重。
	float BlendWeight;

protected:
	/** 如果为 true，跳过所有插值，把相机放到理想位置。下一帧自动设回 false。 */
	UPROPERTY(transient)
	uint32 bResetInterpolation:1;
};


/**
 * UTenkaichiCameraModeStack
 *
 *	用于混合相机模式的栈。
 */
UCLASS()
class UTenkaichiCameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UTenkaichiCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UTenkaichiCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FTenkaichiCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// 获取最顶层模式对应的标签及其混合权重。
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	UTenkaichiCameraMode* GetCameraModeInstance(TSubclassOf<UTenkaichiCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FTenkaichiCameraModeView& OutCameraModeView) const;

protected:

	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UTenkaichiCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UTenkaichiCameraMode>> CameraModeStack;
};

#undef UE_API