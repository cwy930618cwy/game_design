// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiExperienceDefinition.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiExperienceDefinition)

#define LOCTEXT_NAMESPACE "TenkaichiSystem"

UTenkaichiExperienceDefinition::UTenkaichiExperienceDefinition()
{
}

#if WITH_EDITOR
EDataValidationResult UTenkaichiExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	// 先算父类校验结果，默认视为"有效"。
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

	// 禁止"蓝图经验"再被蓝图子类化（在 BP 里子类化一次可以，但不能套娃）。
	if (!GetClass()->IsNative())
	{
		const UClass* ParentClass = GetClass()->GetSuperClass();

		// 往上找最近的"原生 C++ 父类"。
		const UClass* FirstNativeParent = ParentClass;
		while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
		{
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}

		if (FirstNativeParent != ParentClass)
		{
			// 说明这个蓝图经验是从另一个蓝图经验继承的 → 报错，建议用 ActionSets 组合代替继承。
			Context.AddError(FText::Format(LOCTEXT("ExperienceInheritenceIsUnsupported", "蓝图经验不支持被蓝图子类化（请改用 ActionSets 组合）。父类是 {0}，但应是 {1}。"),
				FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
				FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
			));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UTenkaichiExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	// 把每个动作的附加资产打进资源包数据，保证依赖能被正确加载。
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