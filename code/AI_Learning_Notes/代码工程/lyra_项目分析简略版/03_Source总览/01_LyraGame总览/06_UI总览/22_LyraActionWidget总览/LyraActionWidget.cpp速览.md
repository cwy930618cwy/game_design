# `LyraActionWidget.cpp` 速览

> 42 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| `GetIcon()` | ⭐ 见下 |
| `GetEnhancedInputSubsystem()` | 优先用"绑定控件"的 LocalPlayer，没有就用自己 Owning 的 |

## `GetIcon()` 的逻辑

```
如果有 AssociatedInputAction：
    查 EnhancedInput 里这个 Action 绑了哪些键
    取第一个，用 CommonInput 的 TryGetInputBrush 转成笔刷
    （带上当前输入类型和手柄型号）
成功就返回，否则退回 Super::GetIcon()（用默认数据表里的图标）
```

> 💡 **注释直说了这个类的存在意义**：
> "This covers the case of **when a player has rebound a key to something else**"
> —— 原生 `UCommonActionWidget` 只从数据表读默认图标，玩家改了键它不知道。Lyra 加这一层，改成**实时从 EnhancedInput 查实际绑定**。
>
> 这是"改键功能"能在 UI 上正确显示的技术前提，和 `LyraButtonBase::RefreshButtonText` 是同一套思路（一个是文字，一个是图标）。

**优先级**：`GetIcon`
