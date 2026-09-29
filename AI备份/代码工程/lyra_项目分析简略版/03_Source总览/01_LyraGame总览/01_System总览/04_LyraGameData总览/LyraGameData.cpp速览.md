# `LyraGameData.cpp` 速览

> 16 行，整个文件只有一个转发。

| 函数 | 干嘛的 |
|---|---|
| `ULyraGameData()` | 空构造 |
| `Get()` | `return ULyraAssetManager::Get().GetGameData();` |

**说明**：本身不持有单例，真正的持有者是 `ULyraAssetManager`。这里只是给调用方一行就能拿到数据。
