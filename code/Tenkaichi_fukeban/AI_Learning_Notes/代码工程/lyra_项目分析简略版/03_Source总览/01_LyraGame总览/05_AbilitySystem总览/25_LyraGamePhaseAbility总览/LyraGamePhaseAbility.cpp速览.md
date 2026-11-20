# `LyraGamePhaseAbility.cpp` 速览

> 65 行，三个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `ReplicateNo` + `InstancedPerActor` + **`ServerInitiated` + `ServerOnly`**（阶段只在服务器推进） |
| `ActivateAbility()` | **仅权威端** → 通知 PhaseSubsystem `OnBeginPhase(this, Handle)` |
| `EndAbility()` | **仅权威端** → 通知 `OnEndPhase(this, Handle)` |
| `IsDataValid()`（编辑器） | `GamePhaseTag` 没设就报 "GamePhaseTag must be set..." |

> 💡 **为什么阶段要做成技能？** 这样"阶段"就能复用 GAS 的全部能力：
> - 用 `GamePhaseTag` 配合 `ActivationRequiredTags` 控制**哪些技能只能在哪个阶段用**
> - 阶段自己也能播蒙太奇、等事件、配消耗
> - 生命周期由 GAS 管理，不用自己写状态机
>
> 典型用法：给"只有战斗中才能开枪"的技能配一个 `GamePhase.Playing` 的必需 Tag。

**优先级**：`ActivateAbility` → `IsDataValid`
