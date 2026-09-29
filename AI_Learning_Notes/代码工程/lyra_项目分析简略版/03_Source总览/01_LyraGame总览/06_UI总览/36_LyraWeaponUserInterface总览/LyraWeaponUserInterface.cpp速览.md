# `LyraWeaponUserInterface.cpp` 速览

> 55 行。**这个文件有一半是空的**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `NativeConstruct()` / `NativeDestruct()` | 只调 Super（**没有订阅/退订任何东西**） |
| `NativeTick()` | ⭐ 见下 |
| `RebuildWidgetFromWeapon()` | ⚠️ **函数体完全为空** —— 重建逻辑全交给蓝图的 `OnWeaponChanged` |

## `NativeTick()` 每帧做什么

```
1. 取 OwningPlayerPawn
2. 找 ULyraEquipmentManagerComponent
3. GetFirstInstanceOfType<ULyraWeaponInstance>() 取第一把武器
4. 若和 CurrentInstance 不同 且 它的 Instigator 不为空：
       换存 → RebuildWidgetFromWeapon()（空的）→ OnWeaponChanged(旧, 新)
```

> ⚠️ **两个值得注意的地方**：
> 1. **用 Tick 轮询**而不是事件驱动 —— 每帧都要查一遍装备管理器，性能上不算优雅（但武器数量少，可以接受）。
> 2. `RebuildWidgetFromWeapon()` 是空的，注释和命名都在，说明**这是个留给项目自己填的钩子**，Lyra 自己没实现。
>
> 另外那个 `NewInstance->GetInstigator() != nullptr` 的判断是为了**等武器初始化完成**（Instigator 还没设置时先不切换）。

**优先级**：`NativeTick`
