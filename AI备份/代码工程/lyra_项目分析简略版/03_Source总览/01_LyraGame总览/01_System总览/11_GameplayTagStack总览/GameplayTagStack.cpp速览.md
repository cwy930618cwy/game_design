# `GameplayTagStack.cpp` 速览

> 110 行，全集中在**怎么维持 Stacks 和 TagToCountMap 两份数据一致**。

| 函数 | 干嘛的 |
|---|---|
| `GetDebugString()` | 拼成 `%sx%d` |
| `AddStack()` | 标签无效 → Kismet 警告返回；找得到就累加并 `MarkItemDirty`，找不到就 Emplace 并 `MarkItemDirty` |
| `RemoveStack()` | 数量够减就减并标脏；**不够就直接整条移除**并 `MarkArrayDirty` |
| `PreReplicatedRemove()` | 客户端：被删的项从 `TagToCountMap` 里也删掉 |
| `PostReplicatedAdd()` | 客户端：新增项写进 Map |
| `PostReplicatedChange()` | 客户端：变了就覆盖 Map 里的值 |

> 💡 **`AddStack` / `RemoveStack` 里凡是改动都配了 `MarkItemDirty` / `MarkArrayDirty`** —— 少了这两个调用，网络端就收不到变化。

**优先级**：`AddStack` → `RemoveStack`
