# `LyraSettingValueDiscrete_Language.h` 速览

> 语言选项。引擎没有现成的"语言设置项"，所以自己写一个。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscrete` | 离散值设置项基类 |
| `StoreInitial()` / `ResetToDefault()` / `RestoreToInitial()` | 三个标准生命周期 |
| `SetDiscreteOptionByIndex()` / `GetDiscreteOptionIndex()` / `GetDiscreteOptions()` | 离散选项三件套 |
| `OnInitialized()` / `OnApply()` | 初始化 / 应用 |
| `AvailableCultureNames`（protected） | 可用语言名列表（**下标 0 是"系统默认"**） |

**优先级**：`GetDiscreteOptionIndex`（匹配逻辑最复杂）
