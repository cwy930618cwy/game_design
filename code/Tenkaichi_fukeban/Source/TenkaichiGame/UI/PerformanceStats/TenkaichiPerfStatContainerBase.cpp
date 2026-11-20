// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPerfStatContainerBase.h"

#include "Blueprint/WidgetTree.h"
#include "TenkaichiPerfStatWidgetBase.h"
//#include "Settings/TenkaichiSettingsLocal.h"   // ⚠️ 注释挂起：依赖 P18 Settings

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPerfStatContainerBase)

//////////////////////////////////////////////////////////////////////
// UTenkaichiPerfStatsContainerBase

// 构造函数：空实现。
UTenkaichiPerfStatContainerBase::UTenkaichiPerfStatContainerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 控件构造完成（更新可见性 + 监听设置变化）。
void UTenkaichiPerfStatContainerBase::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisibilityOfChildren();

	//UTenkaichiSettingsLocal::Get()->OnPerfStatDisplayStateChanged().AddUObject(this, &ThisClass::UpdateVisibilityOfChildren);   // ⚠️ 注释挂起：依赖 P18 Settings
}

// 控件销毁（解绑设置变化）。
void UTenkaichiPerfStatContainerBase::NativeDestruct()
{
	//UTenkaichiSettingsLocal::Get()->OnPerfStatDisplayStateChanged().RemoveAll(this);   // ⚠️ 注释挂起：依赖 P18 Settings

	Super::NativeDestruct();
}

// 更新所有子控件的可见性。
void UTenkaichiPerfStatContainerBase::UpdateVisibilityOfChildren()
{
#if 0
	// ⚠️ 注释挂起：依赖 P18 UTenkaichiSettingsLocal + ELyraStatDisplayMode
	UTenkaichiSettingsLocal* UserSettings = UTenkaichiSettingsLocal::Get();

	const bool bShowTextWidgets = (StatDisplayModeFilter == ELyraStatDisplayMode::TextOnly) || (StatDisplayModeFilter == ELyraStatDisplayMode::TextAndGraph);
	const bool bShowGraphWidgets = (StatDisplayModeFilter == ELyraStatDisplayMode::GraphOnly) || (StatDisplayModeFilter == ELyraStatDisplayMode::TextAndGraph);
	
	check(WidgetTree);
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (UTenkaichiPerfStatWidgetBase* TypedWidget = Cast<UTenkaichiPerfStatWidgetBase>(Widget))
		{
			const ELyraStatDisplayMode SettingMode = UserSettings->GetPerfStatDisplayState(TypedWidget->GetStatToDisplay());

			bool bShowWidget = false;
			switch (SettingMode)
			{
			case ELyraStatDisplayMode::Hidden:
				bShowWidget = false;
				break;
			case ELyraStatDisplayMode::TextOnly:
				bShowWidget = bShowTextWidgets;
				break;
			case ELyraStatDisplayMode::GraphOnly:
				bShowWidget = bShowGraphWidgets;
				break;
			case ELyraStatDisplayMode::TextAndGraph:
				bShowWidget = bShowTextWidgets || bShowGraphWidgets;
				break;
			}

			TypedWidget->SetVisibility(bShowWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	});
#endif
}