// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiExperienceActionSet.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiExperienceActionSet)

#define LOCTEXT_NAMESPACE "TenkaichiSystem"

UTenkaichiExperienceActionSet::UTenkaichiExperienceActionSet()
{
}

#if WITH_EDITOR
EDataValidationResult UTenkaichiExperienceActionSet::IsDataValid(FDataValidationContext& Context) const
{
	// 先算父类结果，默认"有效"。
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			// 逐个让动作自校验，合并结果。
			EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			// Actions 数组里有空项 → 整体无效，并报错指出下标。
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Actions 里下标 {0} 处是空项"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UTenkaichiExperienceActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	// 把每个动作的附加资源打进资源包，保证依赖能被正确加载。
	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE