# `SActorCanvas.cpp` 速览

> 726 行，**全项目最长的单个文件**。这是手写 Slate 的重头戏。

| 函数 | 干嘛的 |
|---|---|
| `SActorCanvasArrowWidget`（内部类） | 箭头控件，`SLeafWidget`，自己 `OnPaint` 画旋转的方块 |
| 两个方向常量数组 | `ArrowRotations`（四个方向的角度）、`ArrowOffsets`（四个方向的单位偏移） |
| `Construct()` | 存上下文、**预创建 10 个箭头**、关 Tick、启动 ActiveTimer |
| `UpdateCanvas()` | ⭐ 见下 |
| `OnArrangeChildren()` | ⭐ 见下 |
| `OnPaint()` | 缓存 `AllottedGeometry`（供 UpdateCanvas 用），再绘制 |
| `AddIndicatorForEntry()` | ⭐ 见下 |
| `RemoveIndicatorForEntry()` | `UnbindIndicator` → 回收到池 → 移除槽位 |
| `GetOffsetAndSize()` | 按 H/V 对齐算偏移和内边距 |
| `UpdateActiveTimer()` | 有标记 或 还没找到 Manager 才注册定时器 |

## `UpdateCanvas()` 每帧做什么

```
1. 没有缓存的几何体 → 跳过
2. 找 IndicatorManagerComponent（找不到就 TODO: HIDE EVERYTHING）
3. 拿 LocalPlayer 的投影数据，拿不到就隐藏全部
4. 逐个标记：
   - CanAutomaticallyRemove() → 移除（并修正索引）
   - 不可见 → 跳过
   - 投影 → 更新 位置/深度/优先级（通过 FSlot 的脏标记）
5. 有变化才 Invalidate(Paint)
6. 没有标记了就 Stop 定时器
```

## `OnArrangeChildren()` 的排序与贴边

```
排序规则：Priority 相同时按 Depth 从近到远，否则按 Priority 从小到大

贴边（当 GetClampToScreen() 为真）：
   超出矩形 → 用四个平面做线段-平面求交，找出从哪条边出去
   在相机背后 → 按归一化坐标判断贴到哪条边
   算出方向 → 设置箭头角度和偏移（用预创建的 10 个箭头之一）
```

## `AddIndicatorForEntry()` 的异步加载

```
AsyncLoad(IndicatorClass, [回调]{
    ★ 加载期间可能已被移除 → 检查 AllIndicators.Contains
    从 IndicatorPool.GetOrCreateInstance 取控件（池化复用）
    实现了 IActorIndicatorWidget 就 Execute_BindIndicator
    包进 SBox 加到槽位
})
```

> ⚠️ **那段"加载完成了但已经被移除"的检查很重要** —— 异步加载期间标记可能已经被删除，不检查就会残留一个孤儿控件。

**优先级**：`UpdateCanvas` → `OnArrangeChildren` → `AddIndicatorForEntry`
