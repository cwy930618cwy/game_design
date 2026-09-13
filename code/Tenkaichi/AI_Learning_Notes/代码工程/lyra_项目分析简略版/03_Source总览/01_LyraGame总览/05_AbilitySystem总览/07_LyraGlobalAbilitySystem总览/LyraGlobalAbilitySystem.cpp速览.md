# `LyraGlobalAbilitySystem.cpp` 速览

> 157 行，**两个结构体的实现几乎完全对称**。

| 函数 | 干嘛的 |
|---|---|
| `FGlobalAppliedAbilityList::AddToASC()` | 已有就先移除；`GiveAbility` 后记账 |
| `FGlobalAppliedAbilityList::RemoveFromASC()` | `ClearAbility` 后删账 |
| `FGlobalAppliedAbilityList::RemoveFromAll()` | 遍历所有 ASC 清除，然后清空 |
| `FGlobalAppliedEffectList::*` | 同上三个，换成 `ApplyGameplayEffectToSelf` / `RemoveActiveGameplayEffect` |
| `ApplyAbilityToAll()` | 判空 + 查重（已存在就不重复加）→ 建条目 → 遍历已注册 ASC 施加 |
| `RemoveAbilityFromAll()` | 反向 |
| `RegisterASC()` | ⭐ 遍历现有的所有全局技能/GE，**逐个补给新 ASC**，最后加入列表 |
| `UnregisterASC()` | 从所有条目里移除这个 ASC，再从列表删掉 |

## 注册时机（配合 ASC 那边看）

```
ULyraAbilitySystemComponent::InitAbilityActorInfo()
    └─ 等到真有 Pawn Avatar 之后才 RegisterASC(this)
       （源码注释：因为某些全局 GE 需要 Avatar 才能生效）

ULyraAbilitySystemComponent::EndPlay()
    └─ UnregisterASC(this)
```

> 💡 **为什么必须等有 Pawn 之后？** 因为有些全局 GE（比如需要绑定角色网格的）没有 Avatar 就施加上会出问题。这是 Lyra 特意推迟注册的原因。

**优先级**：`RegisterASC` → `ApplyAbilityToAll`
