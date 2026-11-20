// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiExperienceManager.h"
#include "GameModes/TenkaichiExperienceManager.h"
#include "Engine/Engine.h"
#include "Subsystems/SubsystemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiExperienceManager)

#if WITH_EDITOR

void UTenkaichiExperienceManager::OnPlayInEditorBegun()
{
	// 正常情况计数表应该本来就是空的（上个 PIE 结束已清空），清一遍保险。
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UTenkaichiExperienceManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		UTenkaichiExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UTenkaichiExperienceManager>();
		check(ExperienceManagerSubsystem);

		// 记录申请激活该插件的请求方数量。多次加载/激活总是允许的（并发请求会被合并处理）。
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UTenkaichiExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		UTenkaichiExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UTenkaichiExperienceManager>();
		check(ExperienceManagerSubsystem);

		// 只有最后一个请求方允许真正停用该插件。
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif