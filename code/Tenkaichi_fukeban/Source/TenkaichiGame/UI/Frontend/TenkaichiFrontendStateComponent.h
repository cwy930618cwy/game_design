// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Components/GameStateComponent.h"
#include "ControlFlowNode.h"
#include "LoadingProcessInterface.h"

#include "TenkaichiFrontendStateComponent.generated.h"

class FControlFlow;
class FString;
class FText;
class UObject;
struct FFrame;

enum class ECommonUserOnlineContext : uint8;
enum class ECommonUserPrivilege : uint8;
class UCommonActivatableWidget;
class UCommonUserInfo;
class UTenkaichiExperienceDefinition;

// 前端状态机：用 ControlFlow 编排"等用户初始化 → Press Start → 加入会话 → 主菜单"。
UCLASS(Abstract)
class UTenkaichiFrontendStateComponent : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	// 构造函数：空实现。
	UTenkaichiFrontendStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent 接口开始
	// 开始游戏：监听经验加载完成。
	virtual void BeginPlay() override;
	// 生命周期结束。
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~UActorComponent 接口结束

	//~ILoadingProcessInterface 接口开始
	// 是否显示加载屏（前端流程没走完就显示）。
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~ILoadingProcessInterface 接口结束

private:
	// 经验加载完成：创建并执行前端流程。
	void OnExperienceLoaded(const UTenkaichiExperienceDefinition* Experience);

	// 用户初始化完成的回调。
	UFUNCTION()
	void OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	// 流程步骤 1：等用户初始化。
	void FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow);
	// 流程步骤 2：尝试显示 Press Start 屏。
	void FlowStep_TryShowPressStartScreen(FControlFlowNodeRef SubFlow);
	// 流程步骤 3：尝试加入请求的会话。
	void FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow);
	// 流程步骤 4：尝试显示主菜单。
	void FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow);

	// 是否应该显示加载屏。
	bool bShouldShowLoadingScreen = true;

	// Press Start 屏的控件类（软引用）。
	UPROPERTY(EditAnywhere, Category = UI)
	TSoftClassPtr<UCommonActivatableWidget> PressStartScreenClass;

	// 主菜单的控件类（软引用）。
	UPROPERTY(EditAnywhere, Category = UI)
	TSoftClassPtr<UCommonActivatableWidget> MainScreenClass;

	// 前端流程对象。
	TSharedPtr<FControlFlow> FrontEndFlow;
	
	// 如果设置了，这是正在进行的 Press Start 屏任务
	FControlFlowNodePtr InProgressPressStartScreen;

	// 加入会话完成事件的句柄。
	FDelegateHandle OnJoinSessionCompleteEventHandle;
};