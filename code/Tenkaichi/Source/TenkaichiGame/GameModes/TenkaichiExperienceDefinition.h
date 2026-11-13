// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DataAsset.h"
#include "TenkaichiExperienceDefinition.generated.h"

class UGameFeatureAction;
class UTenkaichiPawnData;
class UTenkaichiExperienceActionSet;

/**
 * 经验定义（ExperienceDefinition）
 *
 * 一局游戏要用哪套规则：加载哪些 GameFeature、玩家默认用什么 Pawn、
 * 要执行哪些动作、复用哪些动作集。只存数据，执行逻辑在 ExperienceManagerComponent。
 */
UCLASS(BlueprintType, Const)
class UTenkaichiExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UTenkaichiExperienceDefinition();

	//~UObject 接口
#if WITH_EDITOR
	// 数据资产校验（仅编辑器环境）：检查依赖数据是否有效。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	//~UPrimaryDataAsset 接口
#if WITH_EDITORONLY_DATA
	// 更新资源包数据（仅编辑器数据）：让资源的依赖能被打进 Bundle。
	virtual void UpdateAssetBundleData() override;
#endif
	//~UPrimaryDataAsset 接口结束

public:
	// 这一局要激活的 Game Feature 插件列表（写插件名，如 "TenkaichiGameplayAbilities"）。
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** 玩家默认生成的 Pawn 数据（✅4.3 已教的 PawnData）。 */
	//@TODO: 考虑改成软引用
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UTenkaichiPawnData> DefaultPawnData;

	// 这一局加载/激活/停用时执行的动作列表（引擎 GameFeatureAction）。
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// 追加进这一局的额外动作集列表（P13.2 待教，先前置声明软引用）。
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UTenkaichiExperienceActionSet>> ActionSets;
};