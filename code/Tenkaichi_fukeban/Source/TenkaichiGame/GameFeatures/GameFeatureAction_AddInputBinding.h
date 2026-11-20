// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "GameFeatureAction_AddInputBinding.generated.h"

class AActor;
class UInputMappingContext;
class UPlayer;
class APlayerController;
struct FComponentRequestHandle;
class UTenkaichiInputConfig;

/**
 * 把 InputMappingContext 加到本地玩家的 EnhancedInput 系统。
 * 前提是本地玩家已经设置为使用 EnhancedInput 系统。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Binds"))
class UGameFeatureAction_AddInputBinding final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性激活时调用。
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	// 游戏特性停用时调用。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~UGameFeatureAction 接口结束

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器数据校验。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	// 要添加的输入配置列表。
	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UTenkaichiInputConfig>> InputConfigs;

private:
	// 每个上下文（一次激活）的数据。
	struct FPerContextData
	{
		// 扩展请求句柄（解绑用）。
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// 已经添加过输入映射的 Pawn（弱引用，防悬空）。
		TArray<TWeakObjectPtr<APawn>> PawnsAddedTo;
	};

	// 上下文 → 该上下文的运行时数据。
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase 接口开始
	// 在某个 world 里执行：注册 Pawn 扩展处理器。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~UGameFeatureAction_WorldActionBase 接口结束

	// 重置某个上下文的数据。
	void Reset(FPerContextData& ActiveData);
	// Pawn 扩展事件回调（出现/消失时加/移除输入配置）。
	void HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	// 给玩家添加输入映射。
	void AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData);
	// 移除输入映射。
	void RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData);
};