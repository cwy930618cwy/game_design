// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction.h"

#include "ApplyFrontendPerfSettingsAction.generated.h"

class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;

//////////////////////////////////////////////////////////////////////
// UApplyFrontendPerfSettingsAction

/**
 * 负责告诉用户设置"应用前端/菜单专用的性能设置"的 GameFeatureAction
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Use Frontend Perf Settings"))
class UApplyFrontendPerfSettingsAction final : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性激活时调用（计数 +1，首次时应用前端设置）。
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	// 游戏特性停用时调用（计数 -1，归零时恢复）。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~UGameFeatureAction 接口结束

private:
	// 应用计数（静态，所有实例共享）。
	static int32 ApplicationCounter;
};