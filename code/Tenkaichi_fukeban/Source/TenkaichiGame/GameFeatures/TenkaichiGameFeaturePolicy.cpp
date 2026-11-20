// 版权声明（可保留或省略，不影响编译）
#include "GameFeatures/TenkaichiGameFeaturePolicy.h"

#include "AbilitySystem/TenkaichiGameplayCueManager.h"
#include "GameFeatureData.h"
#include "GameplayCueSet.h"

// 构造函数：空实现（父类处理初始化）。
UTenkaichiGameFeaturePolicy::UTenkaichiGameFeaturePolicy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 返回策略单例。
UTenkaichiGameFeaturePolicy& UTenkaichiGameFeaturePolicy::Get()
{
	return UGameFeaturesSubsystem::Get().GetPolicy<UTenkaichiGameFeaturePolicy>();
}

// 初始化 GameFeature 管理器（注册 2 个观察者）。
void UTenkaichiGameFeaturePolicy::InitGameFeatureManager()
{
	// 创建两个观察者
	Observers.Add(NewObject<UTenkaichiGameFeature_HotfixManager>());
	Observers.Add(NewObject<UTenkaichiGameFeature_AddGameplayCuePaths>());

	// 把它们注册进 GameFeatures 子系统
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.AddObserver(Observer);
	}

	Super::InitGameFeatureManager();
}

// 关闭 GameFeature 管理器（移除观察者）。
void UTenkaichiGameFeaturePolicy::ShutdownGameFeatureManager()
{
	Super::ShutdownGameFeatureManager();

	// 从 GameFeatures 子系统移除观察者
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.RemoveObserver(Observer);
	}
	Observers.Empty();
}

// 返回某个 GameFeature 的预加载资产列表（回退父类）。
TArray<FPrimaryAssetId> UTenkaichiGameFeaturePolicy::GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets) const
{
	return Super::GetPreloadAssetListForGameFeature(GameFeatureToLoad, bIncludeLoadedAssets);
}

// 返回 GameFeature 的预加载 Bundle 状态（回退父类）。
const TArray<FName> UTenkaichiGameFeaturePolicy::GetPreloadBundleStateForGameFeature() const
{
	return Super::GetPreloadBundleStateForGameFeature();
}

// 返回 GameFeature 的加载模式（按 NetMode 决定客户端/服务器）。
void UTenkaichiGameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
	// 编辑器会加载两者，这可能导致卡顿，因为 Bundle 在编辑器里被设置为不预加载
	bLoadClientData = !IsRunningDedicatedServer();
	bLoadServerData = !IsRunningClientOnly();
}

// 某个插件是否允许加载（回退父类）。
bool UTenkaichiGameFeaturePolicy::IsPluginAllowed(const FString& PluginURL, FString* OutReason) const
{
	return Super::IsPluginAllowed(PluginURL, OutReason);
}

//////////////////////////////////////////////////////////////////////
//

//#include "Hotfix/TenkaichiHotfixManager.h"   // ⚠️ 注释挂起：依赖 P18 Hotfix

#if 0
// ⚠️ 注释挂起：依赖 P18 UTenkaichiHotfixManager
// 游戏特性加载时调用：触发 hotfix 从 ini 拉补丁资产。
void UTenkaichiGameFeature_HotfixManager::OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL)
{
	if (UTenkaichiHotfixManager* HotfixManager = Cast<UTenkaichiHotfixManager>(UOnlineHotfixManager::Get(nullptr)))
	{
		HotfixManager->RequestPatchAssetsFromIniFiles();
	}
}
#endif

//////////////////////////////////////////////////////////////////////
//

#include "AbilitySystemGlobals.h"
#include "GameFeatureAction_AddGameplayCuePath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameFeaturePolicy)

class FName;
struct FPrimaryAssetId;

// 游戏特性注册时调用：把 Cue 路径真正注册进 GameplayCueManager。
void UTenkaichiGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UTenkaichiGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering);
	
	const FString PluginRootPath = TEXT("/") + PluginName;
	// 遍历这个 GameFeature 的所有 Action，找出"加 Cue 路径"的 Action
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(Action))
		{
			const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();
			
			if (UTenkaichiGameplayCueManager* GCM = UTenkaichiGameplayCueManager::Get())
			{
				UGameplayCueSet* RuntimeGameplayCueSet = GCM->GetRuntimeCueSet();
				// 记下注册前有多少个 Cue
				const int32 PreInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;

				for (const FDirectoryPath& Directory : DirsToAdd)
				{
					FString MutablePath = Directory.Path;
					// 把插件相对路径修正成完整路径
					UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
					GCM->AddGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);	
				}
				
				// 有新路径就重建运行时库
				if (!DirsToAdd.IsEmpty())
				{
					GCM->InitializeRuntimeObjectLibrary();	
				}

				// 对比注册前后的 Cue 数量，变了就刷新主资产
				const int32 PostInitializeNumCues = RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0;
				if (PreInitializeNumCues != PostInitializeNumCues)
				{
					GCM->RefreshGameplayCuePrimaryAsset();
				}
			}
		}
	}
}

// 游戏特性注销时调用：把 Cue 路径从 GameplayCueManager 移除。
void UTenkaichiGameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	const FString PluginRootPath = TEXT("/") + PluginName;
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		if (const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<UGameFeatureAction_AddGameplayCuePath>(Action))
		{
			const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();
			
			if (UGameplayCueManager* GCM = UAbilitySystemGlobals::Get().GetGameplayCueManager())
			{
				int32 NumRemoved = 0;
				for (const FDirectoryPath& Directory : DirsToAdd)
				{
					FString MutablePath = Directory.Path;
					UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
					NumRemoved += GCM->RemoveGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
				}

				ensure(NumRemoved == DirsToAdd.Num());
				
				// 只有确实移除了才重建运行时库
				if (NumRemoved > 0)
				{
					GCM->InitializeRuntimeObjectLibrary();	
				}			
			}
	}
	}
}