# `LyraSettingsLocal.h` 速览

> ⭐ **本机设置**（画质、音量、帧率）。继承 `UGameUserSettings`，几乎每个字段都标 `UPROPERTY(Config)`。

| 分组 | 字段 |
|---|---|
| **性能统计** | `DisplayStatList`（Map：哪个 stat 用什么显示模式）、`bEnableLatencyFlashIndicators`、`bEnableLatencyTrackingStats` |
| **亮度** | `DisplayGamma`（默认 2.2） |
| **帧率** | `FrameRateLimit_OnBattery` / `_InMenu` / `_WhenBackgrounded` / `_Always` |
| **移动端质量** | `MobileFrameRateLimit`（默认 30）、`DeviceDefaultScalabilitySettings`、`DesiredMobileFrameRateLimit` |
| **主机质量预设** | `UserChosenDeviceProfileSuffix`、`DesiredUserChosenDeviceProfileSuffix` |
| **音频-音量** | `OverallVolume` / `MusicVolume` / `SoundFXVolume` / `DialogueVolume` / `VoiceChatVolume` |
| **音频-设置** | `bUseHeadphoneMode`、`bUseHDRAudioMode`、`AudioOutputDeviceId`、`ControlBusMap` |
| **安全区** | `SafeZoneScale`（默认 -1 表示未设置） |
| **改键相关** | `ControllerPlatform`、`ControllerPreset`、`InputConfigName` |
| **回放** | `bShouldAutoRecordReplays`、`NumberOfReplaysToKeep`（默认 5） |

## 主要方法

| 方法 | 干嘛的 |
|---|---|
| `Get()` | 全局单例访问 |
| `OnExperienceLoaded()` / `OnHotfixDeviceProfileApplied()` | 两个外部时机钩子 |
| `SetShouldUseFrontendPerformanceSettings()` | 菜单专用性能模式（配合 `ApplyFrontendPerfSettingsAction`） |
| `GetPerfStatDisplayState()` / `SetPerfStatDisplayState()` | 性能面板显示哪些 stat |
| `CanRunAutoBenchmark()` / `RunAutoBenchmark()` | 自动跑分定画质 |
| `UpdateGameModeDeviceProfileAndFps()` | 按当前模式更新设备配置和帧率 |
| `UpdateConsoleFramePacing()` / `UpdateDesktopFramePacing()` / `UpdateMobileFramePacing()` | 三种平台各自的帧节奏策略 |

> 💡 **Local vs Shared 的分工**：机器相关的（分辨率、画质、音量）放这；玩家相关的（改键、字幕、灵敏度）放 `LyraSettingsShared`。
>
> `FLyraScalabilitySnapshot` 是个辅助结构，用来快照/恢复一整套画质等级。

**优先级**：`UpdateGameModeDeviceProfileAndFps` → 帧率相关
