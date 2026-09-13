# `GameFeatureAction_AddInputBinding.cpp` 速览

> 176 行。核心是**等 HeroComponent 说"可以绑了"才绑**。

| 函数 | 干嘛的 |
|---|---|
| `OnGameFeatureActivating()` | 取/建 ContextData，不干净就 Reset，再 Super |
| `IsDataValid()`（编辑器） | 逐个检查 `InputConfigs` 有没有空项 |
| `AddToWorld()` | 对 `APawn::StaticClass()` 注册 `HandlePawnExtension` |
| `HandlePawnExtension()` | `ExtensionRemoved`/`ReceiverRemoved` → 撤；`ExtensionAdded` 或 **`ULyraHeroComponent::NAME_BindInputsNow`** → 加 |
| `AddInputMappingForPlayer()` | ⭐ 见下 |
| `RemoveInputMapping()` | 逐个 `HeroComponent->RemoveAdditionalInputConfig` |

## `AddInputMappingForPlayer()` 的门槛

```
1. 拿 Pawn 的 Controller → LocalPlayer
2. 取 UEnhancedInputLocalPlayerSubsystem（取不到就 Error 并放弃）
3. ★ HeroComponent 必须存在 且 IsReadyToBindInputs() 为真
4. 才逐个调 HeroComponent->AddAdditionalInputConfig(配置)
```

> 💡 **`NAME_BindInputsNow` 这个事件是 HeroComponent 广播的**，含义是"输入已经可以绑了"。所以输入绑定和技能授予一样，都**不是**一看到 Actor 就做，而是等一个 InitState 就绪信号。
>
> 这也解释了为什么这两件事要分两个 Action 做：`AddAbilities` 等 `NAME_LyraAbilityReady`，这个等 `NAME_BindInputsNow`。

**优先级**：`AddInputMappingForPlayer`
