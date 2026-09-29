# `LyraInputComponent.h` 速览

> 输入组件。**它最值钱的是两个模板函数**，都在头文件里内联实现。

| 成员 | 干嘛的 |
|---|---|
| `: UEnhancedInputComponent`（`Config = Input`） | 标准继承 |
| `AddInputMappings()` / `RemoveInputMappings()` | ⚠️ **都是空壳**，只 check 参数 |
| `BindNativeAction<...>()` | ⭐ 模板：按 Tag 找一个动作，绑到指定函数 |
| `BindAbilityActions<...>()` | ⭐ 模板：批量把技能动作绑到"按下/松开"两个回调 |
| `RemoveBinds()` | 按句柄数组解绑并清空 |

## `BindAbilityActions` 做了什么

```
遍历 InputConfig->AbilityInputActions：
    Triggered 事件 → 绑到 PressedFunc，并把 InputTag 作为额外参数传进去
    Completed 事件 → 绑到 ReleasedFunc
    每个绑定产生的句柄都记进 BindHandles（供以后 RemoveBinds）
```

> 💡 **关键在于把 `Action.InputTag` 塞进了 BindAction 的额外参数** —— 所以回调里能直接拿到"是哪个 Tag 触发的"，进而转发给 ASC 的 `AbilityInputTagPressed(Tag)`。
>
> 这条链是这样的：
> ```
> 按键 → UInputAction → BindAction 回调（带 InputTag）
>      → ULyraAbilitySystemComponent::AbilityInputTagPressed(Tag)
>      → 按 Tag 找到对应技能 Spec → 激活
> ```

**优先级**：`BindAbilityActions` → `BindNativeAction`
