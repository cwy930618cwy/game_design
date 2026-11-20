# `LyraPawn.h` 速览

> 通用 Pawn 基类（给载具等非人形对象用）。**它的内容和 `LyraCharacter` 高度重复** —— 主要是队伍归属那套。

| 成员 | 干嘛的 |
|---|---|
| `: AModularPawn` + `ILyraTeamAgentInterface` | 组件化 Pawn + 队伍接口 |
| `PreInitializeComponents()` / `EndPlay()` | 生命周期（目前只调 Super） |
| `PossessedBy()` / `UnPossessed()` | ⭐ 被占有/失去占有时同步队伍 ID |
| `SetGenericTeamId()` / `GetGenericTeamId()` / `GetOnTeamIndexChangedDelegate()` | 队伍接口三件套 |
| `DetermineNewTeamAfterPossessionEnds()`（虚函数） | 失去占有后队伍怎么算，默认返回 `NoTeam` |
| `OnControllerChangedTeam()` | Controller 换队时跟上 |
| `MyTeamID` | `ReplicatedUsing = OnRep_MyTeamID` |
| `OnTeamChangedDelegate` | 队伍变化的广播 |
| `OnRep_MyTeamID()` | 客户端收到新队伍时广播 |

> 💡 **`DetermineNewTeamAfterPossessionEnds` 是留给子类改的钩子**。源码注释举例：可以改成保留原队伍，或者归到某个中立阵营。

**优先级**：`PossessedBy` / `UnPossessed` → `SetGenericTeamId`
