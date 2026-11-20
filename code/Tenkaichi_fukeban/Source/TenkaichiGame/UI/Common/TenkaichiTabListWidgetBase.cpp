// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiTabListWidgetBase.h"

#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTabListWidgetBase)

// 控件初始化。
void UTenkaichiTabListWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

// 控件构造完成（建立标签）。
void UTenkaichiTabListWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetupTabs();
}

// 控件销毁（移除标签内容）。
void UTenkaichiTabListWidgetBase::NativeDestruct()
{
	for (FTenkaichiTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
			TabInfo.CreatedTabContentWidget = nullptr;
		}
	}

	Super::NativeDestruct();
}

// 获取某个预注册标签的信息。
bool UTenkaichiTabListWidgetBase::GetPreregisteredTabInfo(const FName TabNameId, FTenkaichiTabDescriptor& OutTabInfo)
{
	const FTenkaichiTabDescriptor* const FoundTabInfo = PreregisteredTabInfoArray.FindByPredicate([&](FTenkaichiTabDescriptor& TabInfo) -> bool
	{
		return TabInfo.TabId == TabNameId;
	});

	if (!FoundTabInfo)
	{
		return false;
	}

	OutTabInfo = *FoundTabInfo;
	return true;
}

// 切换某个标签的隐藏状态。
void UTenkaichiTabListWidgetBase::SetTabHiddenState(FName TabNameId, bool bHidden)
{
	for (FTenkaichiTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.TabId == TabNameId)
		{
			TabInfo.bHidden = bHidden;
			break;
		}
	}
}

// 运行时注册一个新标签。
bool UTenkaichiTabListWidgetBase::RegisterDynamicTab(const FTenkaichiTabDescriptor& TabDescriptor)
{
	// 如果它是隐藏的就忽略。
	if (TabDescriptor.bHidden)
	{
		return true;
	}
	
	PendingTabLabelInfoMap.Add(TabDescriptor.TabId, TabDescriptor);

	return RegisterTab(TabDescriptor.TabId, TabDescriptor.TabButtonType, TabDescriptor.CreatedTabContentWidget);
}

// switcher 变化前的回调（移除旧内容）。
void UTenkaichiTabListWidgetBase::HandlePreLinkedSwitcherChanged()
{
	for (const FTenkaichiTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		// 从绑定的 switcher 移除标签内容控件，因为正在解除关联。
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
		}
	}

	Super::HandlePreLinkedSwitcherChanged();
}

// switcher 变化后的回调（重建标签）。
void UTenkaichiTabListWidgetBase::HandlePostLinkedSwitcherChanged()
{
	if (!IsDesignTime() && GetCachedWidget().IsValid())
	{
		// 在编辑器设计器里或还没构造完，就别建标签了
		SetupTabs();
	}

	Super::HandlePostLinkedSwitcherChanged();
}

// 标签创建完成的回调（填标签信息）。
void UTenkaichiTabListWidgetBase::HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton)
{
	FTenkaichiTabDescriptor* TabInfoPtr = nullptr;
	
	FTenkaichiTabDescriptor TabInfo;
	if (GetPreregisteredTabInfo(TabId, TabInfo))
	{
		TabInfoPtr = &TabInfo;
	}
	else
	{
		TabInfoPtr = PendingTabLabelInfoMap.Find(TabId);
	}
	
	if (TabButton->GetClass()->ImplementsInterface(UTenkaichiTabButtonInterface::StaticClass()))
	{
		if (ensureMsgf(TabInfoPtr, TEXT("A tab button was created with id %s but no label info was specified. RegisterDynamicTab should be used over RegisterTab to provide label info."), *TabId.ToString()))
		{
			ITenkaichiTabButtonInterface::Execute_SetTabLabelInfo(TabButton, *TabInfoPtr);
		}
	}

	PendingTabLabelInfoMap.Remove(TabId);
}

// 第一个标签是否是激活状态。
bool UTenkaichiTabListWidgetBase::IsFirstTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray[0].TabId;
	}

	return false;
}

// 最后一个标签是否是激活状态。
bool UTenkaichiTabListWidgetBase::IsLastTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray.Last().TabId;
	}

	return false;
}

// 某个标签是否可见。
bool UTenkaichiTabListWidgetBase::IsTabVisible(FName TabId)
{
	if (const UCommonButtonBase* Button = GetTabButtonBaseByID(TabId))
	{
		const ESlateVisibility TabVisibility = Button->GetVisibility();
		return (TabVisibility == ESlateVisibility::Visible
			|| TabVisibility == ESlateVisibility::HitTestInvisible
			|| TabVisibility == ESlateVisibility::SelfHitTestInvisible);
	}

	return false;
}

// 可见标签数量。
int32 UTenkaichiTabListWidgetBase::GetVisibleTabCount()
{
	int32 Result = 0;
	const int32 TabCount = GetTabCount();
	for ( int32 Index = 0; Index < TabCount; Index++ )
	{
		if (IsTabVisible(GetTabIdAtIndex( Index )))
		{
			Result++;
		}
	}

	return Result;
}

// 建立所有标签（创建内容 + 注册）。
void UTenkaichiTabListWidgetBase::SetupTabs()
{
	for (FTenkaichiTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.bHidden)
		{
			continue;
		}

		// 如果标签内容还没创建过，就创建它。
		if (!TabInfo.CreatedTabContentWidget && TabInfo.TabContentType)
		{
			TabInfo.CreatedTabContentWidget = CreateWidget<UCommonUserWidget>(GetOwningPlayer(), TabInfo.TabContentType);
			OnTabContentCreatedNative.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
			OnTabContentCreated.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
		}

		if (UCommonAnimatedSwitcher* CurrentLinkedSwitcher = GetLinkedSwitcher())
		{
			// 把标签内容加到新绑定的 switcher 里。
			if (!CurrentLinkedSwitcher->HasChild(TabInfo.CreatedTabContentWidget))
			{
				CurrentLinkedSwitcher->AddChild(TabInfo.CreatedTabContentWidget);
			}
		}

		// 如果标签还没注册过，就注册它。
		if (GetTabButtonBaseByID(TabInfo.TabId) == nullptr)
		{
			RegisterTab(TabInfo.TabId, TabInfo.TabButtonType, TabInfo.CreatedTabContentWidget);
		}
	}
}