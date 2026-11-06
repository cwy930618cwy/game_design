// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiPlayerStart.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiPlayerStart)

// 构造函数：没有任何额外初始化。
ATenkaichiPlayerStart::ATenkaichiPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 检测这个出生点现在能不能放得下一个指定控制器要操控的 Pawn。
// 返回三态：空 / 部分被占 / 满。
ETenkaichiPlayerStartLocationOccupancy ATenkaichiPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* const World = GetWorld();
	// 只有服务器（权威端）才能做占用检测。
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			// 通过 GameMode 拿到这个控制器该用的 Pawn 类型。
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			// 用该类型的默认对象（CDO）来当"试身衣"测试位置。
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();

			// 用 Pawn 的包围盒在这个位置测一下：完全不撞 → 空位。
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return ETenkaichiPlayerStartLocationOccupancy::Empty;
			}
			// 撞到了，但稍微挪一下就能放下 → 部分被占。
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return ETenkaichiPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	// 完全放不下 → 满。
	return ETenkaichiPlayerStartLocationOccupancy::Full;
}

// 这个出生点是否已经被某个控制器认领了？
bool ATenkaichiPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

// 没被占就认领给指定控制器，并启动一个循环定时器定期检查是否该释放。
bool ATenkaichiPlayerStart::TryClaim(AController* OccupyingController)
{
	// 只有给了控制器、且当前没被认领，才认领成功。
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			// 每隔 ExpirationCheckInterval 秒检查一次是否该释放。
			World->GetTimerManager().SetTimer(ExpirationTimerHandle, FTimerDelegate::CreateUObject(this, &ATenkaichiPlayerStart::CheckUnclaimed), ExpirationCheckInterval, true);
		}
		return true;
	}
	return false;
}

// 定时器回调：认领者已经离开、且位置空出来了，就释放认领并停掉定时器。
void ATenkaichiPlayerStart::CheckUnclaimed()
{
	// 只有认领者存在、它已经有 Pawn、且这个位置空出来了，才释放。
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) == ETenkaichiPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}