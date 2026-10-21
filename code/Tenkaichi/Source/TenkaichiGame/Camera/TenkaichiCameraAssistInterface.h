#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "TenkaichiCameraAssistInterface.generated.h"

// 接口的 UHT 模板类：让这个接口暴露给蓝图（BlueprintType）
UINTERFACE(BlueprintType)
class UTenkaichiCameraAssistInterface : public UInterface
{
	GENERATED_BODY()
};

// 真正的接口类：定义相机穿墙的 3 个定制口子
class ITenkaichiCameraAssistInterface
{
	GENERATED_BODY()

public:
	// 获取"允许相机穿透"的 Actor 列表。第三人称相机跟随时，用来忽略掉 view target、Pawn、载具等一堆不该挡镜头的对象。
	virtual void GetIgnoredActorsForCameraPentration(TArray<const AActor*>& OutActorsAllowPenetration) const { }

	// 返回"绝不能穿透"的目标 Actor。正常情况下几乎总是 view target（未实现时默认保持 true）。
	// 但有时 view target 和你要始终框在画面里的根 Actor 不是同一个。
	virtual TOptional<AActor*> GetCameraPreventPenetrationTarget() const
	{
		return TOptional<AActor*>();
	}

	// 相机穿透焦点目标时被调用。想在重叠时隐藏目标 Actor，就在这里做。
	virtual void OnCameraPenetratingTarget() { }
};