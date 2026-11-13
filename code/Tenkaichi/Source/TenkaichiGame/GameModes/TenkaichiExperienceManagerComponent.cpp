// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiExperienceManagerComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TenkaichiExperienceDefinition.h"
#include "TenkaichiExperienceActionSet.h"
#include "TenkaichiExperienceManager.h"
#include "GameFeaturesSubsystem.h"
//#include "System/TenkaichiAssetManager.h"   // ⚠️ 注释挂起：依赖 P14 AssetManager
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystemSettings.h"
#include "TimerManager.h"
//#include "Settings/TenkaichiSettingsLocal.h"   // ⚠️ 注释挂起：依赖 P18 Settings
#include "TenkaichiLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiExperienceManagerComponent)

//@TODO: 异步加载经验定义本身
//@TODO: 显式处理失败（进入"已完成但失败"的状态，而不是 check 崩溃）
//@TODO: 在合适的时机分阶段执行 Action，而不是一次性全做
//@TODO: 支持停用经验并执行卸载动作
//@TODO: 思考停用/清理对已预加载资产意味着什么
//@TODO: 处理停用 GameFeature，目前我们"泄漏"了它们（保持启用）
// （对于从一个经验切到另一个经验的客户端，我们实际上想对比需求、只卸载部分，而不是全卸了再立刻重载）
//@TODO: 同时处理内置插件和 URL 插件（搜索冒号？）

namespace TenkaichiConsoleVariables
{
	// 经验加载完成时额外加的最小随机延迟（秒）。
	static float ExperienceLoadRandomDelayMin = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("tenkaichi.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT("这个值（秒）会被加进经验加载完成的延迟里（连同随机值 tenkaichi.chaos.ExperienceDelayLoad.RandomSecs）"),
		ECVF_Default);

	// 经验加载完成时额外加的随机延迟范围（秒）。
	static float ExperienceLoadRandomDelayRange = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("tenkaichi.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT("0 到这个值（秒）之间的随机时间，会被加进经验加载完成的延迟里（连同固定值 tenkaichi.chaos.ExperienceDelayLoad.MinSecs）"),
		ECVF_Default);

	// 返回"经验加载完成"要额外等多久（固定 + 随机）。
	float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
}

// 构造函数：默认打开网络复制。
UTenkaichiExperienceManagerComponent::UTenkaichiExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

#if 0
// ⚠️ 注释挂起：本函数硬依赖 P14 UTenkaichiAssetManager（SetCurrentExperience）
// 设置当前要用的经验：解析资产 → 取默认实例 → 启动加载。
void UTenkaichiExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	UTenkaichiAssetManager& AssetManager = UTenkaichiAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UTenkaichiExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);
	const UTenkaichiExperienceDefinition* Experience = GetDefault<UTenkaichiExperienceDefinition>(AssetClass);

	check(Experience != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = Experience;
	StartExperienceLoad();
}
#endif

// 注册"经验加载完成"的高优先级回调（已加载就立刻执行）。
void UTenkaichiExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

// 注册"经验加载完成"的普通优先级回调（已加载就立刻执行）。
void UTenkaichiExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

// 注册"经验加载完成"的低优先级回调（已加载就立刻执行）。
void UTenkaichiExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

// 返回当前经验（前提是已加载完，否则 assert）。
const UTenkaichiExperienceDefinition* UTenkaichiExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == ETenkaichiExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

// 返回"经验是否已加载完"。
bool UTenkaichiExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == ETenkaichiExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

// 网络复制回调：客户端收到"当前经验"后，也启动自己的加载。
void UTenkaichiExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

#if 0
// ⚠️ 注释挂起：本函数硬依赖 P14 UTenkaichiAssetManager + FLyraBundles（StartExperienceLoad）
// 开始加载经验资产：收集 Bundle 列表 → 异步加载 → 绑定完成回调。
void UTenkaichiExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == ETenkaichiExperienceLoadState::Unloaded);

	UE_LOG(LogTenkaichiExperience, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	LoadState = ETenkaichiExperienceLoadState::Loading;

	UTenkaichiAssetManager& AssetManager = UTenkaichiAssetManager::Get();

	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UTenkaichiExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// 加载和经验关联的资产

	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add(FTenkaichiBundles::Equipped);

	//@TODO: 把这段客户端/服务器逻辑集中到 TenkaichiAssetManager 里
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);
	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}

	// 如果两个异步加载都在跑，就把它们合并成一个句柄
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({ BundleLoadHandle, RawLoadHandle });
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// 资产已经加载好了，现在就调用回调
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}));
	}

	// 这批资产会被预加载，但我们不会因为它而阻塞经验的开局
	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: 决定要预加载（但不阻塞式）的资产
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}
#endif

