# `LyraPlayerMappableKeyProfile.cpp` 速览

> 18 行，**两个函数，每个都只调 Super**。

| 函数 | 干嘛的 |
|---|---|
| `EquipProfile()` | `Super::EquipProfile();` + 注释 "Do anything you may want to when a new key profile is equipped" |
| `UnEquipProfile()` | `Super::UnEquipProfile();` + 注释 "Do anything you may want to when a new key profile is unequipped" |

> ⚠️ **和 `LyraInputUserSettings` 一样，这是个纯扩展点** —— Lyra 自己没加任何逻辑。

**说明**：如果你想在切换按键方案时做点什么（比如弹提示、记录埋点），这里就是地方。
