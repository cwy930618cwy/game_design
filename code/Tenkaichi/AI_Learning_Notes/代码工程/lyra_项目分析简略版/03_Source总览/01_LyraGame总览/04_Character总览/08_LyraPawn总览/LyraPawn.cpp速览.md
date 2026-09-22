# `LyraPawn.cpp` 速览

> 113 行，**几乎全在讲一件事：队伍 ID 从哪来、什么时候能改**。

| 函数 | 干嘛的 |
|---|---|
| `GetLifetimeReplicatedProps()` | 只复制 `MyTeamID` |
| `PossessedBy()` | 记下旧队伍 → Super → 从 Controller 上取队伍 ID → 订阅 Controller 的换队事件 → 广播 |
| `UnPossessed()` | 取消订阅 → Super → `DetermineNewTeamAfterPossessionEnds()` 决定新队伍 → 广播 |
| `SetGenericTeamId()` | ⚠️ 有两条拒绝路径，见下 |
| `GetGenericTeamId()` / `GetOnTeamIndexChangedDelegate()` | 简单取值 |
| `OnControllerChangedTeam()` | 跟上 Controller 的新队伍并广播 |
| `OnRep_MyTeamID()` | 客户端收到时 `ConditionalBroadcastTeamChanged` |

## `SetGenericTeamId()` 什么时候会失败

```
情况 1：Pawn 已被 Controller 占有
    → Error："it's driven by the associated controller"
情况 2：没有 Controller，但本机不是权威
    → Error："You can't set the team ID ... except on the authority"
```

> 💡 **设计意图很清楚**：只要角色被某个 Controller 控制着，队伍就**由 Controller 说了算**，任何人不能从 Pawn 这边改。这保证了"队伍只有一个真相来源"。

> ⚠️ 注意：`LyraCharacter.cpp` 里有一份**几乎一模一样**的实现（连 Error 文案都只差 pawn/character 一个词）。这是一处明显的重复代码。

**优先级**：`PossessedBy` / `UnPossessed` → `SetGenericTeamId`
