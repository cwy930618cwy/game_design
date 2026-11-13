// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataAsset.h"
#include "TenkaichiExperienceActionSet.generated.h"

class UGameFeatureAction;

/**
 * 经验动作集（ExperienceActionSet）
 *
 * 把一组动作和一组 GameFeature 插件打包成可复用片段，
 * 供 ExperienceDefinition 通过 ActionSets 字段引用（用组合代替蓝图继承）。
 */
UCLASS(BlueprintType, NotBlueprintable)
class UTenkaichiExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UTenkaichiExperienceActionSet();

	//~UObject 接口
#if WITH_EDITOR
	// 数据资产校验（仅编辑器环境）：检查 Actions 数组没有空项。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	//~UPrimaryDataAsset 接口
#if WITH_EDITORONLY_DATA
	// 更新资源包数据（仅编辑器数据）：让动作的依赖资产能被打进 Bundle。
	virtual void UpdateAssetBundleData() override;
#endif
	//~UPrimaryDataAsset 接口结束

public:
	// 这一组要执行的动作列表（引擎 GameFeatureAction，每个可独立配置）。
	UPROPERTY(EditAnywhere, Instanced, Category="Actions to Perform")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// 这一组要激活的 Game Feature 插件名列表。
	UPROPERTY(EditAnywhere, Category="Feature Dependencies")
	TArray<FString> GameFeaturesToEnable;
};