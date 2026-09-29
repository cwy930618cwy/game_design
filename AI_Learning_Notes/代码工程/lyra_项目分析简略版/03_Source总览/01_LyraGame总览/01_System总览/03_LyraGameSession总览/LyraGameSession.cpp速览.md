# `LyraGameSession.cpp` 速览

> 28 行，只有一个函数有真东西。

| 函数 | 干嘛的 |
|---|---|
| `ProcessAutoLogin()` | 直接 `return true` 跳过引擎自带的自动登录；注释指明真逻辑在 **`LyraGameMode::TryDedicatedServerLogin`** |
| `HandleMatchHasStarted()` | 只调 `Super` |
| `HandleMatchHasEnded()` | 只调 `Super` |

**说明**：专服登录这件事 Lyra 挪到 GameMode 去做了，这里刻意留空。
