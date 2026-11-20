# `LyraSettingKeyboardInput.h` 速览

> ⭐ **单个按键绑定**的设置项（不是控件，是数据）。改键功能的核心类。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingValue` | ⭐ 注意不是 Discrete —— 它不通过索引选值 |
| `InitializeInputData()` | ⭐ 由 Registry 调用，传进来 Profile / MappingRow / 过滤选项 |
| `GetKeyTextFromSlot()` | 取某个槽位（主键/副键）当前绑的键名 |
| `StoreInitial()` / `ResetToDefault()` / `RestoreToInitial()` | 三个标准生命周期 |
| `ChangeBinding()` | ⭐ 改键：指定槽位 + 新键 |
| `GetAllMappedActionsFromKey()` | 查这个键还被哪些动作占用（用于"已绑定"警告） |
| `IsMappingCustomized()` | 是否被玩家改过（决定要不要显示"恢复默认"） |
| `GetSettingDisplayName()` / `GetSettingDisplayCategory()` | 显示名和分类 |
| `FindKeyMappingRow()` / `FindMappableKeyProfile()` / `GetUserSettings()` | 三个查找辅助（都通向 EnhancedInput 用户设置） |
| `ActionMappingName` / `ProfileIdentifier` / `QueryOptions` | 定位到具体哪一行的三要素 |
| `InitialKeyMappings` | `TMap<槽位, 键>` —— 初始值，用于 Restore |

**优先级**：`InitializeInputData` → `ChangeBinding`
