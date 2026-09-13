# `LyraExperienceActionSet.h` 速览

> 一个**可复用的 Action 包**，被多个 Experience 引用就能共享同一套行为。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset` | 和 ExperienceDefinition 一样走 AssetManager |
| `BlueprintType, NotBlueprintable` | ⭐ **允许被蓝图引用，但不允许被蓝图继承** |
| `IsDataValid()`（编辑器） | 校验 Actions 有没有空项 |
| `UpdateAssetBundleData()`（编辑器） | 收集每个 Action 的额外资源需求 |
| `Actions` | `EditAnywhere, Instanced` —— 注意是 **EditAnywhere**（比 Experience 里的 EditDefaultsOnly 宽松） |
| `GameFeaturesToEnable` | 这个包自己依赖的 GameFeature 插件列表 |

## 与 `LyraExperienceDefinition` 的字段对比

| 字段 | ExperienceDefinition | ActionSet |
|---|---|---|
| `Actions` | ✅ | ✅ |
| `GameFeaturesToEnable` | ✅ | ✅ |
| `DefaultPawnData` | ✅ | ❌ |
| `ActionSets` | ✅ | ❌ |

**说明**：两者高度相似，区别只在 ActionSet 不能嵌套自己、也不定义角色。
