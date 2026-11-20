// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "AsyncAction_ExperienceReady.generated.h"

class AGameStateBase;
class UTenkaichiExperienceDefinition;
class UWorld;
struct FFrame;

// 声明一个"经验就绪"的动态多播委托（蓝图可绑定）。
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExperienceReadyAsyncDelegate);

/**
 * 异步等待 GameState 就绪且有效，然后调用 OnReady 事件。
 * 如果 GameState 已经有效，就立刻调用 OnReady。
 */
UCLASS()
class UAsyncAction_ExperienceReady : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()

public:
	// 等待经验被确定并加载完成。
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"))
	static UAsyncAction_ExperienceReady* WaitForExperienceReady(UObject* WorldContextObject);

	// 节点激活时调用：启动"等 GameState → 听经验 → 广播"的流程。
	virtual void Activate() override;

public:

	// 经验被确定且就绪/加载完成时调用。
	UPROPERTY(BlueprintAssignable)
	FExperienceReadyAsyncDelegate OnReady;

private:
	// 第 1 步：GameState 出现后的回调。
	void Step1_HandleGameStateSet(AGameStateBase* GameState);
	// 第 2 步：找到 GameState 上的经验管理器组件，开始听"经验加载完成"。
	void Step2_ListenToExperienceLoading(AGameStateBase* GameState);
	// 第 3 步：经验加载完成的回调。
	void Step3_HandleExperienceLoaded(const UTenkaichiExperienceDefinition* CurrentExperience);
	// 第 4 步：广播 OnReady，然后销毁自己。
	void Step4_BroadcastReady();

	// 弱引用保存 World，防止 World 销毁后悬空。
	TWeakObjectPtr<UWorld> WorldPtr;
};