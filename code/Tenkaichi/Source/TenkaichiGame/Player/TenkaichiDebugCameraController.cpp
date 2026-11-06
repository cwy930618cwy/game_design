// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiDebugCameraController.h"
// #if 0  // ⚠️ 注释挂起（与 P12.4 CheatManager 死结）：本头文件 P12.4 的 .h 已建，但 .cpp 挂起
#include "TenkaichiCheatManager.h"
// #endif  // 注释挂起结束

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiDebugCameraController)


ATenkaichiDebugCameraController::ATenkaichiDebugCameraController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 和 ATenkaichiPlayerController 使用同一个作弊类，这样就能通过作弊指令开关调试相机。
	// #if 0  // ⚠️ 注释挂起（依赖 P12.4 CheatManager 类定义）
	CheatClass = UTenkaichiCheatManager::StaticClass();
	// #endif  // 注释挂起结束
}

void ATenkaichiDebugCameraController::AddCheats(bool bForce)
{
	// 和 ATenkaichiPlayerController 的 AddCheats() 保持一致，避免玩家卡在调试相机里出不来。
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}