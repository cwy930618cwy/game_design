# `LyraGameSession.h` 速览

> 会话层：**负责处理"开赛/结束"和禁用默认的自动登录**。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(Config = Game)` | 能从 `DefaultGame.ini` 读配置 |
| `: AGameSession`（是 **A**ctor） | 注意它是 Actor 不是 UObject，存在于关卡里 |
| `ProcessAutoLogin()` | 注释写着 "Override to disable the default behavior" |
| `HandleMatchHasStarted()` | 比赛开始回调 |
| `HandleMatchHasEnded()` | 比赛结束回调 |

**优先级**：`ProcessAutoLogin`
