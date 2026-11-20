# `LyraSettingsShared.cpp` 速览

> 264 行。**存档槽名是 `SharedGameSettings`**。

| 函数 | 干嘛的 |
|---|---|
| 两个 CVar | `gpad.DefaultLeftStickInnerDeadZone` / `...RightStick...`（默认都是 **0.25**） |
| 构造函数 | 订阅语言变化事件；用 CVar 初始化两个死区 |
| `GetLatestDataVersion()` | 返回 1（注释：0 是还没继承 `ULocalPlayerSaveGame` 时的版本） |
| `CreateTemporarySettings()` / `LoadOrCreateSettings()` / `AsyncLoadOrCreateSettings()` | 三种创建方式，**最后都会 `ApplySettings()`** |
| `SaveSettings()` | ⭐ 异步存档 + **顺便保存 EnhancedInput 的改键设置** |
| `ApplySettings()` | 依次应用：字幕 → 后台音频 → 语言 → **EnhancedInput 用户设置** |
| 色盲两个 setter | ⭐ 直接调 `FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(...)` 立即生效 |
| `ApplySubtitleOptions()` | 组装 `FSubtitleFormat` 交给 `USubtitleDisplaySubsystem` |
| `ApplyBackgroundAudioSettings()` | ⭐ **只对主玩家生效**：`FApp::SetUnfocusedVolumeMultiplier(1 或 0)` |
| 语言相关一组 | `PendingCulture` 是"待应用"的，真正 `SetCurrentCulture` 时才写进 `GGameUserSettingsIni` |

## ⚠️ 一个值得注意的空函数

```cpp
void ULyraSettingsShared::ApplyInputSensitivity()
{
}
```

> 所有灵敏度 setter 都会调它，但**它是空的**。也就是说鼠标灵敏度的实际生效**不在这里** —— 而是在 EnhancedInput 的修饰器里（`LyraInputModifiers` / `LyraAimSensitivityData`）。这点容易误解。

**优先级**：`ApplySettings` → `SaveSettings` → 语言那一组
