# `LyraAssetManagerStartupJob.h` 速览

> 一个**纯数据的任务单元**（不是 UObject，是普通 struct），给 `LyraAssetManager` 的启动队列用。

| 成员 | 干嘛的 |
|---|---|
| `SubstepProgressDelegate` | 上报进度的委托（单参数 float） |
| `JobFunc` | 真正要干的活，签名带 `TSharedPtr<FStreamableHandle>&` 以便支持异步加载 |
| `JobName` | 任务名，打日志用 |
| `JobWeight` | 权重，决定它在总进度里占多少 |
| `LastUpdate` | `mutable double`，记录上次更新进度的时刻 |
| `DoJob()` | 执行任务，返回 StreamableHandle |
| `UpdateSubstepProgress()` | 直接上报一次进度 |
| `UpdateSubstepProgressFromStreamable()` | 从 Streamable handle 取进度上报，**节流到每 1/60 秒一次**（注释：算这个很贵） |

**说明**：`JobWeight` 是这套系统能算出总百分比的关键 —— 见 `LyraAssetManager::DoAllStartupJobs`。
