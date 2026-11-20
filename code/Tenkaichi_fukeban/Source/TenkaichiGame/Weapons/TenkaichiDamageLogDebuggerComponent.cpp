// 版权（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiDamageLogDebuggerComponent.h"

#include "Engine/World.h"
#include "TenkaichiLogChannels.h"
#include "Messages/TenkaichiVerbMessage.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiDamageLogDebuggerComponent)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Tenkaichi_Damage_Message);

// 构造函数：开启 Tick，让组件每帧执行结算
UTenkaichiDamageLogDebuggerComponent::UTenkaichiDamageLogDebuggerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

// 组件激活时调用：向消息总线注册"受伤"消息监听
void UTenkaichiDamageLogDebuggerComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = MessageSubsystem.RegisterListener(TAG_Tenkaichi_Damage_Message, this, &ThisClass::OnDamageMessage);
}

// 组件结束/销毁时调用：反注册消息监听，防止泄漏
void UTenkaichiDamageLogDebuggerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(ListenerHandle);

	Super::EndPlay(EndPlayReason);
}

// 每帧调用：距上次受伤超过阈值且本波有记录时，汇总并打印这一波伤害统计
void UTenkaichiDamageLogDebuggerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	const double TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageEntryTime;
	if ((TimeSinceDamage >= SecondsBetweenDamageBeforeLogging) && (DamageLog.Num() > 0))
	{
		TArray<FFrameDamageEntry> Entries;
		DamageLog.GenerateValueArray(/*out*/ Entries);
		DamageLog.Reset();

		// 按"第一次被打的时间"排序，得到时间先后顺序
		Entries.Sort([](const FFrameDamageEntry& A, const FFrameDamageEntry& B) { return A.TimeOfFirstHit < B.TimeOfFirstHit; });

		double TotalDamage = 0.0;
		int32 NumImpacts = 0;
		int32 NumFrames = Entries.Num();
		double MinInterval = TNumericLimits<double>::Max();
		double MaxInterval = -TNumericLimits<double>::Max();
		double TotalInterval = 0.0;

		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			FFrameDamageEntry& EntryA = Entries[i];
			NumImpacts += EntryA.NumImpacts;
			TotalDamage += EntryA.SumDamage;

			// 计算相邻两帧的时间间隔，统计整波伤害的持续时长
			if (i + 1 < Entries.Num())
			{
				FFrameDamageEntry& EntryB = Entries[i+1];

				const double TimeGap = EntryB.TimeOfFirstHit - EntryA.TimeOfFirstHit;
				MinInterval = FMath::Min(MinInterval, TimeGap);
				MaxInterval = FMath::Max(MaxInterval, TimeGap);
				TotalInterval += TimeGap;
			}
		}

		// 打印：命中次数、持续帧数、持续秒数、总伤害
		UE_LOG(LogTenkaichi, Warning, TEXT("%d impacts in %d distinct frames over %.2f seconds did %.2f damage"),
			NumImpacts, NumFrames, TotalInterval, TotalDamage);
		// 打印：相邻帧间隔的最小/最大/平均毫秒数
		if (TotalInterval > 0.0)
		{
			UE_LOG(LogTenkaichi, Warning, TEXT("Interval ranged from %.1f ms to %.1f ms (avg %.1f ms)"),
				MinInterval * 1000.0, MaxInterval * 1000.0, (MaxInterval + MinInterval) / 2.0 * 1000.0);
			// 打印：秒伤 DPS
			UE_LOG(LogTenkaichi, Warning, TEXT("DPS %.2f"), TotalDamage / TotalInterval);
		}
		UE_LOG(LogTenkaichi, Warning, TEXT("\n"));
	}
}

// 收到"受伤"消息时的回调：只有本组件挂载的 Actor 受伤才记录，按帧累积进 DamageLog
void UTenkaichiDamageLogDebuggerComponent::OnDamageMessage(FGameplayTag Channel, const FTenkaichiVerbMessage& Payload)
{
	if (Payload.Target == GetOwner())
	{
		FFrameDamageEntry& LogEntry = DamageLog.FindOrAdd(GFrameCounter);

		// 该帧第一次被打时记录时间点，并更新"上次受伤时间"
		if (LogEntry.TimeOfFirstHit == 0.0)
		{
			LogEntry.TimeOfFirstHit = GetWorld()->GetTimeSeconds();
			LastDamageEntryTime = LogEntry.TimeOfFirstHit;
		}
		LogEntry.NumImpacts++;
		LogEntry.SumDamage += -Payload.Magnitude;
	}
}