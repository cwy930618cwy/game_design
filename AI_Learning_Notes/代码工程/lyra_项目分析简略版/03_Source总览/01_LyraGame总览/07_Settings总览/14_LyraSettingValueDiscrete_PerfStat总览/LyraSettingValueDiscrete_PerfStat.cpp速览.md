# `LyraSettingValueDiscrete_PerfStat.cpp` 速览

> 134 行。

| 内容 | 干嘛的 |
|---|---|
| `FGameSettingEditCondition_PerfStatAllowed` | ⭐ 见下 |
| `SetStat()` | 存 stat、**自动起一个 DevName**（`PerfStat_数字`）、并挂上上面的 EditCondition |
| `AddMode()` | 平行地往两个数组加一项 |
| `OnInitialized()` | 加 4 个选项：None / Text Only / Graph Only / Text and Graph |
| `StoreInitial()` | 记下当前模式 |
| `ResetToDefault()` | 设成 `Hidden` |
| `RestoreToInitial()` | 恢复成 `InitialMode` |
| `SetDiscreteOptionByIndex()` / `GetDiscreteOptionIndex()` | 读写 `ULyraSettingsLocal::SetPerfStatDisplayState` / `GetPerfStatDisplayState` |

## `FGameSettingEditCondition_PerfStatAllowed` 怎么判断

```
遍历 ULyraPerformanceSettings->UserFacingPerformanceStats（若干分组）
    如果某分组包含这个 stat，且该分组的 VisibilityQuery 匹配当前可见性 Tag
        → 允许显示
都不匹配 → Hide（不是 Disable）
```

> 💡 **用 Hide 而不是 Disable**：不支持的 stat 直接**不出现在列表里**，而不是灰着。体验更好。
>
> 源码还留了个 TODO：这套按平台过滤的逻辑应该做成 per-platform 配置，而不是运行时查可见性 Tag。

**优先级**：`FGameSettingEditCondition_PerfStatAllowed`
