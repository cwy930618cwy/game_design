# `LyraDevelopmentStatics.h` 速览

> 只在**编辑器/开发**阶段有意义的工具库，多数函数读的是 `ULyraDeveloperSettings`。

| 函数 | 干嘛的 |
|---|---|
| `ShouldSkipDirectlyToGameplay()` | 编辑器里要不要跳过"等待玩家/热身"直接开打 |
| `ShouldLoadCosmeticBackgrounds()` | 编辑器里要不要加载装饰性背景（带 `ExpandBoolAsExecs`，蓝图出双分支） |
| `CanPlayerBotsAttack()` | 训练的 AI 机器人允不允许攻击 |
| `FindPlayInEditorAuthorityWorld()` | 在多个 PIE 世界里找出那个**权威**（服务器）世界 |
| `FindClassByShortName()`（+ 模板版） | ⭐ 按**短名**找类，注释说为了在作弊命令行里好输 |
| `GetAllBlueprints()`（private） | 从 AssetRegistry 拉全部蓝图资产 |
| `FindBlueprintClass()`（private） | 在蓝图里按名字找，且验证是指定基类的子类 |

**优先级**：`FindClassByShortName` → `FindPlayInEditorAuthorityWorld`
