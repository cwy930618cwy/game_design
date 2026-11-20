// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataAsset.h"

#include "TenkaichiLobbyBackground.generated.h"

class UObject;
class UWorld;

/**
 * 开发者设置 / 编辑器作弊
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class UTenkaichiLobbyBackground : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	// 用作大厅背景的关卡（软引用）。
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> BackgroundLevel;
};