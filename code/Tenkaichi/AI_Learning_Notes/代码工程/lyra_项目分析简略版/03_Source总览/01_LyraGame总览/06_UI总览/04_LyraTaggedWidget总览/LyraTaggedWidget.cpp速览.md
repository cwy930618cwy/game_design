# `LyraTaggedWidget.cpp` 速览

> 82 行。**这个文件最有意思的地方是它"缺了什么"**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `NativeConstruct()` | 注释写着 `//@TODO: Listen for tag changes on our hidden tags`，然后只调了一次 `SetVisibility(GetVisibility())` |
| `NativeDestruct()` | 注释 `//@TODO: Stop listening for tag changes`，**函数体实际什么都不做** |
| `SetVisibility()` | ⭐ 逻辑写好了：记录 `bWantsToBeVisible`、记住 Shown/Hidden 的可见性、算出最终值 |
| `OnWatchedTagsChanged()` | 同上逻辑，但**永远不会被调用**（没人调它） |

## ⚠️ 关键的缺失

```cpp
const bool bHasHiddenTags = false;  //@TODO: Foo->HasAnyTags(HiddenByTags);
```

> 这行在 `SetVisibility` 和 `OnWatchedTagsChanged` 里**各出现一次**，都是硬编码 `false`。
> 也就是说：**`HiddenByTags` 目前完全不起作用**，而且没有任何地方去监听 Tag 变化。
>
> 文件开头还有一句总注释：`//@TODO: The other TODOs in this file are all related to tag-based showing/hiding of widgets, see UE-142237`（关联的引擎 issue 号）。

**结论**：想用"按 Tag 显隐"，得自己补上这段，或者用别的方式实现。
