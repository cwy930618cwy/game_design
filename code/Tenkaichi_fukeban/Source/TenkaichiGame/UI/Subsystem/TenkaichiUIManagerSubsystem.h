// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "TenkaichiUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UObject;

// UI 管理器子系统：管理每个本地玩家的 UI 布局根，并同步 HUD 可见性。
UCLASS()
class UTenkaichiUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	// 构造函数：空实现。
	UTenkaichiUIManagerSubsystem();

	// 初始化：注册每帧 ticker。
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// 反初始化：移除 ticker。
	virtual void Deinitialize() override;

private:
	// 每帧调用：同步 UI 可见性。
	bool Tick(float DeltaTime);
	// 把 UI 根布局的可见性同步到 HUD 的 bShowHUD。
	void SyncRootLayoutVisibilityToShowHUD();
	
	// ticker 句柄。
	FTSTicker::FDelegateHandle TickHandle;
};