#pragma once

#include "CoreMinimal.h"

#include "TenkaichiPenetrationAvoidanceFeeler.generated.h"

/**
 * 定义一个用于相机防穿透的探测射线。
 */
USTRUCT()
struct FTenkaichiPenetrationAvoidanceFeeler
{
	GENERATED_BODY()

	// 描述相对主射线的偏转角度。
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	FRotator AdjustmentRot;

	// 这个 feeler 命中世界物体时对最终位置的影响权重。
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float WorldWeight;

	// 这个 feeler 命中 APawn（角色）时对最终位置的影响权重（设为 0 则完全不与 Pawn 碰撞）。
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float PawnWeight;

	// 探测这条 feeler 时使用的碰撞范围。
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	float Extent;

	// 上一帧什么都没命中时，隔多少帧再探测这条 feeler。
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	int32 TraceInterval;

	// 距离下次探测还有多少帧。
	UPROPERTY(transient)
	int32 FramesUntilNextTrace;


	// 默认构造函数：所有成员归零。
	FTenkaichiPenetrationAvoidanceFeeler()
		: AdjustmentRot(ForceInit)
		, WorldWeight(0)
		, PawnWeight(0)
		, Extent(0)
		, TraceInterval(0)
		, FramesUntilNextTrace(0)
	{
	}

	// 带参构造函数：一次性给全部成员赋值。
	FTenkaichiPenetrationAvoidanceFeeler(const FRotator& InAdjustmentRot,
									const float& InWorldWeight,
									const float& InPawnWeight,
									const float& InExtent,
									const int32& InTraceInterval = 0,
									const int32& InFramesUntilNextTrace = 0)
		: AdjustmentRot(InAdjustmentRot)
		, WorldWeight(InWorldWeight)
		, PawnWeight(InPawnWeight)
		, Extent(InExtent)
		, TraceInterval(InTraceInterval)
		, FramesUntilNextTrace(InFramesUntilNextTrace)
	{
	}
};