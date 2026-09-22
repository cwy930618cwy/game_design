# `LyraSettingKeyboardInput.cpp` 速览

> 257 行。**它是 EnhancedInput 用户设置的适配器**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `bReportAnalytics = false` |
| `GetSettingDisplayName()` / `GetSettingDisplayCategory()` | 都从 `Row->Mappings` 第一个元素读，读不到返回 "Unknown Mapping" |
| `FindKeyMappingRow()` / `FindMappableKeyProfile()` / `GetUserSettings()` | 三级查找链：LocalPlayer → EISubsystem → UserSettings → Profile → Row |
| `OnInitialized()` | ⭐ 设一个 `DynamicDetails` lambda，让详情文字动态显示"Bindings for XXX" |
| `InitializeInputData()` | ⭐ 见下 |
| `GetKeyTextFromSlot()` | 按槽位过滤后返回键的显示名；找不到返回 Invalid |
| `ResetToDefault()` | `Settings->ResetAllPlayerKeysInRow(Args, FailureReason)` |
| `StoreInitial()` | 把每个槽位的当前键记下来 |
| `RestoreToInitial()` | 逐个 `ChangeBinding` 恢复 |
| `ChangeBinding()` | ⭐ **拒绝手柄键**（`!NewKey.IsGamepadKey()`）→ `MapPlayerKey(Args, FailureReason)` |
| `GetAllMappedActionsFromKey()` | 转发 `GetMappingNamesForKey` |
| `IsMappingCustomized()` | 遍历映射，任一 `IsCustomized()` 就为真 |

## `InitializeInputData()` 做的事

```
1. 记下 ProfileIdentifier 和 QueryOptions
2. 遍历 MappingData.Mappings：
       用 DoesMappingPassQueryOptions 过滤（不符合就跳过）
       记下 ActionMappingName
       把「槽位 → 当前键」记进 InitialKeyMappings
       如果映射有显示名，就设为设置项的显示名
3. DevName 设成 "KBM_Input_" + 动作名
```

> 💡 **为什么要过滤**：同一个动作可能同时绑了键盘键和手柄键。键鼠设置页只该显示键盘键，所以传进来带 `EKeys::W` 基准的 `QueryOptions` 做过滤。
>
> `ChangeBinding` 里拒绝手柄键是第二道保险。

**优先级**：`InitializeInputData` → `ChangeBinding`
