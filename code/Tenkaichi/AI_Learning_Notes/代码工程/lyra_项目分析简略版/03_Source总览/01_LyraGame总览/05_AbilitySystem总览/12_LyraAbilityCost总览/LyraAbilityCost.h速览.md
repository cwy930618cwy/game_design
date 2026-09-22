# `LyraAbilityCost.h` 速览

> 技能消耗的**基类**。只有 `.h`，没有 `.cpp` —— 两个虚函数都是内联的默认实现。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS(DefaultToInstanced, EditInlineNew, Abstract)` | ⭐ 这三个标记让它能**直接内联编辑在技能的配置面板里** |
| `CheckCost()` | 虚拟，默认返回 `true`（付得起）。失败时往 `OptionalRelevantTags` 里加原因 Tag |
| `ApplyCost()` | 虚拟，默认什么都不做 |
| `ShouldOnlyApplyCostOnHit()` | 这个消耗是不是"命中才扣" |
| `bOnlyApplyCostOnHit` | 上面的开关字段 |

## 三个子类

| 子类 | 消耗什么 |
|---|---|
| `ULyraAbilityCost_PlayerTagStack` | 消耗 PlayerState 上的 Tag 堆栈（如法力） |
| `ULyraAbilityCost_ItemTagStack` | 消耗物品实例上的 Tag 堆栈（如弹药） |
| `ULyraAbilityCost_InventoryItem` | 消耗背包里的某个物品 |

> 💡 **注释里说明了 `OptionalRelevantTags` 的用途**：失败时加一个 Tag，别处（比如 UI）就能查询它来决定怎么给玩家反馈 —— 例如没弹药时播放"咔哒"声。

**优先级**：看三个子类
