# `LyraControllerDisconnectedScreen.cpp` 速览

> 100 行，五个函数。

| 函数 | 干嘛的 |
|---|---|
| `TAG_Platform_Trait_Input_HasStrictControllerPairing` | `Platform.Trait.Input.HasStrictControllerPairing` |
| 构造函数 | 默认加上"严格配对平台"这个 Trait |
| `NativeOnActivated()` | ⭐ 见下 |
| `ShouldDisplayChangeUserButton()` | 平台 Trait 全包含；**编辑器下还要并上模拟的可见性 Tag** |
| `HandleChangeUserClicked()` | 取玩家主设备 → `ShowPlatformUserSelector`，回调到 `HandleChangeUserCompleted` |
| `HandleChangeUserCompleted()` | ⚠️ **只有日志 + `// TODO: Handle any user changing logic in your game here`** —— 没实现 |

## `NativeOnActivated()` 的两层判断

```
1. 找不到 HBox_SwitchUser 或 Button_ChangeUser → Error 并 return
2. 默认都设为 Collapsed / Hidden
3. ShouldDisplayChangeUserButton() 为真
   且 "未配对用户" 的 FPlatformUserId 有效（不是所有平台都支持）
   → 才设为 SelfHitTestInvisible
4. 绑 OnClicked
```

> ⚠️ **注意这是个半完成的功能**：界面能弹出来、能叫出平台用户选择器，但**选完之后什么都不做**。要在 Lyra 上做主机开发的话，这里必须自己补。
