# `LyraInputComponent.cpp` 速览

> 41 行，**几乎全是空壳**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `AddInputMappings()` | ⚠️ 只 `check` 两个参数，注释写着 "Here you can handle any custom logic..." —— **什么都没做** |
| `RemoveInputMappings()` | ⚠️ 同样只有 check 和注释 |
| `RemoveBinds()` | ⭐ 唯一有实际逻辑的：遍历句柄 `RemoveBindingByHandle`，然后 `Reset()` |

> ⚠️ **为什么 Add/RemoveInputMappings 是空的？** 因为 Lyra 的 InputMappingContext 是由 `ULyraHeroComponent` 通过 `UEnhancedInputLocalPlayerSubsystem` 直接加的，不走这个组件。这两个函数是**留给项目自己扩展的钩子**。
>
> 实际有用的只有 `RemoveBinds` 和头文件里的两个模板函数。

**优先级**：`RemoveBinds`
