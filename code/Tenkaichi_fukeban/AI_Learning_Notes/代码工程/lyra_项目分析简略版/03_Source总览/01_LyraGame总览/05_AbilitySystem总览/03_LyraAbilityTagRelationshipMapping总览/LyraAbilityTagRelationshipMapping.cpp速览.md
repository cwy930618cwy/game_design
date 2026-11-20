# `LyraAbilityTagRelationshipMapping.cpp` 速览

> 64 行，三个函数，**实现都是同一个套路**。

| 函数 | 干嘛的 |
|---|---|
| `GetAbilityTagsToBlockAndCancel()` | 遍历规则，命中就把 `AbilityTagsToBlock` / `AbilityTagsToCancel` 追加到输出 |
| `GetRequiredAndBlockedActivationTags()` | 同上，输出 `ActivationRequiredTags` / `ActivationBlockedTags` |
| `IsAbilityCancelledByTag()` | 找 `AbilityTag == ActionTag` 的规则，看它的 `AbilityTagsToCancel` 是否命中 |

> ⚠️ 三个函数开头都有同一句注释：`// Simple iteration for now` —— 官方明确说**当前是 O(n) 暴力遍历**，规则多了会有性能问题，是个已知的待优化点。

**说明**：没有复杂的查找结构，就是线性扫描数组。
