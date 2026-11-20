// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"

#include "TenkaichiPlayerStart.generated.h"

#define UE_API TENKAICHIGAME_API

class AController;
class UObject;

// 出生点的占用状态：空 / 部分被占 / 满。
enum class ETenkaichiPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};

/**
 * ATenkaichiPlayerStart
 *
 * 很多玩法模式都能用的基础玩家出生点。
 */
UCLASS(MinimalAPI, Config = Game)
class ATenkaichiPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ATenkaichiPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 读取这个出生点的标签集合。
	const FGameplayTagContainer& GetGameplayTags() { return StartPointTags; }

	// 检测这个出生点现在能不能放得下一个指定控制器要操控的 Pawn。
	UE_API ETenkaichiPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** 这个出生点是否已经被某个控制器认领了？ */
	UE_API bool IsClaimed() const;

	/** 如果没被认领，就把它认领给指定控制器 */
	UE_API bool TryClaim(AController* OccupyingController);

protected:
	/** 检查这个出生点是否还被占着（定时器回调） */
	UE_API void CheckUnclaimed();

	/** 认领了这个出生点的控制器 */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	/** 每隔多久检查一次这个出生点是否已经没人在附近了 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** 用来标识这个出生点的标签 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** 记录过期检查的循环定时器句柄 */
	FTimerHandle ExpirationTimerHandle;
};

#undef UE_API