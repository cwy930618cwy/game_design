// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "CommonActivatableWidget.h"
#include "GameFeatureAction_WorldActionBase.h"
#include "UIExtensionSystem.h"

#include "GameFeatureAction_AddWidget.generated.h"

struct FWorldContext;
struct FComponentRequestHandle;

// 一个 HUD 布局请求：要生成的布局控件 + 插到哪个层。
USTRUCT()
struct FTenkaichiHUDLayoutRequest
{
	GENERATED_BODY()

	// 要生成的布局控件
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	// 要把控件插入的层
	UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="UI.Layer"))
	FGameplayTag LayerID;
};


// 一个 HUD 元素请求：要生成的控件 + 放到哪个插槽。
USTRUCT()
struct FTenkaichiHUDElementEntry
{
	GENERATED_BODY()

	// 要生成的控件
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	// 应该把这个控件放到哪个插槽 ID
	UPROPERTY(EditAnywhere, Category = UI)
	FGameplayTag SlotID;
};

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddWidgets

/**
 * 负责添加控件的 GameFeatureAction。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Widgets"))
class UGameFeatureAction_AddWidgets final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性停用时调用。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	// 编辑器专用：把控件资产加入 AssetBundle 数据。
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~UGameFeatureAction 接口结束

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器数据校验。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

private:
	// 要加到 HUD 的布局
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{LayerID} -> {LayoutClass}"))
	TArray<FTenkaichiHUDLayoutRequest> Layout;

	// 要加到 HUD 的控件
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{SlotID} -> {WidgetClass}"))
	TArray<FTenkaichiHUDElementEntry> Widgets;

private:

	// 每个 HUD Actor 的数据。
	struct FPerActorData
	{
		// 已添加的布局（弱引用，防悬空）。
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		// UI 扩展句柄（注销用）。
		TArray<FUIExtensionHandle> ExtensionHandles;
	};

	// 每个上下文（一次激活）的数据。
	struct FPerContextData
	{
		// 组件请求句柄。
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		// HUD Actor → 该 Actor 的数据。
		TMap<FObjectKey, FPerActorData> ActorData; 
	};

	// 上下文 → 该上下文的运行时数据。
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase 接口开始
	// 在某个 world 里执行：注册 HUD Actor 扩展处理器。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~UGameFeatureAction_WorldActionBase 接口结束

	// 重置某个上下文的数据。
	void Reset(FPerContextData& ActiveData);

	// HUD Actor 扩展事件回调。
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	// 给 HUD Actor 添加控件。
	void AddWidgets(AActor* Actor, FPerContextData& ActiveData);
	// 从 HUD Actor 移除控件。
	void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);
};