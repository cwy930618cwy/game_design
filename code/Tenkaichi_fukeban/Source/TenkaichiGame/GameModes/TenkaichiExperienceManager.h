// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "TenkaichiExperienceManager.generated.h"

/**
 * 经验管理器（ExperienceManager）
 *
 * 主要用于多个 PIE 会话之间仲裁：同一个 GameFeature 插件被多次请求激活/停用时，
 * 用引用计数保证"最后一个请求者才能停用"。
 */
UCLASS(MinimalAPI)
class UTenkaichiExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	// PIE 开始前调用：清空插件请求计数表。
	TENKAICHIGAME_API void OnPlayInEditorBegun();

	// 通知：某个插件被激活了（静态方法，计数 +1）。
	static void NotifyOfPluginActivation(const FString PluginURL);
	// 请求停用某个插件（静态方法，计数 -1，归零才真停）。
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	// 非编辑器（发布包）：插件激活什么都不做（空实现）。
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	// 非编辑器（发布包）：插件停用永远允许（返回 true）。
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif

private:
	// 插件名 → 请求激活次数 的计数表
	// （用于 PIE 期间"先入后出"的激活管理：最后申请的人才允许停用）。
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};