# `LyraTabListWidgetBase.cpp` 速览

> 209 行。

| 函数 | 干嘛的 |
|---|---|
| `NativeConstruct()` | 调 `SetupTabs()` |
| `NativeDestruct()` | 把所有已创建的内容 `RemoveFromParent` |
| `GetPreregisteredTabInfo()` | 按 TabId 线性查找 |
| `SetTabHiddenState()` | 改 `bHidden` 字段 |
| `RegisterDynamicTab()` | ⭐ 隐藏的直接返回 true（**当作成功**）；否则记进 PendingMap 再 `RegisterTab` |
| `HandlePreLinkedSwitcherChanged()` | 换 Switcher 前，先把旧内容摘掉 |
| `HandlePostLinkedSwitcherChanged()` | ⭐ 换完后**重新 `SetupTabs()`**（跳过设计器和未构造的情况） |
| `HandleTabCreation_Implementation()` | ⭐ 见下 |
| `IsFirstTabActive()` / `IsLastTabActive()` | 比对首尾 TabId |
| `IsTabVisible()` | 看按钮可见性是不是三种"可见"之一 |
| `GetVisibleTabCount()` | 遍历计数 |
| `SetupTabs()` | ⭐ 见下 |

## `HandleTabCreation_Implementation()` 的坑

```
先查预注册表，没有再查 PendingMap
如果按钮实现了 ILyraTabButtonInterface：
    ensureMsgf(TabInfoPtr, "...RegisterDynamicTab should be used over RegisterTab to provide label info.")
    → 直接用 RegisterTab 而不用 RegisterDynamicTab 会断言失败
```

## `SetupTabs()` 的三步

```
1. 跳过 bHidden 的
2. 内容还没创建就 CreateWidget，并广播 OnTabContentCreated（两个版本）
3. 有 Switcher 就 AddChild（先查 HasChild 防重复）
4. 没注册过就 RegisterTab
```

> 💡 **为什么要自己管创建？** 因为引擎原生的 TabList 会立刻创建所有内容，Lyra 想做到"用到了再建 + 能拿到创建回调去初始化内容"。

**优先级**：`SetupTabs` → `HandleTabCreation_Implementation`
