// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"

#include "TenkaichiCharacterMovementComponent.generated.h"

#define UE_API TENKAICHIGAME_API

class UObject;
struct FFrame;

TENKAICHIGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

/**
 * FTenkaichiCharacterGroundInfo
 *
 *	关于角色脚下地面的信息。只在需要时更新。
 */
USTRUCT(BlueprintType)
struct FTenkaichiCharacterGroundInfo
{
	GENERATED_BODY()

	FTenkaichiCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * UTenkaichiCharacterMovementComponent
 *
 *	本项目使用的基础角色移动组件类。
 */
UCLASS(MinimalAPI, Config = Game)
class UTenkaichiCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	TENKAICHIGAME_API UTenkaichiCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	/** 模拟移动（网络模拟端调用）：如果外部传入了复制加速度，就跑完模拟再把它还原回去 */
	TENKAICHIGAME_API virtual void SimulateMovement(float DeltaTime) override;

	/** 能不能尝试起跳：和引擎一样，但去掉了"蹲着不能跳"的检查 */
	TENKAICHIGAME_API virtual bool CanAttemptJump() const override;

	/** 获取当前地面信息（蓝图可调）：如果数据过期就重新算一次，然后返回缓存 */
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi|CharacterMovement")
	TENKAICHIGAME_API const FTenkaichiCharacterGroundInfo& GetGroundInfo();

	/** 记录一份"要复制给模拟端"的加速度 */
	TENKAICHIGAME_API void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	/** 覆盖"转身角速度"：挂了"移动停止"tag 就转不动（返回 0） */
	TENKAICHIGAME_API virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	/** 覆盖"最大速度"：挂了"移动停止"tag 就跑不动（返回 0） */
	TENKAICHIGAME_API virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	/** 组件初始化（挂到 Pawn 上时引擎调用一次） */
	TENKAICHIGAME_API virtual void InitializeComponent() override;

protected:

	// 缓存的地面信息。别直接碰它！只有通过 GetGroundInfo() 访问时才会更新。
	FTenkaichiCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};

#undef UE_API