# `LyraHealExecution.cpp` 速览

> 57 行，**结构几乎和 `LyraDamageExecution` 一模一样，但简单得多**。

| 步骤 | 干嘛的 |
|---|---|
| `FHealStatics` | 缓存 `BaseHeal` 的捕获定义（同样从 Source 侧） |
| 构造函数 | 登记捕获 |
| `Execute_Implementation()` | 同样包在 `#if WITH_SERVER_CODE` 里 |

## 公式

```
HealingDone = Max(BaseHeal, 0)     ← 只做"不能为负"这一步
```

然后加到 `ULyraHealthSet::GetHealingAttribute()` 上。

> 💡 **和伤害的关键区别**：治疗**没有**距离衰减、没有材质衰减、没有队伍判定。只有一处 `FMath::Max(0.0f, ...)` 防止负值。
>
> 想加"治疗衰减"或"不能治疗敌人"的话，就得照伤害那边的写法补上。

**优先级**：`Execute_Implementation`
