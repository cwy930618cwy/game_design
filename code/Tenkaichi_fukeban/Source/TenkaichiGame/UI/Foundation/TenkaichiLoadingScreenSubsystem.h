// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"

#include "UObject/WeakObjectPtr.h"
#include "TenkaichiLoadingScreenSubsystem.generated.h"

#define UE_API TENKAICHIGAME_API

class UObject;
class UUserWidget;
struct FFrame;

// 声明"加载屏控件变化"的动态多播委托（蓝图可绑）。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTenkaichiLoadingScreenWidgetChangedDelegate, TSubclassOf<UUserWidget>, NewWidgetClass);

/**
 * 追踪/存储当前的加载屏配置，放在一个能跨地图切换存活的地方。
 */
UCLASS(MinimalAPI)
class UTenkaichiLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 构造函数：空实现。
	UE_API UTenkaichiLoadingScreenSubsystem();

	// 设置加载屏内容控件类（显示在加载屏宿主里）。
	UFUNCTION(BlueprintCallable)
	UE_API void SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	// 返回上次设置的加载屏内容控件类。
	UFUNCTION(BlueprintPure)
	UE_API TSubclassOf<UUserWidget> GetLoadingScreenContentWidget() const;

private:
	// 加载屏控件变化事件（蓝图可绑）。
	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FTenkaichiLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;

	// 当前加载屏内容控件类。
	UPROPERTY()
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
};

#undef UE_API