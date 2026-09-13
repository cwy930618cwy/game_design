# `LyraActivatableWidget.h` 速览

> ⭐⭐ **所有 Lyra 界面的基类**。它解决了一个关键问题：**界面打开时，输入该给谁**。

| 成员 | 干嘛的 |
|---|---|
| `ELyraWidgetInputMode` | ⭐ 四种输入模式：`Default` / `GameAndMenu` / `Game` / `Menu` |
| `: UCommonActivatableWidget`（`Abstract, Blueprintable`） | 建立在 CommonUI 之上 |
| `GetDesiredInputConfig()` | 把上面的枚举翻译成 `FUIInputConfig` 交给 CommonUI |
| `ValidateCompiledWidgetTree()`（编辑器） | ⭐ 编译期检查：子类有没有实现 `BP_GetDesiredFocusTarget` |
| `InputConfig` | 这个界面想要的输入模式 |
| `GameMouseCaptureMode` | 鼠标捕获方式，默认 `CapturePermanently` |

## 四种输入模式怎么选

| 模式 | 用途 |
|---|---|
| `GameAndMenu` | HUD —— 既要能操作角色，也要能操作菜单 |
| `Game` | 纯游戏界面 |
| `Menu` | 纯菜单（**鼠标不捕获**） |
| `Default` | 不改，交给上层默认行为 |

> 💡 **为什么这很重要**：CommonUI 靠 `GetDesiredInputConfig` 决定"界面激活时游戏还收不收按键"。没有它，打开背包时角色可能还在跟着鼠标动。

**优先级**：`GetDesiredInputConfig` → `ELyraWidgetInputMode`
