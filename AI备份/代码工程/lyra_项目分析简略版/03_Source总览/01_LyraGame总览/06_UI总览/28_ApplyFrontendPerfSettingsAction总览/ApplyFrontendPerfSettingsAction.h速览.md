# `ApplyFrontendPerfSettingsAction.h` 速览

> 一个 GameFeatureAction：**进入主菜单时套用"前端专用"的性能设置**（比如菜单里锁 30 帧省电）。

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction`（`final`） | 菜单里显示 "Use Frontend Perf Settings" |
| `OnGameFeatureActivating()` / `OnGameFeatureDeactivating()` | 开关 |
| `ApplicationCounter`（static private） | ⭐ **静态计数器** |

> 💡 为什么需要计数器？见 cpp —— 多人 PIE 时会有多个世界同时激活这个 Action。
