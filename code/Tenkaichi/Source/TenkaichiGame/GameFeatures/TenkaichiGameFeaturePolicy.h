// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureStateChangeObserver.h"
#include "GameFeaturesProjectPolicies.h"

#include "TenkaichiGameFeaturePolicy.generated.h"

class FName;
class UGameFeatureData;
struct FPrimaryAssetId;

/**
 * 管理"把游戏特性插件载入内存并激活"的状态机的管理器。
 * 这个类发现插件：要么是随游戏内置分发的，要么是外部报告的（比如 web 服务或其他端点）。
 */
UCLASS(MinimalAPI, Config = Game)
class UTenkaichiGameFeaturePolicy : public UDefaultGameFeaturesProjectPolicies
{
	GENERATED_BODY()

public:
	// 返回策略单例。
	TENKAICHIGAME_API static UTenkaichiGameFeaturePolicy& Get();

	// 构造函数。
	UTenkaichiGameFeaturePolicy(const FObjectInitializer& ObjectInitializer);

	//~UGameFeaturesProjectPolicies 接口开始
	// 初始化 GameFeature 管理器（注册观察者）。
	virtual void InitGameFeatureManager() override;
	// 关闭 GameFeature 管理器（移除观察者）。
	virtual void ShutdownGameFeatureManager() override;
	// 返回某个 GameFeature 的预加载资产列表。
	virtual TArray<FPrimaryAssetId> GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets = false) const override;
	// 某个插件是否允许加载。
	virtual bool IsPluginAllowed(const FString& PluginURL, FString* OutReason) const override;
	// 返回 GameFeature 的预加载 Bundle 状态。
	virtual const TArray<FName> GetPreloadBundleStateForGameFeature() const override;
	// 返回 GameFeature 的加载模式（客户端/服务器）。
	virtual void GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const override;
	//~UGameFeaturesProjectPolicies 接口结束

private:
	// 注册的观察者列表。
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Observers;
};



// 已检查
// 观察者：插件加载时触发 Hotfix。
UCLASS()
class UTenkaichiGameFeature_HotfixManager : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	// 游戏特性加载时调用（触发从 ini 拉补丁资产）。
	virtual void OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;
};

// 已检查
// 观察者：插件注册/注销时增删 GameplayCue 路径。
UCLASS()
class UTenkaichiGameFeature_AddGameplayCuePaths : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	// 游戏特性注册时调用（注册 Cue 路径）。
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
	// 游戏特性注销时调用（移除 Cue 路径）。
	virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
};