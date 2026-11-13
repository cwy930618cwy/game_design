// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "GameFramework/HUD.h"

#include "TenkaichiHUD.generated.h"

namespace EEndPlayReason { enum Type : int; }

class AActor;
class UObject;

/**
 * ATenkaichiHUD
 *
 *  注意：通常你不需要扩展或修改这个类，而是应该在经验里用"Add Widget"动作
 *  来添加一个 HUD 布局和控件到它上面。
 *
 *  这个类的存在主要是为了调试渲染。
 */
UCLASS(Config = Game)
class ATenkaichiHUD : public AHUD
{
	GENERATED_BODY()

public:
	// 构造函数：关闭 Tick。
	ATenkaichiHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	//~UObject 接口开始
	// 组件预初始化（注册为 GameFramework 组件接收者）。
	virtual void PreInitializeComponents() override;
	//~UObject 接口结束

	//~AActor 接口开始
	// 开始游戏（广播"游戏 Actor 就绪"事件）。
	virtual void BeginPlay() override;
	// Actor 生命周期结束（注销组件接收者）。
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束

	//~AHUD 接口开始
	// 返回调试用的 Actor 列表（收集所有带能力系统的 Actor）。
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
	//~AHUD 接口结束
};