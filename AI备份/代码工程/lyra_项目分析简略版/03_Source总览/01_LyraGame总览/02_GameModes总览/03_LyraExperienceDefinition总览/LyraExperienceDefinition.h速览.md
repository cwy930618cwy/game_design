# `LyraExperienceDefinition.h` 速览

> **只有 4 个数据字段的纯配置**。一局游戏长什么样，全靠它描述。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset` | 能被 AssetManager 按 PrimaryAssetId 管理（联网协同的基础） |
| `BlueprintType, Const` | 蓝图可建、**标记为只读**（设计意图：它是配置不是运行时对象） |
| `IsDataValid()`（仅编辑器） | 数据校验，见 cpp |
| `UpdateAssetBundleData()`（仅编辑器数据） | 让每个 Action 往 AssetBundle 里追加自己的资源需求 |
| `GameFeaturesToEnable` | ⭐ 本 Experience 需要启用哪些 GameFeature 插件（`TArray<FString>`，填插件名） |
| `DefaultPawnData` | ⭐ 玩家默认用哪套角色数据（含输入、技能、外观） |
| `Actions` | ⭐ 要执行的 `UGameFeatureAction` 列表，`Instanced` 所以能在编辑器里直接配置 |
| `ActionSets` | ⭐ 引用其它的 `ULyraExperienceActionSet`，**用于组合复用** |

> 💡 **`Actions` + `ActionSets` 的双层结构**：`Actions` 是这份 Experience 独有的，`ActionSets` 是可跨 Experience 复用的包。想给多个模式加同一套东西，就抽成 ActionSet。

**优先级**：`GameFeaturesToEnable` → `Actions` / `ActionSets`
