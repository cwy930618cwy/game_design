# `LyraGamePhaseSubsystem.h` 速览

> 阶段的**总控**（`UWorldSubsystem`）。用 GameplayTag 的**嵌套关系**管理哪些阶段能共存。

| 成员 | 干嘛的 |
|---|---|
| `FLyraGamePhaseDynamicDelegate` / `FLyraGamePhaseDelegate` | 阶段回调（动态版给蓝图，普通版给 C++） |
| `FLyraGamePhaseTagDynamicDelegate` / `FLyraGamePhaseTagDelegate` | 按 Tag 的回调 |
| `EPhaseTagMatchType` | ⭐ `ExactMatch`（精确）/ `PartialMatch`（含子孙） |
| `StartPhase()` | ⭐ 启动一个阶段（传技能类 + 结束回调） |
| `WhenPhaseStartsOrIsActive()` / `WhenPhaseEnds()` | ⭐ 注册观察者 |
| `IsPhaseActive()` | 某阶段是否活跃 |
| `K2_StartPhase` / `K2_WhenPhaseStartsOrIsActive` / `K2_WhenPhaseEnds` | 三个蓝图版（`BlueprintAuthorityOnly`） |
| `OnBeginPhase()` / `OnEndPhase()` | 由 `ULyraGamePhaseAbility` 调用 |
| `ActivePhaseMap` | 句柄 → 阶段条目（Tag + 结束回调） |
| `PhaseStartObservers` / `PhaseEndObservers` | 观察者数组 |
| `FLyraGamePhaseEntry` / `FPhaseObserver` | 内部结构体 |

## 类注释里的例子

```
Game.Playing 和 Game.Playing.WarmUp 可以同时活跃（父子）
Game.Playing 和 Game.ShowingScore 不能（兄弟）
开新阶段时，所有"不是祖先"的活跃阶段都会被结束
```

> ⚠️ 源码里两条 TODO：`StartPhase` **没有返回句柄**，观察者无法取消，会一直累积到世界重置 —— 官方自己标注了这是个隐患。

**优先级**：`StartPhase` → `OnBeginPhase` → `EPhaseTagMatchType`
