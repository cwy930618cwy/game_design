# `LyraSettingAction_SafeZoneEditor.cpp` 速览

> 47 行，**几乎全在构造函数里**。

| 函数 | 干嘛的 |
|---|---|
| `ULyraSettingAction_SafeZoneEditor()` | ⭐ 见下 |
| `GetChildSettings()` | 返回只有一项的数组 |
| `SafeZoneValue::ResetToDefault()` | Super + `SSafeZone::SetGlobalSafeZoneScale(默认值)` |
| `SafeZoneValue::RestoreToInitial()` | Super + `SSafeZone::SetGlobalSafeZoneScale(初始值)` |

## 构造函数里配置的数值项

```
DevName         = "SafeZoneValue"
显示名 / 描述    = "Safe Zone Value" / "The safezone area percentage."
默认值          = 0.0
Getter/Setter   = GET_LOCAL_SETTINGS_FUNCTION_PATH(GetSafeZone / SetSafeZone)
显示格式        = 直接显示数字
SetSettingParent(this)   ← 建立父子关系
```

> 💡 可以看到这里又用到了 `GET_LOCAL_SETTINGS_FUNCTION_PATH` 宏 —— 全目录的设置项几乎都靠它把 UI 和 `LyraSettingsLocal` 的字段连起来。
>
> 注意重置/恢复时**必须手动改 `SSafeZone::SetGlobalSafeZoneScale`**，否则值改了但屏幕上的安全区不会跟着变。

**优先级**：构造函数
