# `LyraGameplayAbility_Death.h` 速览

> 把"死亡"做成一个技能。类注释写明：**它由 `GameplayEvent.Death` 这个事件自动触发**。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraGameplayAbility`（`Abstract`） | 标准继承 |
| `ActivateAbility()` / `EndAbility()` | 激活/结束 |
| `StartDeath()` / `FinishDeath()` | ⭐ `BlueprintCallable`，给蓝图在动画合适时机调用 |
| `bAutoStartDeath` | 勾上就自动 `StartDeath`（默认 true） |

> 💡 **为什么死亡要做成技能？** 因为这样死亡就能：
> - 走 GAS 的激活/结束流程
> - 用激活组（`Exclusive_Blocking`）**挡住其它所有技能**
> - 播放蒙太奇、等待动画通知
> - 用 Tag 精确控制谁能豁免（见 cpp）

**优先级**：`ActivateAbility` → `StartDeath`
