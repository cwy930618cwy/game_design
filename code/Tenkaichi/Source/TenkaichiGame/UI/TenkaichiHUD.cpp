// 版权声明（可保留或省略，不影响编译）
#include "TenkaichiHUD.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Async/TaskGraphInterfaces.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiHUD)

class AActor;
class UWorld;

//////////////////////////////////////////////////////////////////////
// ATenkaichiHUD

// 构造函数：关闭 Tick。
ATenkaichiHUD::ATenkaichiHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// 组件预初始化（注册为 GameFramework 组件接收者）。
void ATenkaichiHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// 注册为"GameFramework 组件接收者"：这样 ComponentManager 能给它发扩展事件
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 开始游戏（广播"游戏 Actor 就绪"事件）。
void ATenkaichiHUD::BeginPlay()
{
	// 广播"游戏 Actor 就绪"——P15.7 AddWidgets 监听的就是这个事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

// Actor 生命周期结束（注销组件接收者）。
void ATenkaichiHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

// 返回调试用的 Actor 列表（收集所有带能力系统的 Actor）。
void ATenkaichiHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	UWorld* World = GetWorld();

	Super::GetDebugActorList(InOutList);

	// 添加所有带能力系统组件的 Actor。
	for (TObjectIterator<UAbilitySystemComponent> It; It; ++It)
	{
		if (UAbilitySystemComponent* ASC = *It)
		{
			// 跳过 CDO 和原型对象
			if (!ASC->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				AActor* AvatarActor = ASC->GetAvatarActor();
				AActor* OwnerActor = ASC->GetOwnerActor();

				if (AvatarActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AvatarActor))
				{
					AddActorToDebugList(AvatarActor, InOutList, World);
				}
				else if (OwnerActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor))
				{
					AddActorToDebugList(OwnerActor, InOutList, World);
				}
			}
		}
	}
}