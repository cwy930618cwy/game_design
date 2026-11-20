// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "UObject/ObjectKey.h"

#include "GameFeatureAction_SplitscreenConfig.generated.h"

class UObject;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_SplitscreenConfig

/**
 * 负责配置分屏的 GameFeatureAction。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Splitscreen Config"))
class UGameFeatureAction_SplitscreenConfig final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性停用时调用：撤掉本 Action 投的票。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~UGameFeatureAction 接口结束

	//~UGameFeatureAction_WorldActionBase 接口开始
	// 在某个 world 里执行：给 viewport 投票禁用分屏。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~UGameFeatureAction_WorldActionBase 接口结束

public:
	// 是否禁用分屏。
	UPROPERTY(EditAnywhere, Category=Action)
	bool bDisableSplitscreen = true;

private:
	// 本 Action 投过票的 viewport 列表。
	TArray<FObjectKey> LocalDisableVotes;

	// 全局投票表：viewport → 票数（静态，所有 Action 实例共享）。
	static TMap<FObjectKey, int32> GlobalDisableVotes;
};