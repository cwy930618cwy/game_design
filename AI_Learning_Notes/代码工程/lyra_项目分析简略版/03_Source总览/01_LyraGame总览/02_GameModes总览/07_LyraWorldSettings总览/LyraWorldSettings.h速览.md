# `LyraWorldSettings.h` 速览

> 让**每张地图自带一份默认 Experience**。

| 成员 | 干嘛的 |
|---|---|
| `: AWorldSettings` | 标准 UE WorldSettings 的 Lyra 版 |
| `CheckForErrors()`（编辑器） | 地图检查（MapCheck）时跑，见 cpp |
| `GetDefaultGameplayExperience()` | 把软引用解析成 `FPrimaryAssetId` |
| `DefaultGameplayExperience` | `EditDefaultsOnly` 的软引用，指向一份 ExperienceDefinition |
| `ForceStandaloneNetMode`（仅编辑器数据） | ⭐ 勾上后，在编辑器里按 Play 会**强制 Standalone**（用于前端/主菜单这类关卡） |

**优先级**：`GetDefaultGameplayExperience`
