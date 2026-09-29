# `LyraTaggedActor.h` 速览

> 一个**带静态 GameplayTag 的简易 Actor**。给场景里"需要被识别但不需要 ASC"的物体用。

| 成员 | 干嘛的 |
|---|---|
| `: AActor` + `IGameplayTagAssetInterface` | 实现了 Tag 资产接口，所以能被伤害系统等按 Tag 识别 |
| `GetOwnedGameplayTags()` | 返回身上的静态 Tag 集合 |
| `CanEditChange()`（仅编辑器） | ⭐ **禁用原生的 `AActor::Tags` 属性**，逼大家用下面这个专门的字段 |
| `StaticGameplayTags` | `EditAnywhere, BlueprintReadOnly` 的 Tag 容器 |

**说明**：它的作用是让关卡里放的物体能参与"按 Tag 判定"的逻辑（比如某些伤害只对带特定 Tag 的物体生效），而不用为此挂一整套 GAS。
