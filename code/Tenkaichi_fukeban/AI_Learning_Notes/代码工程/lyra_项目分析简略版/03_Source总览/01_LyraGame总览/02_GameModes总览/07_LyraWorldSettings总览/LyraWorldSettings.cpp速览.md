# `LyraWorldSettings.cpp` 速览

> 55 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `GetDefaultGameplayExperience()` | 软引用不为空时用 `GetPrimaryAssetIdForPath` 转换；**转换失败打 Error 并提示"你可能需要给这个 GameFeature 插件配 Asset Rules 或项目设置"** |
| `CheckForErrors()`（编辑器） | 遍历地图里所有 `APlayerStart`，如果发现是**原生 `APlayerStart` 而不是 `ALyraPlayerStart`**，就在 MapCheck 里报警告，提示替换 |

> 💡 World Settings 上的配置经常被忽略。PIE 时 Experience 没按预期走，先确认两件事：这里 `DefaultGameplayExperience` 有没有配，以及有没有被 `LyraGameMode` 里更高优先级的来源（尤其是 PIE 下的 Developer Settings）覆盖掉。

**优先级**：`GetDefaultGameplayExperience`
