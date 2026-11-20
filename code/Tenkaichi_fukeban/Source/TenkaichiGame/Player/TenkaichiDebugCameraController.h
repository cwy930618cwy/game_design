// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/DebugCameraController.h"

#include "TenkaichiDebugCameraController.generated.h"

class UObject;


/**
 * ATenkaichiDebugCameraController
 *
 *	通过作弊管理器开启调试相机时，用来控制这个相机的控制器。
 */
UCLASS()
class ATenkaichiDebugCameraController : public ADebugCameraController
{
	GENERATED_BODY()

public:

	ATenkaichiDebugCameraController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 强制加载作弊类（覆写引擎钩子）。
	virtual void AddCheats(bool bForce) override;
};