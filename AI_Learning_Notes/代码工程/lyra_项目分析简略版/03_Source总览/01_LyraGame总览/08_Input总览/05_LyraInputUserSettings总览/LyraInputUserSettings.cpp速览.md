# `LyraInputUserSettings.cpp` 速览

> 12 行，**只有一个函数，而且只调了 Super**。

| 函数 | 干嘛的 |
|---|---|
| `ApplySettings()` | `Super::ApplySettings();` + 两行注释 |

> 💡 那两行注释值得一看：
> ```
> // Add any functionality you want to happen when the input settings are applied to the user
> // This is a good place to put a breakpoint in your debugger to see the flow of
> // how input settings are used :)
> ```
>
> 也就是说 **Lyra 自己没往里加任何设置项**，这个类是纯粹的扩展点。官方还贴心地告诉你：想搞清楚输入设置的应用流程，在这里打个断点最方便。

**说明**：`ULyraPlayerMappableKeySettings` 的 `GetTooltipText()` 因为带 `UE_API` 宏，实现应该也在别处或内联 —— 这个 cpp 里没有。
