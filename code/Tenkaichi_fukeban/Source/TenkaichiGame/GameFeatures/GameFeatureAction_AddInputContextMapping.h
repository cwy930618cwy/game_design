// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "GameFeatureAction_AddInputContextMapping.generated.h"

class AActor;
class UInputMappingContext;
class UPlayer;
class APlayerController;
struct FComponentRequestHandle;

// 一个输入映射上下文 + 优先级 + 是否注册到用户设置。
USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	// 输入映射上下文资产。
	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// 优先级更高（数值更大）的输入映射会优先于优先级低的。
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;

	// /** 如果为 true，这个映射上下文会在这个 game feature action 注册时被注册进用户设置。 */
	UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;
};

/**
 * 把 InputMappingContext 加到本地玩家的 EnhancedInput 系统。
 * 前提是本地玩家已经设置为使用 EnhancedInput 系统。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Mapping"))
class UGameFeatureAction_AddInputContextMapping final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction 接口开始
	// 游戏特性注册时调用（把 IMC 注册到用户设置）。
	virtual void OnGameFeatureRegistering() override;
	// 游戏特性激活时调用。
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	// 游戏特性停用时调用。
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	// 游戏特性注销时调用（把 IMC 从用户设置注销）。
	virtual void OnGameFeatureUnregistering() override;
	//~UGameFeatureAction 接口结束

	//~UObject 接口开始
#if WITH_EDITOR
	// 编辑器数据校验。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~UObject 接口结束

	// 要添加的输入映射列表。
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<FInputMappingContextAndPriority> InputMappings;

private:
	// 每个上下文（一次激活）的数据。
	struct FPerContextData
	{
		// 扩展请求句柄（解绑用）。
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// 已经添加过输入映射的玩家控制器（弱引用，防悬空）。
		TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;
	};

	// 上下文 → 该上下文的运行时数据。
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** 当 game instance 变化时用来注册 IMC 的委托句柄。 */
	FDelegateHandle RegisterInputContextMappingsForGameInstanceHandle;

	/** 把本 Action 拥有的输入映射上下文注册到输入注册子系统。同时绑定 GameInstance 启动、以及本地玩家增删的事件。 */
	void RegisterInputMappingContexts();

	/** 把本 Action 拥有的输入映射上下文注册到指定 GameInstance 的输入注册子系统。这也会在 GameInstance 启动时被调用。 */
	void RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);

	/** 把本 Action 拥有的输入映射上下文注册到指定本地玩家的输入注册子系统。这也会在本地玩家被添加时被调用。 */
	void RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	/** 把本 Action 拥有的输入映射上下文从输入注册子系统注销。同时解绑 GameInstance 启动、以及本地玩家增删的事件。 */
	void UnregisterInputMappingContexts();

	/** 把本 Action 拥有的输入映射上下文从指定 GameInstance 的输入注册子系统注销。 */
	void UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);

	/** 把本 Action 拥有的输入映射上下文从指定本地玩家的输入注册子系统注销。这也会在本地玩家被移除时被调用。 */
	void UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	//~UGameFeatureAction_WorldActionBase 接口开始
	// 在某个 world 里执行：注册玩家控制器扩展处理器。
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~UGameFeatureAction_WorldActionBase 接口结束

	// 重置某个上下文的数据。
	void Reset(FPerContextData& ActiveData);
	// 玩家控制器扩展事件回调。
	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	// 给玩家添加输入映射。
	void AddInputMappingForPlayer(UPlayer* Player, FPerContextData& ActiveData);
	// 移除输入映射。
	void RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData);
};