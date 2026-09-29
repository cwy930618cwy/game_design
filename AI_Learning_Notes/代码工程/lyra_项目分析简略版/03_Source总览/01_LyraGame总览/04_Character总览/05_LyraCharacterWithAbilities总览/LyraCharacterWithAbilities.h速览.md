# `LyraCharacterWithAbilities.h` 速览

> **自带 ASC 的角色**。源文件顶部注释直接说明了它的用途：
> "ALyraCharacter 通常从占有的 PlayerState 上取 ASC；这个类表示**自带完整 ASC** 的角色。"

| 成员 | 干嘛的 |
|---|---|
| `: ALyraCharacter`（`Blueprintable`） | 继承角色全部能力 |
| `PostInitializeComponents()` | 把 ASC 的 Owner 和 Avatar 都设成自己 |
| `GetAbilitySystemComponent()` | 返回**自己的** ASC（覆盖了父类转发给 PawnExtComponent 的行为） |
| `AbilitySystemComponent`（private） | 自建的 `ULyraAbilitySystemComponent` |
| `HealthSet` / `CombatSet`（private） | 自建的两个属性集 |

## 什么时候用它

| 场景 | 用哪个 |
|---|---|
| 玩家角色（ASC 要跨死亡、跨 Pawn 保留） | `ALyraCharacter`（ASC 在 **PlayerState** 上） |
| AI 生物 / 不需要 PlayerState 的对象 | `ALyraCharacterWithAbilities`（ASC **在自己身上**） |

**说明**：ASC 放在 PlayerState 上是为了**角色死后重生时技能和数据不丢**，这是 Lyra 的重要设计。
