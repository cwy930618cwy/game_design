# `LyraIndicatorManagerComponent.cpp` 速览

> 43 行，四个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | `bAutoRegister = true` + `bAutoActivate = true` —— **不用手动加就能用** |
| `GetComponent()` | 从 Controller 上 `FindComponentByClass`，Controller 为空返回 nullptr |
| `AddIndicator()` | 先 `SetIndicatorManagerComponent(this)`（**反向绑定**）→ 广播 → 加入数组 |
| `RemoveIndicator()` | `ensure` 这个标记确实归自己管 → 广播 → 移除 |

> 💡 **注意顺序**：`AddIndicator` 里是**先广播后加入数组**，而 `RemoveIndicator` 是**先广播后移除** —— 这样回调里拿到的状态是一致的（都是"变化尚未生效"）。

**优先级**：`AddIndicator`
