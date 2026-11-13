// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonListView.h"

#include "TenkaichiListView.generated.h"

#define UE_API TENKAICHIGAME_API

class UUserWidget;
class ULocalPlayer;
class UTenkaichiWidgetFactory;

// 列表视图：按"工厂规则"为每条数据选控件类。
UCLASS(MinimalAPI, meta = (DisableNativeTick))
class UTenkaichiListView : public UCommonListView
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UE_API UTenkaichiListView(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if WITH_EDITOR
	// 编辑器校验：必须至少配一条工厂规则。
	UE_API virtual void ValidateCompiledDefaults(IWidgetCompilerLog& InCompileLog) const override;
#endif

protected:
	// 生成单条条目控件（先问工厂规则选类）。
	UE_API virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	//virtual bool OnIsSelectableOrNavigableInternal(UObject* SelectedItem) override;

protected:
	// 工厂规则列表（内联实例化）。
	UPROPERTY(EditAnywhere, Instanced, Category="Entry Creation")
	TArray<TObjectPtr<UTenkaichiWidgetFactory>> FactoryRules;
};

#undef UE_API