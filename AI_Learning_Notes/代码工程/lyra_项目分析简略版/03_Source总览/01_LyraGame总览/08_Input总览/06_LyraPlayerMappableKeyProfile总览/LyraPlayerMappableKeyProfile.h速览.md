# `LyraPlayerMappableKeyProfile.h` 速览

> 改键配置档（比如"默认配置"和"玩家自定义配置"）。

| 成员 | 干嘛的 |
|---|---|
| `: UEnhancedPlayerMappableKeyProfile` | EnhancedInput 的键位配置档基类 |
| `EquipProfile()` | ⭐ 这个配置档被启用时调用 |
| `UnEquipProfile()` | ⭐ 被停用/切换走时调用 |

> 💡 "Key Profile" 的意义：玩家可以在多套按键方案之间切换（比如"标准"和"左手模式"）。切换时旧的 `UnEquip`、新的 `Equip`。
>
> 这两个函数是**给你写自定义逻辑的钩子** —— 比如切换配置档时通知 UI 刷新、或者做点别的事。

**优先级**：两个函数（作为扩展点）
