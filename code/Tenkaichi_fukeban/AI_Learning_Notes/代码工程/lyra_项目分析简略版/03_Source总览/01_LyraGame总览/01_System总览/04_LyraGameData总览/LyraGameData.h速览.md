# `LyraGameData.h` 速览

> 一个只读数据资产，**全局通用的 GameplayEffect 就配在这里**（资源/drive 的那个 DataAsset）。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset` | 不是 UDataAsset，所以能被 AssetManager 按 PrimaryAssetId 管理 |
| `Meta = (Const)` / `BlueprintType` | 标记只读，蓝图可读 |
| `Get()` | 静态单例访问，内部转发给 AssetManager |
| `DamageGameplayEffect_SetByCaller` | 通用伤害 GE，伤害值用 **SetByCaller** 传入 |
| `HealGameplayEffect_SetByCaller` | 通用治疗 GE，同上 |
| `DynamicTagGameplayEffect` | 用来动态增删 GameplayTag 的 GE |

**说明**：只有三个字段，但它们是**所有伤害/治疗都绕不开的公共配置**。
