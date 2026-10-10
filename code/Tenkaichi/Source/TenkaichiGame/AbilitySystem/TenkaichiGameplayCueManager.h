// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 引擎类头文件：提供 UGameplayCueManager 基类
#include "GameplayCueManager.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiGameplayCueManager.generated.h"

class FString;
class UClass;
class UObject;
class UWorld;
struct FObjectKey;

UCLASS()
class UTenkaichiGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	UTenkaichiGameplayCueManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UTenkaichiGameplayCueManager* Get();

	//~UGameplayCueManager 接口（以下是重写父类的方法）
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;
	//~End of UGameplayCueManager 接口

	static void DumpGameplayCues(const TArray<FString>& Args);

	// 当使用「延迟加载」模式时，本方法负责加载那些「无论如何都要常驻」的 cue
	void LoadAlwaysLoadedCues();

	// 刷新「单个 GameplayCue 主资产」所对应的 bundle（打包时把 cue 归组）
	void RefreshGameplayCuePrimaryAsset();

private:
	// 监听「某个 GameplayTag 被加载」的回调
	void OnGameplayTagLoaded(const FGameplayTag& Tag);
	// 垃圾回收（GC）结束后的回调
	void HandlePostGarbageCollect();
	// 批量处理已加载的 tag
	void ProcessLoadedTags();
	// 处理「某个 tag 要预加载对应的 cue」
	void ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject);
	// 预加载一个 cue 完成的回调
	void OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue);
	// 把一个加载好的 cue 类登记进缓存
	void RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject);
	// 地图加载完成后的回调（清理失效的 cue 引用）
	void HandlePostLoadMap(UWorld* NewWorld);
	// 根据加载模式，重新挂/卸这些委托监听
	void UpdateDelayLoadDelegateListeners();
	// 判断当前是否应该「延迟加载 cue」（专用服务器不延迟）
	bool ShouldDelayLoadGameplayCues() const;

private:
	struct FLoadedGameplayTagToProcessData
	{
		FGameplayTag Tag;
		TWeakObjectPtr<UObject> WeakOwner;

		FLoadedGameplayTagToProcessData() {}
		FLoadedGameplayTagToProcessData(const FGameplayTag& InTag, const TWeakObjectPtr<UObject>& InWeakOwner) : Tag(InTag), WeakOwner(InWeakOwner) {}
	};

private:
	// 客户端上因「被内容引用」而预加载的 cue（能力表现类）
	UPROPERTY(transient)
	TSet<TObjectPtr<UClass>> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;

	// 客户端上预加载后「永远保持加载」的 cue（被代码引用，或显式要求常驻）
	UPROPERTY(transient)
	TSet<TObjectPtr<UClass>> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagToProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;
	bool bProcessLoadedTagsAfterGC = false;
};