// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFramework/WorldSettings.h"
#include "TenkaichiWorldSettings.generated.h"

#define UE_API TENKAICHIGAME_API

class UTenkaichiExperienceDefinition;

/**
 * 默认的世界设置对象，主要用来设置"在这张地图上玩时默认用哪个经验"。
 */
UCLASS(MinimalAPI)
class ATenkaichiWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	UE_API ATenkaichiWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	UE_API virtual void CheckForErrors() override;
#endif

public:
	// 当服务器打开这张地图时，返回要用的默认经验（如果没被"面向用户的经验"覆盖的话）。
	UE_API FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	// 当服务器打开这张地图时，默认要用的经验（如果没被"面向用户的经验"覆盖的话）。
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<UTenkaichiExperienceDefinition> DefaultGameplayExperience;

public:

#if WITH_EDITORONLY_DATA
	// 这个关卡是不是"前端/独立经验"的一部分？
	// 设为 true 时，在编辑器里点 Play 会强制 NetMode 为 Standalone（单机）。
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif
};

#undef UE_API