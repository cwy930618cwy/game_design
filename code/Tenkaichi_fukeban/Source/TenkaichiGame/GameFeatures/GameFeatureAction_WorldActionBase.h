// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"

#include "GameFeatureAction_WorldActionBase.generated.h"

class FDelegateHandle;
class UGameInstance;
class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

/**
 * 那些希望做"世界特定"事情的 GameFeatureAction 的基类。
 */
UCLASS(Abstract)
class UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性激活时调用。
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	// 游戏特性停用时调用。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~UGameFeatureAction 接口结束

private:
	// GameInstance 启动时的回调（补调 AddToWorld）。
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	// 用动作特定的逻辑覆盖这个函数。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::AddToWorld,);

private:
	// 每个状态变化上下文对应的 GameInstance 启动回调句柄。
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};