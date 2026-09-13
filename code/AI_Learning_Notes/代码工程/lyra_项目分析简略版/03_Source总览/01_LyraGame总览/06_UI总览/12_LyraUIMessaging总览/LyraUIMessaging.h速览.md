# `LyraUIMessaging.h` 速览

> 弹窗子系统：**统一显示确认框和错误框**。

| 成员 | 干嘛的 |
|---|---|
| `: UCommonMessagingSubsystem` | 来自 CommonGame 插件 |
| `Initialize()` | 预加载两个对话框类 |
| `ShowConfirmation()` / `ShowError()` | ⭐ 显示确认/错误对话框 |
| `ConfirmationDialogClassPtr` / `ErrorDialogClassPtr` | 已加载的硬引用 |
| `ConfirmationDialogClass` / `ErrorDialogClass` | ⭐ `UPROPERTY(config)` 的软引用 —— **类在 ini 里配** |

> 💡 **对话框类是用 `TSoftClassPtr` + `config` 配置的**，所以不同项目/不同模式可以换成自己的对话框样式，不用改代码。

**优先级**：`ShowConfirmation`
