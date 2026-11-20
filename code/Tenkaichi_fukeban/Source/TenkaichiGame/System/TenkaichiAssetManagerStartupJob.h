// 版权声明（可保留或省略，不影响编译）
#pragma once

#include "Engine/StreamableManager.h"

// 声明"启动任务子步骤进度"的委托：带一个参数（新进度，float）。
DECLARE_DELEGATE_OneParam(FTenkaichiAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);

// 处理从流式句柄上报进度。
struct FTenkaichiAssetManagerStartupJob
{
	// 子步骤进度回调委托。
	FTenkaichiAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;
	// 实际要执行的加载函数（接收任务引用 + 流式句柄引用）。
	TFunction<void(const FTenkaichiAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;
	// 任务名（日志用）。
	FString JobName;
	// 进度权重。
	float JobWeight;
	// 上次上报进度的时间（节流用）。
	mutable double LastUpdate = 0;

	// 构造函数：初始化任务函数、名字、权重。
	FTenkaichiAssetManagerStartupJob(const FString& InJobName, const TFunction<void(const FTenkaichiAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc, float InJobWeight)
		: JobFunc(InJobFunc)
		, JobName(InJobName)
		, JobWeight(InJobWeight)
	{}

	// 执行真正的加载，如果创建了句柄就返回它。
	TSharedPtr<FStreamableHandle> DoJob() const;

	// 上报子步骤进度。
	void UpdateSubstepProgress(float NewProgress) const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	// 从流式句柄取进度并上报（带节流，最多每秒 60 次）。
	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
	{
		if (SubstepProgressDelegate.IsBound())
		{
			// StreamableHandle::GetProgress 遍历一张很大的图，开销不小
			double Now = FPlatformTime::Seconds();
			if (LastUpdate - Now > 1.0 / 60)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
};