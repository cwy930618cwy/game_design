// 版权声明（可保留或省略，不影响编译）
#include "ApplyFrontendPerfSettingsAction.h"

//#include "Settings/TenkaichiSettingsLocal.h"   // ⚠️ 注释挂起：依赖 P18 Settings

#include UE_INLINE_GENERATED_CPP_BY_NAME(ApplyFrontendPerfSettingsAction)

struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;

//////////////////////////////////////////////////////////////////////
// UApplyFrontendPerfSettingsAction

// 游戏用户设置（以及它们驱动的引擎性能/可伸缩性设置）是全局的，
// 所以对多人 PIE 来说没必要按 world 追踪：
// 只要有任何一个 PIE world 在菜单里，我们就应用它。
//
// 不过默认情况下，我们不会在编辑器里应用前端性能设置，
// 除非开发者设置 ApplyFrontEndPerformanceOptionsInPIE 被启用
int32 UApplyFrontendPerfSettingsAction::ApplicationCounter = 0;

// 游戏特性激活时调用（计数 +1，首次时应用前端设置）。
void UApplyFrontendPerfSettingsAction::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	ApplicationCounter++;
	if (ApplicationCounter == 1)
	{
		//UTenkaichiSettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(true);   // ⚠️ 注释挂起：依赖 P18 Settings
	}
}

// 游戏特性停用时调用（计数 -1，归零时恢复）。
void UApplyFrontendPerfSettingsAction::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	ApplicationCounter--;
	check(ApplicationCounter >= 0);

	if (ApplicationCounter == 0)
	{
		//UTenkaichiSettingsLocal::Get()->SetShouldUseFrontendPerformanceSettings(false);   // ⚠️ 注释挂起：依赖 P18 Settings
	}
}