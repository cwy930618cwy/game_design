# `LyraAssetManagerStartupJob.cpp` 速览

> 25 行，只有 `DoJob()` 一个函数。

| 步骤 | 干嘛的 |
|---|---|
| 计时开始 | `FPlatformTime::Seconds()` |
| 打日志 | `Startup job "xxx" starting` |
| `JobFunc(*this, Handle)` | 执行真正的任务 |
| 如果产生了 Handle | 绑定更新回调 → `WaitUntilComplete(0.0f, false)` → **解绑**回调 |
| 打日志 | `Startup job "xxx" took x.xx seconds to complete` |

> 💡 `WaitUntilComplete(0.0f, false)` 让它**变成同步等待**，所以即使任务本身是异步加载，整个启动流程仍然是阻塞串行的。

**说明**：日志文件 `Saved/Logs` 里能用这些行看到每个启动任务花了多久。
