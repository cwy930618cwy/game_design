// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h
#include "TenkaichiGameplayCueManager.h"
#include "Engine/AssetManager.h"
#include "TenkaichiLogChannels.h"
#include "GameplayCueSet.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "UObject/UObjectThreadContext.h"
#include "Async/Async.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameplayCueManager)

enum class ELyraEditorLoadMode
{
	// 提前全部加载：编辑器里加载时间更长，但 PIE 更快，特效永不播放失败
	LoadUpfront,

	// 编辑器外：cue 的 tag 一注册就异步加载
	// 编辑器内：cue 被调用时才异步加载
	//   注意：PIE 里可能出现「X 的特效怎么没出来」的问题，适合迭代快，但对策划不友好
	PreloadAsCuesAreReferenced_GameOnly,

	// tag 一注册就异步加载
	PreloadAsCuesAreReferenced
};

namespace TenkaichiGameplayCueManagerCvars
{
	static FAutoConsoleCommand CVarDumpGameplayCues(
		TEXT("Tenkaichi.DumpGameplayCues"),
		TEXT("Shows all assets that were loaded via TenkaichiGameplayCueManager and are currently in memory."),
		FConsoleCommandWithArgsDelegate::CreateStatic(UTenkaichiGameplayCueManager::DumpGameplayCues));

	static ELyraEditorLoadMode LoadMode = ELyraEditorLoadMode::LoadUpfront;
}

const bool bPreloadEvenInEditor = true;

struct FGameplayCueTagThreadSynchronizeGraphTask : public FAsyncGraphTaskBase
{
	TFunction<void()> TheTask;
	FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) : TheTask(MoveTemp(Task)) { }
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) { TheTask(); }
	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
};

UTenkaichiGameplayCueManager::UTenkaichiGameplayCueManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UTenkaichiGameplayCueManager* UTenkaichiGameplayCueManager::Get()
{
	return Cast<UTenkaichiGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

void UTenkaichiGameplayCueManager::OnCreated()
{
	Super::OnCreated();

	UpdateDelayLoadDelegateListeners();
}

void UTenkaichiGameplayCueManager::LoadAlwaysLoadedCues()
{
	if (ShouldDelayLoadGameplayCues())
	{
		UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

		//@TODO: 尝试通过「从本地 GameplayTag 里过滤出 GameplayCue. 前缀」来收集这些 tag？
		TArray<FName> AdditionalAlwaysLoadedCueTags;

		for (const FName& CueTagName : AdditionalAlwaysLoadedCueTags)
		{
			FGameplayTag CueTag = TagManager.RequestGameplayTag(CueTagName, /*ErrorIfNotFound=*/ false);
			if (CueTag.IsValid())
			{
				ProcessTagToPreload(CueTag, nullptr);
			}
			else
			{
				UE_LOG(LogTenkaichi, Warning, TEXT("UTenkaichiGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"), *CueTagName.ToString());
			}
		}
	}
}

bool UTenkaichiGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	switch (TenkaichiGameplayCueManagerCvars::LoadMode)
	{
        case ELyraEditorLoadMode::LoadUpfront:
            return true;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
            #if WITH_EDITOR
                    if (GIsEditor)
                    {
                        return false;
                    }
            #endif
            break;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced:
            break;
	}

	return !ShouldDelayLoadGameplayCues();
}

bool UTenkaichiGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
	return false;
}

bool UTenkaichiGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
	return true;
}

