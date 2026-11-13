// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiAssetManager.h"
#include "TenkaichiLogChannels.h"
#include "TenkaichiGameplayTags.h"
#include "TenkaichiGameData.h"
#include "AbilitySystemGlobals.h"
#include "Character/TenkaichiPawnData.h"
#include "Misc/App.h"
#include "Stats/StatsMisc.h"
#include "Engine/Engine.h"
#include "AbilitySystem/TenkaichiGameplayCueManager.h"
#include "Misc/ScopedSlowTask.h"
#include "System/TenkaichiAssetManagerStartupJob.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiAssetManager)

// 定义 Bundle 常量"Equipped"。
const FName FTenkaichiBundles::Equipped("Equipped");

//////////////////////////////////////////////////////////////////////

// 控制台命令：打印所有已加载资产。
static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("Tenkaichi.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UTenkaichiAssetManager::DumpLoadedAssets)
);

//////////////////////////////////////////////////////////////////////

// 宏：添加一个带权重的启动任务。
#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FTenkaichiAssetManagerStartupJob(#JobFunc, [this](const FTenkaichiAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
// 宏：添加一个权重为 1 的启动任务。
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)

//////////////////////////////////////////////////////////////////////

// 构造函数：初始化 DefaultPawnData。
UTenkaichiAssetManager::UTenkaichiAssetManager()
{
	DefaultPawnData = nullptr;
}

// 返回 AssetManager 单例对象。
UTenkaichiAssetManager& UTenkaichiAssetManager::Get()
{
	check(GEngine);

	if (UTenkaichiAssetManager* Singleton = Cast<UTenkaichiAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogTenkaichi, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to TenkaichiAssetManager!"));

	// 上面的 Fatal 错误会阻止这行被执行。
	return *NewObject<UTenkaichiAssetManager>();
}

// 同步加载指定路径的资产。
UObject* UTenkaichiAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// 资产管理器还没就绪，就用 LoadObject。
		return AssetPath.TryLoad();
	}

	return nullptr;
}

// 返回"是否要记录资产加载日志"。
bool UTenkaichiAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

// 线程安全地把一个已加载资产加入内存保持列表。
void UTenkaichiAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

// 打印所有被资产管理器加载和追踪的资产。
void UTenkaichiAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogTenkaichi, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogTenkaichi, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogTenkaichi, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogTenkaichi, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}

// 启动初始加载（扫描 + 跑启动任务）。
void UTenkaichiAssetManager::StartInitialLoading()
{
	SCOPED_BOOT_TIMING("UTenkaichiAssetManager::StartInitialLoading");

	// 这会做所有的扫描，即使加载被延迟了现在也要做
	Super::StartInitialLoading();

	STARTUP_JOB(InitializeGameplayCueManager());

	{
		// 加载基础游戏数据资产
		STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);
	}

	// 跑所有排队的启动任务
	DoAllStartupJobs();
}

// 初始化 GameplayCue 管理器。
void UTenkaichiAssetManager::InitializeGameplayCueManager()
{
	SCOPED_BOOT_TIMING("UTenkaichiAssetManager::InitializeGameplayCueManager");

	UTenkaichiGameplayCueManager* GCM = UTenkaichiGameplayCueManager::Get();
	check(GCM);
	GCM->LoadAlwaysLoadedCues();
}


// 返回全局游戏数据。
const UTenkaichiGameData& UTenkaichiAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UTenkaichiGameData>(TenkaichiGameDataPath);
}

// 返回默认 Pawn 数据。
const UTenkaichiPawnData* UTenkaichiAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}

// 真正加载指定类的游戏数据。
UPrimaryDataAsset* UTenkaichiAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("TenkaichiEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
		UE_LOG(LogTenkaichi, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

		// 这在编辑器里可能被递归调用（因为它是在 PostLoad 里按需调用的），
		// 所以那种情况下强制同步加载主资产、异步加载其余部分
		if (GIsEditor)
		{
			Asset = DataClassPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);

				// 这应该总是能成功
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		// 加载任何 GameData 资产失败都是不可接受的，会导致难以诊断的软失败。
		UE_LOG(LogTenkaichi, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}


// 跑所有启动任务。
void UTenkaichiAssetManager::DoAllStartupJobs()
{
	SCOPED_BOOT_TIMING("UTenkaichiAssetManager::DoAllStartupJobs");
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	if (IsRunningDedicatedServer())
	{
		// 专用服务器不需要周期性进度更新，直接跑任务
		for (const FTenkaichiAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() > 0)
		{
			float TotalJobValue = 0.0f;
			for (const FTenkaichiAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				TotalJobValue += StartupJob.JobWeight;
			}

			float AccumulatedJobValue = 0.0f;
			for (FTenkaichiAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				const float JobValue = StartupJob.JobWeight;
				StartupJob.SubstepProgressDelegate.BindLambda([This = this, AccumulatedJobValue, JobValue, TotalJobValue](float NewProgress)
					{
						const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;
						const float OverallPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;

						This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
					});

				StartupJob.DoJob();

				StartupJob.SubstepProgressDelegate.Unbind();

				AccumulatedJobValue += JobValue;

				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
			}
		}
		else
		{
			UpdateInitialGameContentLoadPercent(1.0f);
		}
	}

	StartupJobs.Empty();

	UE_LOG(LogTenkaichi, Display, TEXT("All startup jobs took %.2f seconds to complete"), FPlatformTime::Seconds() - AllStartupJobsStartTime);
}

// 上报初始游戏内容加载百分比（目前空实现，可接加载屏）。
void UTenkaichiAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// 可以把状态路由到早期启动加载屏
}

#if WITH_EDITOR
// PIE 开始前（编辑器专用）。
void UTenkaichiAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("TenkaichiEditor", "BeginLoadingPIEData", "Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		const UTenkaichiGameData& LocalGameDataCommon = GetGameData();

		// 故意放在 GetGameData 之后，避免把 GameData 时间算进这个计时器
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE asset preloading complete"), nullptr);

		// 你可以在这里添加预加载任何"我们将要用的经验"需要的东西
		// （比如从世界设置抓默认经验 + 开发者设置里的经验覆盖）
	}
}
#endif