#pragma once

#include "TenkaichiCameraMode.h"
#include "Curves/CurveFloat.h"
#include "TenkaichiPenetrationAvoidanceFeeler.h"
#include "DrawDebugHelpers.h"
#include "TenkaichiCameraMode_ThirdPerson.generated.h"

class UCurveVector;

/**
 * UTenkaichiCameraMode_ThirdPerson
 *
 *	一个基础的第三人称相机模式。
 */
UCLASS(Abstract, Blueprintable)
class UTenkaichiCameraMode_ThirdPerson : public UTenkaichiCameraMode
{
	GENERATED_BODY()

public:

	// 构造函数。
	UTenkaichiCameraMode_ThirdPerson();

protected:

	// 覆盖基类的视图更新，加入第三人称偏移、蹲伏偏移和防穿墙逻辑。
	virtual void UpdateView(float DeltaTime) override;

	// 检测目标是否下蹲，并设置对应的蹲伏偏移目标。
	void UpdateForTarget(float DeltaTime);

	// 防穿墙入口：计算安全点并调整相机位置避免穿墙。
	void UpdatePreventPenetration(float DeltaTime);

	// 核心防穿墙射线检测：用多个 feeler 射线打点，算出被挡比例并平滑收镜。
	void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

	// 覆盖调试绘制：打印被撞到的 Actor。
	virtual void DrawDebug(UCanvas* Canvas) const override;

protected:

	// 用视图俯仰角求值的曲线，用来定义相对目标的本地空间偏移。
	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
	TObjectPtr<const UCurveVector> TargetOffsetCurve;

	// UE-103986：PIE 期间实时编辑 RuntimeFloatCurves 不生效（不像曲线资产）。
	// 该问题解决后它将变成默认方式，TargetOffsetCurve 将被移除。
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	bool bUseRuntimeFloatCurves;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetZ;

	// 改变蹲伏偏移淡入/淡出的速度。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float CrouchOffsetBlendMultiplier = 5.0f;

	// 防穿墙相关
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendOutTime = 0.15f;

	/** 如果为真，做碰撞检测让相机不会穿进世界物体里。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bPreventPenetration = true;

	/** 如果为真，尝试提前探测附近的墙并提前移动相机，帮助避免弹跳。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bDoPredictiveAvoidance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionPushOutDistance = 2.f;

	/** 当相机距离因穿墙被推进到其完整距离的这个百分比时 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float ReportPenetrationPercent = 0.f;

	/**
	 * 这些是用于找到相机放置位置的探测射线。
	 * 索引 0：用来防止碰撞的常规探测射线。
	 * 索引 1+：当 bDoPredictiveAvoidance=true 时使用这些射线，扫描玩家向那个方向旋转时可能的撞击，
	 *           并提前把相机拉进来，避免撞击到遮挡物。
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FTenkaichiPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:

	// 设置蹲伏偏移的目标值并重置过渡。
	void SetTargetCrouchOffset(FVector NewTargetOffset);

	// 每帧平滑推进蹲伏偏移的过渡。
	void UpdateCrouchOffset(float DeltaTime);

	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	float CrouchOffsetBlendPct = 1.0f;
	FVector CurrentCrouchOffset = FVector::ZeroVector;

};