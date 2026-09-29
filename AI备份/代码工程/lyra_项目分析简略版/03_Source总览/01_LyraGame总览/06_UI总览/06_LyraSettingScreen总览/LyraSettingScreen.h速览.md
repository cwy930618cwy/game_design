# `LyraSettingScreen.h` 速览

> 设置界面的容器（带 Tab 分类）。建立在 `UGameSettingScreen` 之上。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingScreen`（`Abstract`, `DisableNativeTick`） | 来自 GameSettings 插件 |
| `NativeOnInitialized()` | 绑定三个输入动作 |
| `CreateRegistry()` | ⭐ 创建并初始化 `ULyraGameSettingRegistry` |
| `HandleBackAction()` / `HandleApplyAction()` / `HandleCancelChangesAction()` | 返回 / 应用 / 取消 |
| `OnSettingsDirtyStateChanged_Implementation()` | ⭐ **只有设置被改过时才挂上"应用/取消"按钮** |
| `TopSettingsTabs` | `BindWidget`，顶部的 Tab 列表 |
| 三个 `FDataTableRowHandle` | 返回/应用/取消 三个输入动作的数据 |
| 三个 `FUIActionBindingHandle` | 对应的绑定句柄 |

**优先级**：`CreateRegistry` → `OnSettingsDirtyStateChanged_Implementation`
