// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/AssetManager.h"
#include "TenkaichiAssetManagerStartupJob.h"
#include "Templates/SubclassOf.h"
#include "TenkaichiAssetManager.generated.h"

#define UE_API TENKAICHIGAME_API

class UPrimaryDataAsset;

class UTenkaichiGameData;
class UTenkaichiPawnData;

// 定义 Bundle 名常量（用于按 Bundle 加载资产）。
struct FTenkaichiBundles
{
	static const FName Equipped;
};


/**
 * UTenkaichiAssetManager
 *
 *	资产管理器的游戏实现，覆写功能并存储游戏特定类型。
 *	大多数游戏都想要覆写 AssetManager，因为它是放"游戏特定加载逻辑"的好地方。
 *	这个类通过在 DefaultEngine.ini 里设置 'AssetManagerClassName' 来使用。
 */
UCLASS(MinimalAPI, Config = Game)
class UTenkaichiAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	// 构造函数：初始化 DefaultPawnData。
	UE_API UTenkaichiAssetManager();

	// 返回 AssetManager 单例对象。
	static UE_API UTenkaichiAssetManager& Get();

	// 返回 TSoftObjectPtr 引用的资产（如果还没加载就同步加载）。
	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// 返回 TSoftClassPtr 引用的子类（如果还没加载就同步加载）。
	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// 打印所有当前被资产管理器加载和追踪的资产。
	static UE_API void DumpLoadedAssets();

	// 返回全局游戏数据。
	UE_API const UTenkaichiGameData& GetGameData();
	// 返回默认 Pawn 数据。
	UE_API const UTenkaichiPawnData* GetDefaultPawnData() const;

protected:
	// 加载（或取缓存的）指定类型的游戏数据。
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// 如果需要就做一次阻塞加载
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName()));
	}


	// 同步加载指定路径的资产。
	static UE_API UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);
	// 返回"是否要记录资产加载日志"。
	static UE_API bool ShouldLogAssetLoads();

	// 线程安全地把一个已加载资产加入内存保持列表。
	UE_API void AddLoadedAsset(const UObject* Asset);

	//~UAssetManager 接口开始
	// 启动初始加载（扫描 + 跑启动任务）。
	UE_API virtual void StartInitialLoading() override;
#if WITH_EDITOR
	// PIE 开始前（编辑器专用）。
	UE_API virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~UAssetManager 接口结束

	// 真正加载指定类的游戏数据。
	UE_API UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:

	// 要使用的全局游戏数据资产。
	UPROPERTY(Config)
	TSoftObjectPtr<UTenkaichiGameData> TenkaichiGameDataPath;

	// 已加载的游戏数据版本。
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;

	// 玩家状态上没有设置 Pawn 数据时，生成玩家 Pawn 用的 Pawn 数据。
	UPROPERTY(Config)
	TSoftObjectPtr<UTenkaichiPawnData> DefaultPawnData;

private:
	// 冲刷 StartupJobs 数组，处理所有启动工作。
	UE_API void DoAllStartupJobs();

	// 设置能力系统。
	UE_API void InitializeGameplayCueManager();

	// 加载过程中周期性调用，可用来把状态喂给加载屏。
	UE_API void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	// 启动时要执行的任务列表，用来追踪启动进度。
	TArray<FTenkaichiAssetManagerStartupJob> StartupJobs;

private:

	// 被资产管理器加载和追踪的资产。
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	// 修改加载资产列表时用的作用域锁。
	FCriticalSection LoadedAssetsCritical;
};


// 模板函数内联实现（返回 TSoftObjectPtr 引用的资产）。
template<typename AssetType>
AssetType* UTenkaichiAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// 加入已加载资产列表
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

// 模板函数内联实现（返回 TSoftClassPtr 引用的子类）。
template<typename AssetType>
TSubclassOf<AssetType> UTenkaichiAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			// 加入已加载资产列表
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}

#undef UE_API