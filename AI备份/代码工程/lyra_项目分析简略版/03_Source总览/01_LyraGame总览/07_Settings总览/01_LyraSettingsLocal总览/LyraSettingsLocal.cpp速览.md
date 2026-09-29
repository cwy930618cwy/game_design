# `LyraSettingsLocal.cpp` 速览

> 约 1000 行，**全目录最复杂的实现**。移动端画质钳制是主要难点。

| 内容 | 干嘛的 |
|---|---|
| 一堆 CVar | 控制"设备配置驱动的帧率/同步方式"、移动端的三个质量限制表；另有 3 个**仅编辑器**的 CVar（PIE 里是否应用帧率/前端性能/设备配置，默认都是 false） |
| `TMobileQualityWrapper<T>` | ⭐ 模板类：**解析并缓存** `"FPS:值,FPS2:值2"` 这种格式的字符串，支持按帧率查询限制值 |
| `LyraSettingsHelpers` 命名空间 | 平台 Trait 查询、取最高画质档、从 DeviceProfile 填充画质快照、三个质量限制表的单例 |
| `ConstrainFrameRateToBeCompatibleWithOverallQuality()` | ⭐ 在"想要的帧率"和"想要的质量"之间找平衡 |
| `SetToDefaults()` / `LoadSettings()` / `ResetToCurrentSettings()` | 生命周期 |
| `BeginDestroy()` | 摘掉应用激活状态回调 |
| 三个 `UpdateXXXFramePacing()` | 主机用 `rhi.SyncInterval`、PC 用平滑帧率、移动端用设备配置 |

## `SetToDefaults()` 里的默认值

```
耳机模式 / HDR 音频 = false
延迟统计 = 平台是否支持
设备配置后缀 = 平台的默认后缀
菜单帧率 144 / 后台 30 / 电池 60
移动端帧率 = GetDefaultMobileFrameRate()
```

## 那个模板类解决了什么

移动端常有这种需求：**帧率设到 60 时，分辨率质量最高只能是 75**。
Lyra 把这种规则做成可配置的字符串：`"0:100,30:100,60:75"`，
`TMobileQualityWrapper` 负责解析 + 排序 + **只在 CVar 变化时重新解析**（缓存）。

> 💡 **两处 `static_assert(sizeof(Scalability::FQualityLevels) == 88, ...)`** —— 引擎改了这个结构体大小时，这里会编译报错提醒更新。这是防御性的好习惯。

**优先级**：`TMobileQualityWrapper` → `ConstrainFrameRateToBeCompatibleWithOverallQuality` → 三个 FramePacing
