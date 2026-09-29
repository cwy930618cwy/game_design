# `GameplayTagStack.h` 速览

> Lyra 自造的关键工具：**让 GameplayTag 能带数量**（弹药 30、护盾 50 都靠它）。两个结构体，都是可复制的。

## `FGameplayTagStack`：单个「标签 + 数量」

| 成员 | 干嘛的 |
|---|---|
| `: FFastArraySerializerItem` | 说明它是**网络增量复制**数组里的一项 |
| `Tag` | 哪个标签（private） |
| `StackCount` | 有几个（private，默认 0） |
| `GetDebugString()` | 输出成 `Tagx30` 这种形式 |

## `FGameplayTagStackContainer`：一堆「标签 + 数量」

| 成员 | 干嘛的 |
|---|---|
| `: FFastArraySerializer` | 走 FastArray 增量复制，只传变化的那几项 |
| `AddStack()` / `RemoveStack()` | 增减数量，<=0 就移除 |
| `GetStackCount()` / `ContainsTag()` | 查询（走下面的加速表，O(1)） |
| `PreReplicatedRemove` / `PostReplicatedAdd` / `PostReplicatedChange` | FFastArraySerializer 回调，**客户端靠这三个同步加速表** |
| `NetDeltaSerialize()` | 绑定 `FastArrayDeltaSerialize` |
| `Stacks` | ⭐ 真正参与复制的数组 |
| `TagToCountMap` | ⭐ 本地加速用的 Map，不参与复制 |
| `TStructOpsTypeTraits` | 打开 `WithNetDeltaSerializer` |

> 💡 **这套设计的核心**：只复制 `Stacks`，`TagToCountMap` 是本地缓存，通过三个 Post/Pre 回调保持两边同步。查询走 Map，复制走数组。

**优先级**：`AddStack` → `RemoveStack` → 三个 `Post/Pre` 回调
