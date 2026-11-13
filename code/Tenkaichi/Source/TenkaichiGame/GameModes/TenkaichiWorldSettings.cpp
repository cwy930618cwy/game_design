// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "TenkaichiLogChannels.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiWorldSettings)

// 构造函数：只调用父类构造，本类没有需要额外初始化的成员。
ATenkaichiWorldSettings::ATenkaichiWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 返回这张地图默认要用的经验（解析成主资产 ID，供 AssetManager 加载）。
FPrimaryAssetId ATenkaichiWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;
	if (!DefaultGameplayExperience.IsNull())
	{
		// 把软引用路径交给引擎资产管理器，解析成主资产 ID。
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			// 解析失败：说明这个软引用路径指不到一个合法的主资产（可能要在 GameFeature 插件或项目设置里加资产规则）。
			UE_LOG(LogTenkaichiExperience, Error, TEXT("%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"),
				*GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
		}
	}
	return Result;
}

#if WITH_EDITOR
// 地图检查时的校验：警告地图里放了普通的 APlayerStart，应换成 ATenkaichiPlayerStart。
void ATenkaichiWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	// 消息日志分类 "MapCheck"，用于编辑器地图检查面板。
	FMessageLog MapCheck("MapCheck");

	// 遍历当前世界里的所有 APlayerStart 出生点。
	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			// 普通 APlayerStart（不是 Lyra 自定义的子类）→ 警告用户换成 ATenkaichiPlayerStart。
			MapCheck.Warning()
				->AddToken(FUObjectToken::Create(PlayerStart))
				->AddToken(FTextToken::Create(FText::FromString("is a normal APlayerStart, replace with ATenkaichiPlayerStart.")));
		}
	}

	//@TODO: 确认这个软引用路径真的能解析成主资产 ID（例如不要指向一个没被扫描的目录里的经验）。
}
#endif