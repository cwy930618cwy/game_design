# `LyraSettingValueDiscrete_Resolution.cpp` 速览

> 约 400 行。**核心是"按窗口模式切换分辨率列表"**。

| 步骤 | 干嘛的 |
|---|---|
| `OnInitialized()` | 从 RHI 拿所有支持的分辨率，过滤掉太小的，分别填进三套数组 |
| `OnDependencyChanged()` | ⭐ 窗口模式变了 → `SelectAppropriateResolutions()` 换列表 |
| `SelectAppropriateResolutions()` | 按新的 `LastWindowMode` 选 `ResolutionsFullscreen` / `ResolutionsWindowedFullscreen` / `ResolutionsWindowed` |
| `GetDiscreteOptions()` | 把当前那套渲染成文本（如 `1920x1080 (60Hz)`） |
| `SetDiscreteOptionByIndex()` | 按模式调用对应的 `SetScreenResolution` / `SetFullscreenMode` 等 |
| 三个 `FindIndex...` | 精确查找 / 找不到就取最近的 / 强制返回一个合法的 |

> 💡 **窗口模式下的分辨率是"生成"的而不是"枚举"的** —— `GetStandardWindowResolutions` 按最小尺寸、最大尺寸、最小宽高比生成一组合适的分辨率，这样窗口能自由调整大小。

**优先级**：`SelectAppropriateResolutions` → `OnDependencyChanged`
