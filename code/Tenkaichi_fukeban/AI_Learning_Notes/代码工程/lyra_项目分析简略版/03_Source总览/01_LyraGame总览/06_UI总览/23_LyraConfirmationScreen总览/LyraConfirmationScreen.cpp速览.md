# `LyraConfirmationScreen.cpp` 速览

> 101 行。

| 函数 | 干嘛的 |
|---|---|
| `SetupDialog()` | ⭐ 见下 |
| `KillDialog()` | 只调 Super（**没有额外清理**） |
| `NativeOnInitialized()` | 给 `Border_TapToCloseZone` 绑鼠标按下事件 |
| `CloseConfirmationWindow()` | `DeactivateWidget()` + 执行结果回调 |
| `HandleTapToCloseZoneMouseButtonDown()` | 触摸或左键 → 当作 **Declined**（取消） |
| `ValidateCompiledDefaults()`（编辑器） | `CancelAction` 没配 → **编译 Error** |

## `SetupDialog()` 怎么造按钮

```
1. 填标题和正文
2. EntryBox_Buttons->Reset()，清空时把每个按钮的 OnClicked 也 Clear 掉
   （重要：复用旧按钮会残留旧回调）
3. 遍历 Descriptor->ButtonActions：
        Confirmed → 默认"点击"动作
        Declined  → 默认"返回"动作
        Cancelled → CancelAction（自己配的）
4. 每个动作 CreateEntry<ULyraButtonBase>：
        设触发输入动作、绑 OnClicked 传自己的 Result、设文字
5. 存下结果回调
```

> 💡 **第 2 步那个 `OnClicked().Clear()` 很关键**：`UDynamicEntryBox` 会复用条目对象，不清回调的话第二次打开对话框会触发上一次的回调，是典型 bug 源。

**优先级**：`SetupDialog`