void UTenkaichiGameplayCueManager::DumpGameplayCues(const TArray<FString>& Args)
{
	UTenkaichiGameplayCueManager* GCM = Cast<UTenkaichiGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
	if (!GCM)
	{
		UE_LOG(LogTenkaichi, Error, TEXT("DumpGameplayCues failed. No UTenkaichiGameplayCueManager found."));
		return;
	}

	const bool bIncludeRefs = Args.Contains(TEXT("Refs"));

	UE_LOG(LogTenkaichi, Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
	for (UClass* CueClass : GCM->AlwaysLoadedCues)
	{
		UE_LOG(LogTenkaichi, Log, TEXT("  %s"), *GetPathNameSafe(CueClass));
	}

	UE_LOG(LogTenkaichi, Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
	for (UClass* CueClass : GCM->PreloadedCues)
	{
		TSet<FObjectKey>* ReferencerSet = GCM->PreloadedCueReferencers.Find(CueClass);
		int32 NumRefs = ReferencerSet ? ReferencerSet->Num() : 0;
		UE_LOG(LogTenkaichi, Log, TEXT("  %s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);
		if (bIncludeRefs && ReferencerSet)
		{
			for (const FObjectKey& Ref : *ReferencerSet)
			{
				UObject* RefObject = Ref.ResolveObjectPtr();
				UE_LOG(LogTenkaichi, Log, TEXT("    ^- %s"), *GetPathNameSafe(RefObject));
			}
		}
	}

	UE_LOG(LogTenkaichi, Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
	int32 NumMissingCuesLoaded = 0;
	if (GCM->RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const FGameplayCueNotifyData& CueData : GCM->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
		{
			if (CueData.LoadedGameplayCueClass && !GCM->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass) && !GCM->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
			{
				NumMissingCuesLoaded++;
				UE_LOG(LogTenkaichi, Log, TEXT("  %s"), *CueData.LoadedGameplayCueClass->GetPathName());
			}
		}
	}

	UE_LOG(LogTenkaichi, Log, TEXT("=========== Gameplay Cue Notify summary ==========="));
	UE_LOG(LogTenkaichi, Log, TEXT("  ... %d cues in always loaded list"), GCM->AlwaysLoadedCues.Num());
	UE_LOG(LogTenkaichi, Log, TEXT("  ... %d cues in preloaded list"), GCM->PreloadedCues.Num());
	UE_LOG(LogTenkaichi, Log, TEXT("  ... %d cues loaded on demand"), NumMissingCuesLoaded);
	UE_LOG(LogTenkaichi, Log, TEXT("  ... %d cues in total"), GCM->AlwaysLoadedCues.Num() + GCM->PreloadedCues.Num() + NumMissingCuesLoaded);
}

// 监听「某个 GameplayTag 被加载」的回调
void UTenkaichiGameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
	FScopeLock ScopeLock(&LoadedGameplayTagsToProcessCS);
	bool bStartTask = LoadedGameplayTagsToProcess.Num() == 0;
	FUObjectSerializeContext* LoadContext = FUObjectThreadContext::Get().GetSerializeContext();
	UObject* OwningObject = LoadContext ? LoadContext->SerializedObject : nullptr;
	LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);
	if (bStartTask)
	{
		TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady([]()
			{
				if (GIsRunning)
				{
					if (UTenkaichiGameplayCueManager* StrongThis = Get())
					{
						// 如果正在垃圾回收，不能调 StaticFindObject（以及其他一些静态 UObject 函数），
						// 所以先等 GC 结束，再处理这些 tag
						if (IsGarbageCollecting())
						{
							StrongThis->bProcessLoadedTagsAfterGC = true;
						}
						else
						{
							StrongThis->ProcessLoadedTags();
						}
					}
				}
			});
	}
}

void UTenkaichiGameplayCueManager::HandlePostGarbageCollect()
{
	if (bProcessLoadedTagsAfterGC)
	{
		ProcessLoadedTags();
	}
	bProcessLoadedTagsAfterGC = false;
}

// 批量处理已加载的 tag
void UTenkaichiGameplayCueManager::ProcessLoadedTags()
{
	TArray<FLoadedGameplayTagToProcessData> TaskLoadedGameplayTagsToProcess;
	{
		// 只锁「拷贝一份 + 清空」这么短的时间
		FScopeLock TaskScopeLock(&LoadedGameplayTagsToProcessCS);
		TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
		LoadedGameplayTagsToProcess.Empty();
	}

	// 关停时可能返回，此时不继续处理
	if (GIsRunning)
	{
		if (RuntimeGameplayCueObjectLibrary.CueSet)
		{
			for (const FLoadedGameplayTagToProcessData& LoadedTagData : TaskLoadedGameplayTagsToProcess)
			{
				if (RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(LoadedTagData.Tag))
				{
					if (!LoadedTagData.WeakOwner.IsStale())
					{
						ProcessTagToPreload(LoadedTagData.Tag, LoadedTagData.WeakOwner.Get());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogTenkaichi, Warning, TEXT("UTenkaichiGameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing."));
		}
	}
}

void UTenkaichiGameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
	switch (TenkaichiGameplayCueManagerCvars::LoadMode)
	{
        case ELyraEditorLoadMode::LoadUpfront:
            return;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
            #if WITH_EDITOR
                    if (GIsEditor)
                    {
                        return;
                    }
            #endif
            break;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced:
            break;
	}

	check(RuntimeGameplayCueObjectLibrary.CueSet);

	int32* DataIdx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag);
	if (DataIdx && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
	{
		const FGameplayCueNotifyData& CueData = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx];

		UClass* LoadedGameplayCueClass = FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString());
		if (LoadedGameplayCueClass)
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
		}
		else
		{
			bool bAlwaysLoadedCue = OwningObject == nullptr;
			TWeakObjectPtr<UObject> WeakOwner = OwningObject;
			StreamableManager.RequestAsyncLoad(CueData.GameplayCueNotifyObj, FStreamableDelegate::CreateUObject(this, &ThisClass::OnPreloadCueComplete, CueData.GameplayCueNotifyObj, WeakOwner, bAlwaysLoadedCue), FStreamableManager::DefaultAsyncLoadPriority, false, false, TEXT("GameplayCueManager"));
		}
	}
}

// 预加载一个 cue 完成的回调
void UTenkaichiGameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue)
{
	if (bAlwaysLoadedCue || OwningObject.IsValid())
	{
		if (UClass* LoadedGameplayCueClass = Cast<UClass>(Path.ResolveObject()))
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject.Get());
		}
	}
}

