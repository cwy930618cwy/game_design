// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiTaggedWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiTaggedWidget)

//@TODO: 这个文件里其他的 TODO 都和"按 tag 显示/隐藏控件"有关，见 UE-142237

// 构造函数：空实现。
UTenkaichiTaggedWidget::UTenkaichiTaggedWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 控件构造完成（初始化可见性）。
void UTenkaichiTaggedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsDesignTime())
	{
		// 监听我们隐藏 tag 的变化
		//@TODO: 就是我说的那个东西

		// 设置初始可见性值（检查 tag 等等……）
		SetVisibility(GetVisibility());
	}
}

// 控件销毁。
void UTenkaichiTaggedWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		//@TODO: 停止监听 tag 变化
	}

	Super::NativeDestruct();
}

// 设置可见性（会记住调用者的意图，再按 tag 决定实际可见性）。
void UTenkaichiTaggedWidget::SetVisibility(ESlateVisibility InVisibility)
{
#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		Super::SetVisibility(InVisibility);
		return;
	}
#endif

	// 记住调用者请求的可见性；即使当前被 tag 压制了，
	// 等 tag 解除后我们也应该尊重这次调用
	bWantsToBeVisible = ConvertSerializedVisibilityToRuntime(InVisibility).IsVisible();
	if (bWantsToBeVisible)
	{
		ShownVisibility = InVisibility;
	}
	else
	{
		HiddenVisibility = InVisibility;
	}

	const bool bHasHiddenTags = false;//@TODO: Foo->HasAnyTags(HiddenByTags);

	// 真正应用可见性
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}

// 监听的 tag 变化时的回调。
void UTenkaichiTaggedWidget::OnWatchedTagsChanged()
{
	const bool bHasHiddenTags = false;//@TODO: Foo->HasAnyTags(HiddenByTags);

	// 真正应用可见性
	const ESlateVisibility DesiredVisibility = (bWantsToBeVisible && !bHasHiddenTags) ? ShownVisibility : HiddenVisibility;
	if (GetVisibility() != DesiredVisibility)
	{
		Super::SetVisibility(DesiredVisibility);
	}
}