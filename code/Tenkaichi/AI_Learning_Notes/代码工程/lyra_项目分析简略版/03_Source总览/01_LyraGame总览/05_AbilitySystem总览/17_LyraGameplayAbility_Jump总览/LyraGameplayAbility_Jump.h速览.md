# `LyraGameplayAbility_Jump.h` 速览

> **最简单的技能范例** —— 想学"Lyra 的技能该怎么写"，看这个就对了。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraGameplayAbility`（`UCLASS(Abstract)`） | 继承 Lyra 技能基类，本身还是抽象类（蓝图再继承一次） |
| `CanActivateAbility()` | 额外检查角色 `CanJump()` |
| `EndAbility()` | 兜底调一次 `CharacterJumpStop()` |
| `CharacterJumpStart()` / `CharacterJumpStop()` | `BlueprintCallable`，给蓝图调用的开始/停止跳跃 |

**说明**：它把"跳"这个动作包成技能后，就能吃到 GAS 的全部好处 —— 能被 Tag 阻断（比如被眩晕时跳不了）、能参与激活组、能配消耗。
