# `SActorCanvas.h` 速览

> ⭐ **手写 Slate 的标记画布**，整个系统最复杂的部分（255 行头文件）。

| 成员 | 干嘛的 |
|---|---|
| `: SPanel` + `FAsyncMixin` + `FGCObject` | 面板 + 异步加载能力 + **参与 GC 引用** |
| `FSlot`（内部类） | ⭐ 一个标记槽位，带一堆 `bDirty` 标记用于**只更新变化的部分** |
| `FArrowSlot` | 箭头槽位 |
| `Construct()` | 初始化，**预创建 10 个箭头** |
| `OnArrangeChildren()` | ⭐ 核心：排序 + 计算位置 + 贴边 + 放箭头 |
| `OnPaint()` | 缓存几何体并绘制 |
| `UpdateCanvas()` | ⭐ 每帧（ActiveTimer）更新投影 |
| `OnIndicatorAdded()` / `OnIndicatorRemoved()` | 响应 Manager 的事件 |
| `AddIndicatorForEntry()` | ⭐ 异步加载控件类并创建（用 `IndicatorPool` 池化） |
| `GetOffsetAndSize()` | 按对齐方式算偏移 |
| `UpdateActiveTimer()` | ⭐ **没有标记就停掉定时器**（省性能） |
| `AddReferencedObjects()` | GC 引用 `AllIndicators` |
| `bDrawElementsInOrder` | 按顺序绘制（**会禁用批处理，drawcall 变多**） |

## `FSlot` 的脏标记设计

```
ScreenPosition / Depth / Priority / 可见性 等每个都有 setter
值变了才置 bDirty = true
→ UpdateCanvas 里只在真的变了时才 Invalidate(Paint)
```

> 💡 **这套"脏标记"设计是为了性能**：屏幕标记可能有很多个，每帧全量重排重绘太贵，只更新真正变化的部分。

**优先级**：`UpdateCanvas` → `OnArrangeChildren` → `FSlot`
