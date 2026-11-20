# `LyraGameSettingRegistry_Video.cpp` 速览

> 696 行，**全目录最大的文件**。视频（画质）设置全在这。

| 内容 | 干嘛的 |
|---|---|
| 两个自定义 EditCondition 类 | `FGameSettingEditCondition_FramePacingMode`（按帧节奏模式启用/禁用）、`FGameSettingEditCondition_VideoQuality`（平台不支持就没有） |
| `InitializeVideoSettings()` | ⭐ 组装 4 个子分组 |
| `InitializeVideoSettings_FrameRates()` | 4 个帧率上限选项 |
| `AddFrameRateOptions()`（自由函数） | 从 `ULyraPerformanceSettings` 读可选帧率列表，再加"无限" |

## 视频设置的 4 个分组

| 分组 | 包含的设置项 |
|---|---|
| **Display** | 窗口模式、分辨率、性能统计页 |
| **Graphics** | 色盲模式 + 强度、亮度、安全区 |
| **Graphics Quality** | 质量预设、移动帧率、自动设置、3D 分辨率、全局光照、阴影、抗锯齿、视距、纹理、特效、反射、后处理 |
| **Advanced Graphics** | 垂直同步 |

## 几个设计细节

| 点 | 说明 |
|---|---|
| 平台差异怎么处理 | 大量用 `FWhenPlatformHasTrait::KillIfMissing`（如"不支持窗口模式"就没这项） |
| 设置项之间的联动 | `AddEditDependency` —— 如"分辨率"依赖"窗口模式"；"质量预设"依赖"自动设置" |
| ⭐ 双向依赖 | 大多数细项都 `GraphicsQualityPresets->AddEditDependency(Setting)` —— 改任一细项会把预设变成"自定义" |
| 移动/主机差异 | 用 `FramePacingMode`（MobileStyle / ConsoleStyle / DesktopStyle）区分该显示哪套选项 |

> 💡 **`static_assert((int32)ELyraDisplayablePerformanceStat::Count == 18, ...)`** 这类断言也在同目录其它文件里出现 —— 提醒你加新 stat 时记得回来改设置页。

**优先级**：`InitializeVideoSettings` → 两个 EditCondition 类
