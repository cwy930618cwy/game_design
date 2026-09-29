# `LyraAbilityTagRelationshipMapping.h` 速览

> 一张纯配置表：**技能 Tag 之间谁阻断谁、谁取消谁**。配在 `PawnData` 上，不同角色可以有不同规则。

## `FLyraAbilityTagRelationship`（一条规则）

| 字段 | 干嘛的 |
|---|---|
| `AbilityTag` | 这条规则针对哪个 Tag（`Categories = "Gameplay.Action"`） |
| `AbilityTagsToBlock` | 带这个 Tag 的技能激活时，**要阻断**哪些技能 |
| `AbilityTagsToCancel` | 带这个 Tag 的技能激活时，**要取消**哪些技能 |
| `ActivationRequiredTags` | 隐式追加到"激活必需 Tag" |
| `ActivationBlockedTags` | 隐式追加到"激活禁止 Tag" |

## `ULyraAbilityTagRelationshipMapping`

| 成员 | 干嘛的 |
|---|---|
| `: UDataAsset` | 普通数据资产（不是 PrimaryDataAsset） |
| `AbilityTagRelationships` | 规则数组 |
| `GetAbilityTagsToBlockAndCancel()` | 给定技能 Tag，算出要 Block/Cancel 哪些 |
| `GetRequiredAndBlockedActivationTags()` | 算出额外的必需/禁止 Tag |
| `IsAbilityCancelledByTag()` | 判断某动作 Tag 会不会取消掉这些技能 |

> 💡 **这张表解决了 GAS 的一个痛点**：原生里"技能 A 启动时取消技能 B"要写在代码或技能自己的 Tag 里，散落各处。Lyra 把它**集中成一张表**，改起来非常直观。
