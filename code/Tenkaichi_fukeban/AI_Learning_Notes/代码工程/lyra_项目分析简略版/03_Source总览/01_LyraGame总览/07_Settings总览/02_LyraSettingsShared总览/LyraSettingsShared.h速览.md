# `LyraSettingsShared.h` 速览

> ⭐ **跨设备共享设置**（改键、字幕、灵敏度）。继承 `ULocalPlayerSaveGame`，**可云同步**。

| 分组 | 字段 / 方法 |
|---|---|
| **色盲** | `ColorBlindMode`、`ColorBlindStrength`（0~10） |
| **手柄** | `bForceFeedbackEnabled`（震动）、`GamepadMoveStickDeadZone` / `GamepadLookStickDeadZone`（死区）、4 个扳机马达字段 |
| **字幕** | `bEnableSubtitles`、`SubtitleTextSize` / `TextColor` / `TextBorder` / `BackgroundOpacity` |
| **音频** | `AllowAudioInBackground` |
| **语言** | `PendingCulture`、`bResetToDefaultCulture` |
| **灵敏度** | `MouseSensitivityX/Y`、`TargetingMultiplier`、`bInvertVerticalAxis`、`bInvertHorizontalAxis` |
| **手柄灵敏度** | `GamepadLookSensitivityPreset`、`GamepadTargetingSensitivityPreset`（1~10 档） |

## 三个枚举

| 枚举 | 取值 |
|---|---|
| `EColorBlindMode` | Off / Deuteranope / Protanope / Tritanope |
| `ELyraAllowBackgroundAudioSetting` | Off / AllSounds |
| `ELyraGamepadSensitivity` | Slow → Insane 共 **10 档**（`Invalid=0` 和 `MAX` 被隐藏） |

## 主要方法

| 方法 | 干嘛的 |
|---|---|
| `CreateTemporarySettings()` | 建一个临时的（还没登录时用） |
| `LoadOrCreateSettings()` | ⚠️ **同步**加载，注释说"会卡主线程" |
| `AsyncLoadOrCreateSettings()` | ⭐ 异步加载（**实际用的是这个**） |
| `SaveSettings()` / `ApplySettings()` | 保存 / 应用到各系统 |
| `IsDirty()` / `ClearDirtyFlag()` | 脏标记（决定设置页要不要显示"应用"按钮） |
| `ChangeValueAndDirty<T>()`（模板） | ⭐ 统一的"改值 + 标脏 + 广播" |

> 💡 **`ChangeValueAndDirty` 是核心小工具**：值真的变了才赋值、标脏、广播 `OnSettingChanged`。所有 setter 都走它，省掉大量重复代码。

**优先级**：`ChangeValueAndDirty` → 三个加载方法
