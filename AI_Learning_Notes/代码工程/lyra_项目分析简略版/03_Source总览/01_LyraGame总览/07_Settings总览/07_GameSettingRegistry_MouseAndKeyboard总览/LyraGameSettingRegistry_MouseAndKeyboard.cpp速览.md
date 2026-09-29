# `LyraGameSettingRegistry_MouseAndKeyboard.cpp` 速览

> 221 行。**改键功能的入口就在这**。

| 内容 | 干嘛的 |
|---|---|
| `WhenPlatformSupportsMouseAndKeyboard` | 平台不支持键鼠就整组禁用 |
| `InitializeMouseAndKeyboardSettings()` | ⭐ 两个分组 |

## 两个分组

| 分组 | 设置项 | 存在哪 |
|---|---|---|
| **Sensitivity** | X 轴灵敏度、Y 轴灵敏度、瞄准灵敏度、反转垂直、反转水平 | **Shared** |
| **Keyboard & Mouse**（改键） | ⭐ **自动生成** —— 遍历 EnhancedInput 的所有键位配置，每个生成一个 `ULyraSettingKeyboardInput` | Shared |

## 改键列表怎么自动生成的

```
1. 取 UEnhancedInputUserSettings
2. 遍历它所有的 KeyProfiles
3. 遍历每个 Profile 的 PlayerMappingRows
4. 过滤：只要"键鼠类"的键（用 FPlayerMappableKeyQueryOptions，配 EKeys::W 做基准）
5. 按 DisplayCategory 分组（没有分类的归到 "Default Experiences"）
6. 每个生成一个 ULyraSettingKeyboardInput
```

> 💡 **这就是"改键界面不用手写"的原因**：它直接**反射 EnhancedInput 的用户设置**，你加了新按键映射，设置页会自动多出一项。
>
> 注意 `GetOrCreateSettingCollection` 里那句注释：`// If you want to just get one profile pair, then you can do UserSettings->GetCurrentProfile` —— 当前实现是列出**所有** profile。

**优先级**：`InitializeMouseAndKeyboardSettings`（重点看改键那一段）
