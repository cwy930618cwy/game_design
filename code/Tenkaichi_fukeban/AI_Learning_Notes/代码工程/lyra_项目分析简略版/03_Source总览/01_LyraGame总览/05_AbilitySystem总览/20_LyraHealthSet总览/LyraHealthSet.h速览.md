# `LyraHealthSet.h` 速览

> 血量属性集。**它不直接显示"血"的变化，而是用两个"元属性" `Damage` / `Healing` 做中转**。

| 成员 | 干嘛的 |
|---|---|
| `: ULyraAttributeSet`（`BlueprintType`） | 继承 Lyra 属性集基类 |
| `Health` / `MaxHealth` | ⭐ 真正复制的属性（`REPNOTIFY_Always`） |
| `Healing` / `Damage` | ⭐ **元属性（Meta Attribute）**：不复制，只作为中转，用完清零 |
| `OnHealthChanged` / `OnMaxHealthChanged` / `OnOutOfHealth` | 三个 `FLyraAttributeEvent` 广播 |
| `OnRep_Health()` / `OnRep_MaxHealth()` | 客户端的复制回调 |
| `PreGameplayEffectExecute()` / `PostGameplayEffectExecute()` | ⭐ GE 应用前后的钩子 |
| `PreAttributeBaseChange()` / `PreAttributeChange()` / `PostAttributeChange()` | 属性变化钩子 |
| `ClampAttribute()` | 钳制：血量 [0, MaxHealth]，最大血量 **至少 1** |
| `bOutOfHealth` + 两个 `...BeforeAttributeChange` | 记录"之前是多少"，用于算变化量和判定跨过 0 |

## 5 个全局 Tag

```
Gameplay.Damage                 ← 伤害
Gameplay.DamageImmunity         ← 免疫（但自毁伤害无视它）
Gameplay.Damage.SelfDestruct    ← 自毁
Gameplay.Damage.FellOutOfWorld  ← 掉出世界
Lyra.Damage.Message             ← 伤害消息（广播给 UI/播报）
```

> 💡 **为什么要用 `Damage` 元属性中转？** 因为这样"护甲减伤""暴击倍率""无敌"都能在 `PreGameplayEffectExecute` 里统一改 `Magnitude`，而不用动真正的 Health。这是 GAS 社区的标准最佳实践，Lyra 完整实现了它。

**优先级**：`PreGameplayEffectExecute` → `PostGameplayEffectExecute` → `ClampAttribute`
