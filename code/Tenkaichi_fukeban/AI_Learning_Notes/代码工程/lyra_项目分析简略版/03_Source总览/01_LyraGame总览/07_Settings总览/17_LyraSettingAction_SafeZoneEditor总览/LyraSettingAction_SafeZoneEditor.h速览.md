# `LyraSettingAction_SafeZoneEditor.h` 速览

> **一个头文件里放了 2 个类**：安全区数值项 + 触发安全区编辑界面的动作项。

| 类 | 继承 | 干嘛的 |
|---|---|---|
| `ULyraSettingValueScalarDynamic_SafeZoneValue` | `UGameSettingValueScalarDynamic` | 安全区的数值（0~1） |
| `ULyraSettingAction_SafeZoneEditor` | `UGameSettingAction` | 一个"按钮型"设置项，点了会打开编辑界面 |

| 成员 | 干嘛的 |
|---|---|
| `SafeZoneValue::ResetToDefault()` / `RestoreToInitial()` | 重置/恢复时**同步改 Slate 全局缩放** |
| `SafeZoneEditor::GetChildSettings()` | ⭐ 返回那个数值项 —— 这就是"动作项"和"数值项"的关联方式 |
| `SafeZoneValueSetting`（private） | 构造时新建的数值项 |

> 💡 **这个模式值得注意**：`UGameSettingAction` 是"点了要干点什么"的设置项，但它自己不存值 —— 值放在它的**子设置项**里。编辑界面通过 `GetChildSettings()` 拿到数值项去读写。

**优先级**：`SafeZoneEditor` 的构造函数 → `GetChildSettings`
