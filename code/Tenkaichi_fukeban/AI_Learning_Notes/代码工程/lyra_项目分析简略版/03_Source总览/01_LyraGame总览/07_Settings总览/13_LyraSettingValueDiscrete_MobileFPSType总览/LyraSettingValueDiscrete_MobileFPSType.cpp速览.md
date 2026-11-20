# `LyraSettingValueDiscrete_MobileFPSType.cpp` 速览

> 102 行。**和 OverallQuality 是一对**，两者互相牵制。

| 函数 | 干嘛的 |
|---|---|
| `OnInitialized()` | ⭐ 见下 |
| `GetDefaultFPS()` | 返回 `GetDefaultMobileFrameRate()` |
| `MakeLimitString()` | `FText::Format("{0} FPS", ...)` |
| `StoreInitial()` | 记下 `InitialValue` |
| `ResetToDefault()` / `RestoreToInitial()` | 都走 `SetValue` |
| `SetDiscreteOptionByIndex()` | 从 `FPSOptions` 取出键（帧率值），越界就用默认 |
| `GetDiscreteOptionIndex()` | 反查当前值在哪个位置 |
| `GetDiscreteOptions()` | 直接返回 map 的 value 数组 |
| `GetValue()` / `SetValue()` | 读写 `ULyraSettingsLocal` |

## `OnInitialized()` 的关键

```
遍历 PlatformSettings->MobileFrameRateLimits
    用 IsSupportedMobileFramePace(TestLimit) 过滤
    加入 FPSOptions

如果"某个帧率开始会限制画质"，加警告：
    "Note: Changing the framerate setting to X or higher might lower your Quality Presets."
```

> 💡 **帧率 ↔ 画质 的双向牵制**：移动端常见限制是"想要 60 帧，分辨率质量就不能超过 75"。所以这两项都会显示警告，并且**各自都依赖对方**（在 `_Video.cpp` 里互相 `AddEditDependency`）。

**优先级**：`OnInitialized`
