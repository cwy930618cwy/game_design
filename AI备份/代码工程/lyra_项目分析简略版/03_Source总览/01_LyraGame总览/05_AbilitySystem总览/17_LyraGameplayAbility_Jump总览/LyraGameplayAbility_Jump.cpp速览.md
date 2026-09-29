# `LyraGameplayAbility_Jump.cpp` 速览

> 72 行，**四个函数，是全部技能里最好懂的一个**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `InstancedPerActor` + `LocalPredicted`（本地预测，跳跃手感才跟手） |
| `CanActivateAbility()` | Avatar 无效 → false；角色 `CanJump()` 为假 → false；再交给 Super |
| `EndAbility()` | ⭐ 先 `CharacterJumpStop()` 再 Super —— 注释说明：**防止蓝图忘了调停止跳跃** |
| `CharacterJumpStart()` | 本地控制且没在跳时：先 `UnCrouch()` 再 `Jump()` |
| `CharacterJumpStop()` | 本地控制且在跳时：`StopJumping()` |

> 💡 **两个都判了 `IsLocallyControlled()`** —— 因为这是 `LocalPredicted` 技能，客户端和服务器都会跑，但只有本地控制的那个才该真正操作角色。

**优先级**：`CanActivateAbility` → `CharacterJumpStart`
