# `LyraTaggedWidget.h` 速览

> 按 GameplayTag 自动显隐的控件（比如"只有狙击开镜时才显示"的准星）。
> ⚠️ **注意：这个功能目前没实现完。**

| 成员 | 干嘛的 |
|---|---|
| `: UCommonUserWidget`（`Abstract, Blueprintable`） | 标准控件继承 |
| `SetVisibility()` | 重写，"想显示"和"实际显示"分开记 |
| `NativeConstruct()` / `NativeDestruct()` | 构造/析构（**监听 Tag 变化的代码是空的**） |
| `HiddenByTags` | ⭐ 玩家身上有这些 Tag 时，本控件隐藏 |
| `ShownVisibility` / `HiddenVisibility` | 显示/隐藏时分别用什么可见性（默认 `Visible` / `Collapsed`） |
| `bWantsToBeVisible` | ⭐ "忽略 Tag 的话，我本来想不想显示" |
| `OnWatchedTagsChanged()`（private） | Tag 变化时的回调 |

> 💡 **`bWantsToBeVisible` 的设计是对的**：即使当前被 Tag 压着隐藏了，也要记住"调用方本来想让我显示"，等 Tag 解除后能正确恢复。

**优先级**：`SetVisibility`（但功能未完）
