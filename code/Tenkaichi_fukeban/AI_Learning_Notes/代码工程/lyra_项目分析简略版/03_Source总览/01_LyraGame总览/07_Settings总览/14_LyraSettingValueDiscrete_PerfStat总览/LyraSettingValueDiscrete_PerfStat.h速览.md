# `LyraSettingValueDiscrete_PerfStat.h` 速览

> 单个性能统计项的显示模式（隐藏 / 文字 / 图表 / 都要）。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscrete` | 离散值设置项基类 |
| `SetStat()` | ⭐ 指定这个设置项管哪个 stat |
| 三个标准生命周期 | `StoreInitial` / `ResetToDefault` / `RestoreToInitial` |
| 离散三件套 | `SetDiscreteOptionByIndex` / `GetDiscreteOptionIndex` / `GetDiscreteOptions` |
| `OnInitialized()` | 添加 4 个模式选项 |
| `AddMode()`（protected） | 同时往 `Options` 和 `DisplayModes` 里加一项 |
| `Options` / `DisplayModes` | 显示文本 和 对应枚举值，**两个数组平行** |
| `StatToDisplay` / `InitialMode` | 管的 stat、初始模式 |

**优先级**：`SetStat` → `OnInitialized`
