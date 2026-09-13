# `LyraSettingValueDiscrete_MobileFPSType.h` 速览

> 移动端帧率上限选项（30 / 60 / ... FPS）。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscrete` | 离散值设置项基类 |
| `StoreInitial()` / `ResetToDefault()` / `RestoreToInitial()` | 三个标准生命周期 |
| 离散三件套 | `SetDiscreteOptionByIndex` / `GetDiscreteOptionIndex` / `GetDiscreteOptions` |
| `OnInitialized()` | 按平台支持的帧率列表建选项 |
| `GetValue()` / `SetValue(...)`（private） | 读写 `ULyraSettingsLocal::DesiredMobileFrameRateLimit` |
| `GetDefaultFPS()`（private） | 默认帧率 |
| `MakeLimitString()`（static private） | 生成 "60 FPS" 这种文本 |
| `InitialValue`（protected） | 初始值，用于 Restore |
| `FPSOptions`（protected） | `TSortedMap<int32, FText>` —— **有序**，保证选项顺序稳定 |

**优先级**：`OnInitialized`
