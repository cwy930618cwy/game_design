// 版权声明（可保留或省略，不影响编译）
#include "GameFeatureAction_SplitscreenConfig.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_SplitscreenConfig)

#define LOCTEXT_NAMESPACE "TenkaichiGameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_SplitscreenConfig

// 定义静态成员（全局投票表）。
TMap<FObjectKey, int32> UGameFeatureAction_SplitscreenConfig::GlobalDisableVotes;

// 游戏特性停用时调用：撤掉本 Action 投的票。
void UGameFeatureAction_SplitscreenConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (int32 i = LocalDisableVotes.Num() - 1; i >= 0; i-- )
	{
		FObjectKey ViewportKey = LocalDisableVotes[i];
		UGameViewportClient* GVP = Cast<UGameViewportClient>(ViewportKey.ResolveObjectPtr());
		const FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GVP);
		if (GVP && WorldContext)
		{
			if (!Context.ShouldApplyToWorldContext(*WorldContext))
			{
				// 上下文不对，忽略它；已销毁的对象算作本上下文的一部分
				continue;
			}
		}

		int32& VoteCount = GlobalDisableVotes[ViewportKey];
		if (VoteCount <= 1)
		{
			GlobalDisableVotes.Remove(ViewportKey);

			if (GVP && WorldContext)
			{
				GVP->SetForceDisableSplitscreen(false);
			}
		}
		else
		{
			--VoteCount;
		}
		LocalDisableVotes.RemoveAt(i);
	}
}

// 在某个 world 里执行：给 viewport 投票禁用分屏。
void UGameFeatureAction_SplitscreenConfig::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	if (bDisableSplitscreen)
	{
		if (UGameInstance* GameInstance = WorldContext.OwningGameInstance)
		{
			if (UGameViewportClient* VC = GameInstance->GetGameViewportClient())
			{
				FObjectKey ViewportKey(VC);

				LocalDisableVotes.Add(ViewportKey);

				int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
				VoteCount++;
				if (VoteCount == 1)
				{
					VC->SetForceDisableSplitscreen(true);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE