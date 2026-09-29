# `LyraUIMessaging.cpp` 速览

> 50 行，三个函数，**两个 Show 函数几乎一模一样**。

| 函数 | 干嘛的 |
|---|---|
| `TAG_UI_LAYER_MODAL` | `UI.Layer.Modal` —— 弹窗专用层 |
| `Initialize()` | 把两个软引用 `LoadSynchronous()` 成硬引用 |
| `ShowConfirmation()` | 见下 |
| `ShowError()` | 同上，只是用 `ErrorDialogClassPtr` |

## Show 的流程

```
1. 取 UCommonLocalPlayer
2. 取 RootUILayout (UPrimaryGameLayout)
3. PushWidgetToLayerStack<UCommonGameDialog>(
       UI.Layer.Modal,
       对话框类,
       [回调](UCommonGameDialog& Dialog){ Dialog.SetupDialog(描述符, 结果回调); })
```

> 💡 **用 `UI.Layer.Modal` 这个独立层很关键** —— 弹窗要盖在所有界面之上，独立一层才能保证层级正确，也方便统一处理"弹窗时屏蔽下层输入"。

**优先级**：`ShowConfirmation`
