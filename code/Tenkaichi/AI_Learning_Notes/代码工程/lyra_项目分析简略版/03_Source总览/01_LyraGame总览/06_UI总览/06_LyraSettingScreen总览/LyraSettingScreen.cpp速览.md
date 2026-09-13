# `LyraSettingScreen.cpp` 速览

> 75 行，六个函数。

| 函数 | 干嘛的 |
|---|---|
| `NativeOnInitialized()` | 用 `RegisterUIActionBinding` 绑定三个动作（返回/应用/取消） |
| `CreateRegistry()` | `NewObject<ULyraGameSettingRegistry>()` 并用 `ULyraLocalPlayer` 初始化 |
| `HandleBackAction()` | ⭐ 先尝试 `AttemptToPopNavigation()`（还在子页面就先退出子页面）；否则 `ApplyChanges()` + `DeactivateWidget()` —— **返回即保存** |
| `HandleApplyAction()` | `ApplyChanges()` |
| `HandleCancelChangesAction()` | `CancelChanges()` |
| `OnSettingsDirtyStateChanged_Implementation()` | ⭐ 设置变脏 → 动态挂上"应用"和"取消"两个绑定；不脏 → 摘掉 |

> 💡 **两个值得注意的交互细节**：
> 1. **返回键会保存设置**（`ApplyChanges` 然后关闭），不是丢弃 —— 和很多游戏的习惯不同。
> 2. **"应用/取消"按钮是动态出现/消失的**，只有真的改了东西才显示。这是靠上面那个 `OnSettingsDirtyStateChanged` 实现的。

**优先级**：`HandleBackAction` → `OnSettingsDirtyStateChanged_Implementation`
