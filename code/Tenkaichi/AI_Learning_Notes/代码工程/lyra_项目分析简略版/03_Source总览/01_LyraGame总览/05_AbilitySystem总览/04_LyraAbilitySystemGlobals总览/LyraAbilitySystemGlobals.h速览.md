# `LyraAbilitySystemGlobals.h` 速览

> **21 行，全目录最小的头文件**。只干一件事。

| 成员 | 干嘛的 |
|---|---|
| `: UAbilitySystemGlobals`（`Config = Game`） | 全局单例配置类 |
| `AllocGameplayEffectContext()` | 唯一 override：返回 Lyra 自己的 `FLyraGameplayEffectContext` |

**说明**：GAS 内部每次创建 GE 上下文都会调这个函数。覆写它，就等于**让整个项目的 GE 都换上 Lyra 的自定义上下文**（从而能携带伤害来源、弹夹 ID 等额外数据）。
