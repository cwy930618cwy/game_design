# `LyraSettingValueDiscrete_OverallQuality.cpp` 速览

> 105 行，**最值得看的是"自定义"这个动态选项**。

| 函数 | 干嘛的 |
|---|---|
| `OnInitialized()` | ⭐ 见下 |
| `StoreInitial()` / `ResetToDefault()` / `RestoreToInitial()` | ⚠️ **三个都是空函数** |
| `SetDiscreteOptionByIndex()` | 选到"自定义"就什么都不做；否则 `SetOverallScalabilityLevel(Index)` |
| `GetDiscreteOptionIndex()` | 当前等级为 `INDEX_NONE` 就返回"自定义"下标 |
| `GetDiscreteOptions()` | ⭐ 同理：等级有效返回 `Options`，无效返回 `OptionsWithCustom` |
| `GetOverallQualityLevel()` | 读 `UGameUserSettings::GetOverallScalabilityLevel()` |

## `OnInitialized()` 的两件事

```
① AddOptionIfPossible：按平台支持的 MaxQualityLevel 决定要不要加这一档
   （低端机可能只有 Low / Medium）
② 如果移动端"某个画质档位会限制帧率"，就加一条警告文本：
   "Note: Changing the Quality setting to X or higher might limit your framerate."
```

> 💡 **"自定义"是被动出现的**：`GetOverallScalabilityLevel()` 在各项一致时返回那个档位，不一致时返回 `INDEX_NONE`。Lyra 用这个返回值判断要不要多显示一个 Custom 选项 —— 不用自己维护状态。
>
> ⚠️ 那三个空函数意味着 **StoreInitial / ResetToDefault / RestoreToInitial 对这个设置项不生效** —— 点"恢复初始"不会真的恢复画质。

**优先级**：`GetDiscreteOptions` → `OnInitialized`
