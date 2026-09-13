# `LyraSettingValueDiscrete_OverallQuality.h` 速览

> "整体画质"选项（低/中/高/史诗 + 自定义）。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValueDiscrete` | 离散值设置项基类 |
| 三个标准生命周期 | `StoreInitial` / `ResetToDefault` / `RestoreToInitial` |
| 离散三件套 | `SetDiscreteOptionByIndex` / `GetDiscreteOptionIndex` / `GetDiscreteOptions` |
| `OnInitialized()` | 按平台支持的最大档位建选项列表 |
| `GetCustomOptionIndex()`（private） | "自定义"这一项的下标 |
| `GetOverallQualityLevel()`（private） | 读当前整体画质等级 |
| `Options` / `OptionsWithCustom` | ⭐ **两份列表** —— 一份带"自定义"，一份不带 |

> 💡 **两份列表的设计**：只有在玩家真的把各项调得"不统一"时，才额外显示"Custom"这一项。这样正常情况下看不到多余的选项。

**优先级**：`GetDiscreteOptionIndex`
