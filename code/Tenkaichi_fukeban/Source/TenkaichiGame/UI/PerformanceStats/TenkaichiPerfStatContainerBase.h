// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonUserWidget.h"
//#include "Performance/TenkaichiPerformanceStatTypes.h"   // ⚠️ 注释挂起：依赖 P18 Performance

#include "TenkaichiPerfStatContainerBase.generated.h"

class UObject;
struct FFrame;

/**
 * UTenkaichiPerfStatsContainerBase
 *
 * 包含一组 UTenkaichiPerfStatWidgetBase 控件的面板，
 * 并根据用户设置管理它们的可见性。
 */
UCLASS(Abstract)
class UTenkaichiPerfStatContainerBase : public UCommonUserWidget
{
public:
	// 构造函数：空实现。
	UTenkaichiPerfStatContainerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	GENERATED_BODY()

	//~UUserWidget 接口开始
	// 控件构造完成（更新可见性 + 监听设置变化）。
	virtual void NativeConstruct() override;
	// 控件销毁（解绑设置变化）。
	virtual void NativeDestruct() override;
	//~UUserWidget 接口结束

	// 更新所有子控件的可见性。
	UFUNCTION(BlueprintCallable)
	void UpdateVisibilityOfChildren();

protected:
	// 我们显示文字统计还是图表统计？
	//UPROPERTY(EditAnywhere, Category=Display)
	//ELyraStatDisplayMode StatDisplayModeFilter = ELyraStatDisplayMode::TextAndGraph;   // ⚠️ 注释挂起：依赖 P18 Performance
};