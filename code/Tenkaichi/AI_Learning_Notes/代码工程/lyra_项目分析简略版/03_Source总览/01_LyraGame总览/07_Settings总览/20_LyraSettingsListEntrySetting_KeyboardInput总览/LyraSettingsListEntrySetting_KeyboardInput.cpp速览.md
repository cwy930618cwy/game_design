# `LyraSettingsListEntrySetting_KeyboardInput.cpp` 速览

> 172 行。**改键交互的完整实现**。

| 函数 | 干嘛的 |
|---|---|
| `PressAnyKeyLayer` | 用 `UI.Layer.Modal` 层弹"请按键"面板 |
| `SetSetting()` | 转成 `ULyraSettingKeyboardInput` 并 Refresh |
| `NativeOnInitialized()` | 给 4 个按钮绑点击 |
| 主/副键点击 | 推入"请按键"面板，订阅选中和取消 |
| 选中回调 | 摘掉回调 → `ChangeBinding(0 或 1, 键)` |
| `ChangeBinding()` | ⭐ 见下 |
| 重复键确认回调 | 用户确认后**用 `OriginalKeyToBind` 真正改** |
| `HandleClearClicked()` | 两个槽位都设成 `EKeys::Invalid` |
| `HandleResetToDefaultClicked()` | 调设置项的 `ResetToDefault()` |
| `Refresh()` | 更新两个按钮文字；**只在改过时才显示"恢复默认"** |
| `NativeOnEntryReleased()` | ⭐ 把 `KeyboardInputSetting` 置空（条目复用，防止悬空引用） |

## `ChangeBinding()` 的流程（处理"键已被占用"）

```
1. 先记下 OriginalKeyToBind = 想绑的键
2. 查这个键还被哪些动作占用
3. 如果被占用：
       弹"键已绑定"警告框，列出冲突的动作名
       用户确认 → 用 OriginalKeyToBind 再改一次
       用户取消 → 什么都不做
4. 没被占用 → 直接改
```

> 💡 **为什么要先存 `OriginalKeyToBind`**：因为警告框的回调里拿不到用户最初想绑的键，得提前存起来。
>
> `NativeOnEntryReleased` 里置空是列表控件的标准做法 —— `UGameSettingListEntry` 会被 ListView 复用，不清理就会拿着上一个设置项的引用。

**优先级**：`ChangeBinding` → `Refresh`