// 资产加载完成：收集 GameFeature 插件 URL → 激活它们。
void UTenkaichiExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == ETenkaichiExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	UE_LOG(LogTenkaichiExperience, Log, TEXT("EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s, %s)"),
		*CurrentExperience->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	// 找出我们要激活的 GameFeature 插件的 URL——过滤掉重复项和没有有效映射的项
	GameFeaturePluginURLs.Reset();

	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnExperienceLoadComplete 找不到插件 %s 的 URL（经验 %s）——修复数据，本次运行忽略它"), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}

		// 		// 加入我们的额外插件
		// 		if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
		// 		{
		// 			FString PluginURL;
		// 			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
		// 			{
		// 				GameFeaturePluginURLs.AddUnique(PluginURL);
		// 			}
		// 		}
	};

	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);
	for (const TObjectPtr<UTenkaichiExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	// 加载并激活这些特性
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = ETenkaichiExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			UTenkaichiExperienceManager::NotifyOfPluginActivation(PluginURL);
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		OnExperienceFullLoadCompleted();
	}
}

// 单个 GameFeature 插件激活完成的回调：计数减一，归零则进入下一步。
void UTenkaichiExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// 把还在加载的插件计数减一
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

// 全部就绪：执行 Action → 广播三档"加载完成"。
void UTenkaichiExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != ETenkaichiExperienceLoadState::Loaded);

	// 插入一个测试用随机延迟（如果配置了的话）
	if (LoadState != ETenkaichiExperienceLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = TenkaichiConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;

			LoadState = ETenkaichiExperienceLoadState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnExperienceFullLoadCompleted, DelaySecs, /*bLooping=*/ false);

			return;
		}
	}

	LoadState = ETenkaichiExperienceLoadState::ExecutingActions;

	// 执行这些 Action
	FGameFeatureActivatingContext Context;

	// 如果设定了，就只应用到自己这个 world context 上
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action != nullptr)
			{
				//@TODO: 这些函数不带 world 参数，在客户端-服务器 PIE 下可能有问题
				// 当前行为和 GameplayTags 这类系统一致：加载和注册作用于整个进程，
				// 但把结果真正应用到 Actor 上则限制在特定 world 里
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	ActivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UTenkaichiExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	LoadState = ETenkaichiExperienceLoadState::Loaded;

	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();

	// 应用必要的可伸缩性设置
#if !UE_SERVER
	//UTenkaichiSettingsLocal::Get()->OnExperienceLoaded();   // ⚠️ 注释挂起：依赖 P18 Settings
#endif
}

// 单个 Action 停用完成的回调：暂停者计数 +1，凑齐则全部停用完成。
void UTenkaichiExperienceManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	++NumObservedPausers;

	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}

// 登记网络复制属性（当前经验）。
void UTenkaichiExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

// 组件生命周期结束：停用本经验加载的 GameFeature、卸载 Action。
void UTenkaichiExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 停用本经验加载的任何特性
	//@TODO: 这也应该按"后进先出"（FILO）处理
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		if (UTenkaichiExperienceManager::RequestToDeactivatePlugin(PluginURL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
		}
	}

	//@TODO: 确保对"部分加载"状态也有正确处理
	if (LoadState == ETenkaichiExperienceLoadState::Loaded)
	{
		LoadState = ETenkaichiExperienceLoadState::Deactivating;

		// 确保在"有人注册为暂停者但立刻触发"的情况下，我们不会过早完成转换
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		// 停用并卸载这些 Action
		FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });

		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
				}
			}
		};

		DeactivateListOfActions(CurrentExperience->Actions);
		for (const TObjectPtr<UTenkaichiExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateListOfActions(ActionSet->Actions);
			}
		}

		NumExpectedPausers = Context.GetNumPausers();

		if (NumExpectedPausers > 0)
		{
			UE_LOG(LogTenkaichiExperience, Error, TEXT("异步停用的 Action 在 Tenkaichi 经验里还没有被完全支持"));
		}

		if (NumExpectedPausers == NumObservedPausers)
		{
			OnAllActionsDeactivated();
		}
	}
}

// 问加载屏系统：还没加载完就显示加载画面。
bool UTenkaichiExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState != ETenkaichiExperienceLoadState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	else
	{
		return false;
	}
}

// 所有 Action 都停用完成：重置状态、清空当前经验。
void UTenkaichiExperienceManagerComponent::OnAllActionsDeactivated()
{
	//@TODO: 我们其实只是停用了，没有完全卸载……
	LoadState = ETenkaichiExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
	//@TODO:	GEngine->ForceGarbageCollection(true);
}