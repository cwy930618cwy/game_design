# `LyraGameViewportClient.cpp` 速览

> 31 行，两个函数，一个静态 Tag。

| 函数 | 干嘛的 |
|---|---|
| `TAG_Platform_Trait_Input_HardwareCursor` | `Platform.Trait.Input.HardwareCursor` |
| 构造函数 | 空 |
| `Init()` | ⭐ 见下 |

## `Init()` 做了什么

```
查询平台 Trait 里有没有 HardwareCursor 这个 Tag
    → 有（PC）：用硬件光标 → SetUseSoftwareCursorWidgets(false)
    → 没有（主机/移动端）：用软件光标 → SetUseSoftwareCursorWidgets(true)
```

> 💡 源码注释解释得很清楚：**项目设置里为主机/移动端配了软件光标，但 PC 上用系统硬件光标就够了**，所以在这里按平台切一下。
>
> 软件光标是为了让手柄也能"移动鼠标指针"（手柄没有物理鼠标），这是主机 UI 的标配。
