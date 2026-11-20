# `LyraInputUserSettings.h` 速览

> **一个头文件里放了 2 个类**，都是改键系统的一部分。

| 类 | 继承 | 干嘛的 |
|---|---|---|
| `ULyraInputUserSettings` | `UEnhancedInputUserSettings` | ⭐ 玩家的输入设置（改键数据存这） |
| `ULyraPlayerMappableKeySettings` | `UPlayerMappableKeySettings` | 每个按键映射上的**额外元数据** |

## `ULyraInputUserSettings`

| 成员 | 干嘛的 |
|---|---|
| `ApplySettings()` | 唯一 override |

> ⚠️ **这个类当前是空的** —— 源码注释明确说明它是**给你加自定义输入设置的地方**，还给了几个建议：
> - "toggle vs. hold" 切换
> - 瞄准灵敏度
> - 并提醒：**属性要标 `UPROPERTY(SaveGame)` 才会被序列化**

## `ULyraPlayerMappableKeySettings`

| 成员 | 干嘛的 |
|---|---|
| `GetTooltipText()` | 取提示文字 |
| `Tooltip`（protected） | `EditAnywhere` 的本地化文本 |

> 💡 这个类的作用是**给每个按键映射挂一份自定义元数据**，设置界面可以读它来显示 tooltip。默认实现只加了 Tooltip 一项，你也可以照着加别的（比如图标、分类）。

**优先级**：`ULyraInputUserSettings`（作为扩展点）
