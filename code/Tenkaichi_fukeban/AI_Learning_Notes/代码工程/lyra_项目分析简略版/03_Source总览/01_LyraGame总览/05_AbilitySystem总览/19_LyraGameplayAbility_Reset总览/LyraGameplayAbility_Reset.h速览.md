# `LyraGameplayAbility_Reset.h` 速览

> 把玩家快速重置回出生状态。类注释：由 `GameplayEvent.RequestReset` 触发，**仅服务器**。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraGameplayAbility` | 标准继承 |
| `ActivateAbility()` | 唯一的 override |
| `FLyraPlayerResetMessage` | 重置广播出去的消息体，只带一个 `OwnerPlayerState` |

**说明**：一个工具类技能，通常用于调试或某些玩法的"回到起点"。
