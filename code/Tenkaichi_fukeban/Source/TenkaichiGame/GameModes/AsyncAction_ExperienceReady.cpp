// 版权声明（可保留或省略，不影响编译）
#include "GameModes/AsyncAction_ExperienceReady.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameModes/TenkaichiExperienceManagerComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ExperienceReady)

// 构造函数：空实现（基类构造函数处理初始化）。
UAsyncAction_ExperienceReady::UAsyncAction_ExperienceReady(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 静态入口：创建节点、保存 World、注册到 GameInstance 保活。
UAsyncAction_ExperienceReady* UAsyncAction_ExperienceReady::WaitForExperienceReady(UObject* InWorldContextObject)
{
	UAsyncAction_ExperienceReady* Action = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UAsyncAction_ExperienceReady>();
		Action->WorldPtr = World;
		Action->RegisterWithGameInstance(World);
	}

	return Action;
}

// 节点激活：GameState 有了就听经验，没有就先等 GameState 出现。
void UAsyncAction_ExperienceReady::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			Step2_ListenToExperienceLoading(GameState);
		}
		else
		{
			World->GameStateSetEvent.AddUObject(this, &ThisClass::Step1_HandleGameStateSet);
		}
	}
	else
	{
		// 没有 World，就永远不会自然完成了
		SetReadyToDestroy();
	}
}

// 第 1 步：GameState 出现后的回调（先移除监听，再进第 2 步）。
void UAsyncAction_ExperienceReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}

	Step2_ListenToExperienceLoading(GameState);
}

// 第 2 步：找到经验管理器组件，经验已加载就延迟一帧广播，否则注册"加载完成"回调。
void UAsyncAction_ExperienceReady::Step2_ListenToExperienceLoading(AGameStateBase* GameState)
{
	check(GameState);
	UTenkaichiExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UTenkaichiExperienceManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		// 经验恰好已经加载完了，但还是故意延迟一帧，
		// 确保没人写出"依赖这个节点永远异步"的代码。
		//@TODO: 考虑对动态生成的东西 / 加载屏已经消失之后的任何时刻不延迟？
		//@TODO: 也许直接在经验加载里注入一个随机 0-1 秒延迟？
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::Step4_BroadcastReady));
	}
	else
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnTenkaichiExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::Step3_HandleExperienceLoaded));
	}
}

// 第 3 步：经验加载完成的回调（直接进第 4 步）。
void UAsyncAction_ExperienceReady::Step3_HandleExperienceLoaded(const UTenkaichiExperienceDefinition* CurrentExperience)
{
	Step4_BroadcastReady();
}

// 第 4 步：广播 OnReady，然后销毁自己。
void UAsyncAction_ExperienceReady::Step4_BroadcastReady()
{
	OnReady.Broadcast();

	SetReadyToDestroy();
}