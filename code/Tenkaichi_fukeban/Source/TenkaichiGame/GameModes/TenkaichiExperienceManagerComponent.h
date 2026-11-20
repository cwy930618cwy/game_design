// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/GameStateComponent.h"
#include "LoadingProcessInterface.h"

#include "TenkaichiExperienceManagerComponent.generated.h"

#define UE_API TENKAICHIGAME_API

namespace UE::GameFeatures { struct FResult; }

class UTenkaichiExperienceDefinition;

// 声明一个"经验加载完成"的多播委托：带一个参数（指向经验定义的指针）。
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTenkaichiExperienceLoaded, const UTenkaichiExperienceDefinition* /*Experience*/);

// 经验加载的状态机：表示"经验加载走到哪一步了"。
enum class ETenkaichiExperienceLoadState
{
	Unloaded,                // 还没加载
	Loading,                 // 正在加载资产
	LoadingGameFeatures,     // 正在激活 GameFeature 插件
	LoadingChaosTestingDelay,// 可选的测试用随机延迟
	ExecutingActions,        // 正在执行 Action
	Loaded,                  // 加载完成
	Deactivating             // 正在卸载（退出时反向走）
};

UCLASS(MinimalAPI)
class UTenkaichiExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	// 构造函数：初始化状态机。
	UE_API UTenkaichiExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent 接口开始
	// 组件生命周期结束时的回调：负责停用 GameFeature、卸载 Action。
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~UActorComponent 接口结束

	//~ILoadingProcessInterface 接口开始
	// 问加载屏系统"我是不是还没加载完、要不要显示加载画面"。
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~ILoadingProcessInterface 接口结束

	// 尝试设置当前要用的经验（可以是 UI 经验，也可以是玩法经验）。
	UE_API void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// 保证这个委托在经验加载完成后、其他委托之前被调用；
	// 但如果经验已经加载完了，就立刻调用它。
	UE_API void CallOrRegister_OnExperienceLoaded_HighPriority(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate);

	// 保证这个委托在经验加载完成后被调用；
	// 如果经验已经加载完了，就立刻调用它。
	UE_API void CallOrRegister_OnExperienceLoaded(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate);

	// 保证这个委托在经验加载完成后被调用；
	// 如果经验已经加载完了，就立刻调用它。
	UE_API void CallOrRegister_OnExperienceLoaded_LowPriority(FOnTenkaichiExperienceLoaded::FDelegate&& Delegate);

	// 返回当前经验（前提是它已经完全加载完，否则会 assert 报错）。
	UE_API const UTenkaichiExperienceDefinition* GetCurrentExperienceChecked() const;

	// 返回"经验是不是已经完全加载完"。
	UE_API bool IsExperienceLoaded() const;

private:
	// 网络复制回调：客户端收到"当前经验"后，也启动自己的加载流程。
	UFUNCTION()
	void OnRep_CurrentExperience();

	// 开始加载经验资产（Bundle + 原始资产）。
	void StartExperienceLoad();
	// 资产加载完成后的回调：收集并激活 GameFeature 插件。
	void OnExperienceLoadComplete();
	// 单个 GameFeature 插件激活完成的回调。
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	// 全部就绪后的回调：执行 Action 并广播"加载完成"。
	void OnExperienceFullLoadCompleted();

	// 单个 Action 停用完成的回调。
	void OnActionDeactivationCompleted();
	// 所有 Action 都停用完成的回调。
	void OnAllActionsDeactivated();

private:
	// 当前经验定义（网络复制，客户端通过 OnRep_CurrentExperience 同步）。
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UTenkaichiExperienceDefinition> CurrentExperience;

	// 当前加载状态。
	ETenkaichiExperienceLoadState LoadState = ETenkaichiExperienceLoadState::Unloaded;

	// 还在加载中的 GameFeature 插件计数。
	int32 NumGameFeaturePluginsLoading = 0;
	// 本经验要激活的 GameFeature 插件 URL 列表。
	TArray<FString> GameFeaturePluginURLs;

	// 卸载时"观察到的暂停者"计数。
	int32 NumObservedPausers = 0;
	// 卸载时"期望的暂停者"计数。
	int32 NumExpectedPausers = 0;

	// 经验加载完成后、先于其他委托被调用的委托（比如给"准备常规玩法"的子系统用）。
	FOnTenkaichiExperienceLoaded OnExperienceLoaded_HighPriority;

	// 经验加载完成后被调用的委托。
	FOnTenkaichiExperienceLoaded OnExperienceLoaded;

	// 经验加载完成后被调用的委托。
	FOnTenkaichiExperienceLoaded OnExperienceLoaded_LowPriority;
};

#undef UE_API