// 把一个加载好的 cue 类登记进缓存
void UTenkaichiGameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
	check(LoadedGameplayCueClass);

	const bool bAlwaysLoadedCue = OwningObject == nullptr;
	if (bAlwaysLoadedCue)
	{
		AlwaysLoadedCues.Add(LoadedGameplayCueClass);
		PreloadedCues.Remove(LoadedGameplayCueClass);
		PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
	}
	else if ((OwningObject != LoadedGameplayCueClass) && (OwningObject != LoadedGameplayCueClass->GetDefaultObject()) && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass))
	{
		PreloadedCues.Add(LoadedGameplayCueClass);
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass);
		ReferencerSet.Add(OwningObject);
	}
}

void UTenkaichiGameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
	if (RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (UClass* CueClass : AlwaysLoadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}

		for (UClass* CueClass : PreloadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}
	}

	for (auto CueIt = PreloadedCues.CreateIterator(); CueIt; ++CueIt)
	{
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindChecked(*CueIt);
		for (auto RefIt = ReferencerSet.CreateIterator(); RefIt; ++RefIt)
		{
			if (!RefIt->ResolveObjectPtr())
			{
				RefIt.RemoveCurrent();
			}
		}
		if (ReferencerSet.Num() == 0)
		{
			PreloadedCueReferencers.Remove(*CueIt);
			CueIt.RemoveCurrent();
		}
	}
}

// 根据加载模式，重新挂/卸这些委托监听
void UTenkaichiGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	switch (TenkaichiGameplayCueManagerCvars::LoadMode)
	{
        case ELyraEditorLoadMode::LoadUpfront:
            return;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
            #if WITH_EDITOR
                    if (GIsEditor)
                    {
                        return;
                    }
            #endif
            break;
        case ELyraEditorLoadMode::PreloadAsCuesAreReferenced:
            break;
	}

	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

// 判断当前是否应该「延迟加载 cue」（专用服务器不延迟）
bool UTenkaichiGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
	const bool bClientDelayLoadGameplayCues = true;
	return !IsRunningDedicatedServer() && bClientDelayLoadGameplayCues;
}

const FPrimaryAssetType UFortAssetManager_GameplayCueRefsType = TEXT("GameplayCueRefs");
const FName UFortAssetManager_GameplayCueRefsName = TEXT("GameplayCueReferences");
const FName UFortAssetManager_LoadStateClient = FName(TEXT("Client"));

void UTenkaichiGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
	TArray<FSoftObjectPath> CuePaths;
	UGameplayCueSet* RuntimeGameplayCueSet = GetRuntimeCueSet();
	if (RuntimeGameplayCueSet)
	{
		RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
	}

	FAssetBundleData BundleData;
	BundleData.AddBundleAssetsTruncated(UFortAssetManager_LoadStateClient, CuePaths);

	FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(UFortAssetManager_GameplayCueRefsType, UFortAssetManager_GameplayCueRefsName);
	UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}ndleData);
}undleData);
}