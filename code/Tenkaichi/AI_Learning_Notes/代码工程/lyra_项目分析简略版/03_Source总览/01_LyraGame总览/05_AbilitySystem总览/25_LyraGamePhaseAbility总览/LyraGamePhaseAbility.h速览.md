# `LyraGamePhaseAbility.h` 速览

> ⭐ **把"游戏阶段"做成一个技能**。这是 Lyra 里很妙的一个设计。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraGameplayAbility`（`Abstract`, `HideCategories = Input`） | 继承自技能，所以天然能：被 Tag 控制、播放蒙太奇、等待事件 |
| `GamePhaseTag` | ⭐ 这个技能代表哪个阶段（如 `GamePhase.Playing.SuddenDeath`） |
| `GetGamePhaseTag()` | 取值 |
| `IsDataValid()`（编辑器） | ⭐ 校验：`GamePhaseTag` 必须设置 |
| `ActivateAbility()` / `EndAbility()` | 通知 `ULyraGamePhaseSubsystem` 阶段开始/结束 |

## `GamePhaseTag` 的嵌套规则（源码注释讲得很清楚）

```
Game.Playing 和 Game.Playing.WarmUp 可以共存（父子）
Game.Playing 和 Game.ShowingScore 不能共存（兄弟）
从 Game.Playing.CaptureTheFlag 切到 Game.Playing.PostGame：
    → Game.Playing 保留（是祖先）
    → CaptureTheFlag 结束（不是祖先）
```

> 💡 用 `MatchesTag` 判断祖先关系 —— **一个 Tag 匹配它的所有子孙**，这是这套嵌套机制的技术基础。

**优先级**：`GamePhaseTag` → `ActivateAbility`
