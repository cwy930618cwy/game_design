# `LyraActorUtilities.cpp` 速览

> 41 行，只有一个函数。

| 步骤 | 干嘛的 |
|---|---|
| 默认 `NM_Standalone` | 找不到就用单机作为兜底 |
| 沿 Outer 链向上找 | 找到 `UActorComponent` 或 `AActor` 就取其 `GetNetMode()` 并 break |
| `switch (NetMode)` | 映射成 `EBlueprintExposedNetMode` 四个值 |
| default | `ensure(false)` 后返回 `Standalone` |

> ⚠️ **一处可疑**：`for` 循环拿到的是 `TestObject`，但取值时用的还是 `WorldContextObject`（循环变量没被使用）。目前传 Component/Actor 都能正常工作，只是那层向上遍历实际没生效。

**优先级**：`SwitchOnNetMode`
