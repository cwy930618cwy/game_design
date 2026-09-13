# `LyraGameEngine.cpp` 速览

> 20 行，**两个函数都只调 `Super`**。

| 函数 | 干嘛的 |
|---|---|
| `ULyraGameEngine()` | 只调用 `Super(ObjectInitializer)` |
| `Init()` | 只调用 `Super::Init(InEngineLoop)` |

**说明**：当前没有任何 Lyra 自己的逻辑，是纯占位实现。
