// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction.h"
#include "UObject/SoftObjectPath.h"

#include "GameFeatureAction_AddGameplayCuePath.generated.h"

/**
 * 负责把 GameplayCue 路径添加到 GameplayCue 管理器的 GameFeatureAction。
 *
 * @see UAbilitySystemGlobals::GameplayCueNotifyPaths
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Gameplay Cue Path"))
class UGameFeatureAction_AddGameplayCuePath final : public UGameFeatureAction
{
	GENERATED_BODY()

public:

	// 构造函数：加一个常用的默认路径。
	UGameFeatureAction_AddGameplayCuePath();

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器数据校验：检查路径是否有效。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	// 返回要添加的目录路径列表（供观察者读取）。
	const TArray<FDirectoryPath>& GetDirectoryPathsToAdd() const { return DirectoryPathsToAdd; }

private:
	// 要注册到 GameplayCue 管理器的路径列表。相对游戏内容目录。
	UPROPERTY(EditAnywhere, Category = "Game Feature | Gameplay Cues", meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> DirectoryPathsToAdd;
};