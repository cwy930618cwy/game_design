# `LyraSettingValueDiscrete_Resolution.h` 速览

> 分辨率选项。**难点在于"不同窗口模式下可用分辨率不同"**。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscrete` | 离散值设置项基类 |
| 三个标准生命周期 | `StoreInitial` / `ResetToDefault` / `RestoreToInitial` |
| 离散三件套 | `SetDiscreteOptionByIndex` / `GetDiscreteOptionIndex` / `GetDiscreteOptions` |
| `OnInitialized()` / `OnDependencyChanged()` | ⭐ 后者在"窗口模式"变化时重建列表 |
| `InitializeResolutions()` | 枚举 RHI 支持的分辨率 |
| `ShouldAllowFullScreenResolution()` | 过滤掉太小的全屏分辨率 |
| `GetStandardWindowResolutions()`（static） | 生成标准窗口分辨率列表（按最小/最大尺寸和宽高比） |
| `SelectAppropriateResolutions()` | ⭐ 按当前窗口模式挑选合适的那套 |
| `FindIndexOfDisplayResolution()` / `...ForceValid()` / `FindClosestResolutionIndex()` | 三个查找辅助 |
| `LastWindowMode` | 记住上次是什么模式，用于判断要不要重建 |
| 4 个 `Resolutions*` 数组 | ⭐ 全屏 / 窗口全屏 / 窗口 各一套，加一个当前生效的 |
| `FScreenResolutionEntry` | 内部结构：宽高 + 刷新率 + 可选覆盖文本 |

> 💡 **为什么需要 4 套数组**：窗口模式下可选任意分辨率，全屏模式只能用显示器支持的，窗口全屏只能等于桌面分辨率。切换模式时要换一套列表 —— 这就是 `OnDependencyChanged` 的作用。

**优先级**：`SelectAppropriateResolutions` → `OnDependencyChanged`
