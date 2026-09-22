# `LyraUIManagerSubsystem.h` 速览

> UI 的总管（`GameInstance` 级子系统）。**它只干一件事：同步 HUD 的显示/隐藏**。

| 成员 | 干嘛的 |
|---|---|
| `: UGameUIManagerSubsystem` | 来自 CommonGame 插件 |
| `Initialize()` / `Deinitialize()` | 挂/摘 Ticker |
| `Tick()` | 每帧调一次同步 |
| `SyncRootLayoutVisibilityToShowHUD()` | ⭐ 核心 |
| `TickHandle` | `FTSTicker` 句柄 |

> 💡 用 `FTSTicker` 而不是 `Tick` 虚函数，是因为 `UGameInstanceSubsystem` 没有自带的 Tick —— FTSTicker 是标准做法。

**优先级**：`SyncRootLayoutVisibilityToShowHUD`
