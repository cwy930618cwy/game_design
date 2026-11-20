// 版权声明（可保留或省略，不影响编译）
#include "IndicatorLayer.h"

#include "SActorCanvas.h"
#include "Widgets/Layout/SBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorLayer)

class SWidget;

/////////////////////////////////////////////////////
// UIndicatorLayer

// 构造函数：标记为变量 + 设默认可见性。
UIndicatorLayer::UIndicatorLayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

// 释放 Slate 资源。
void UIndicatorLayer::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyActorCanvas.Reset();
}

// 重建 Slate 控件（创建 SActorCanvas）。
TSharedRef<SWidget> UIndicatorLayer::RebuildWidget()
{
	if (!IsDesignTime())
	{
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (ensureMsgf(LocalPlayer, TEXT("Attempting to rebuild a UActorCanvas without a valid LocalPlayer!")))
		{
			MyActorCanvas = SNew(SActorCanvas, FLocalPlayerContext(LocalPlayer), &ArrowBrush);
			return MyActorCanvas.ToSharedRef();
		}
	}

	// 给它一个简单的盒子，NullWidget 从 UWidget 里用不安全
	return SNew(SBox);
}