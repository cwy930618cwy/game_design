# `GameFeatureAction_AddGameplayCuePath.h` 速览

> 声明"这个插件的 GameplayCue 放在哪些目录"。⚠️ **它只声明数据，不干活**。

| 成员 | 干嘛的 |
|---|---|
| `: UGameFeatureAction`（**不是** WorldActionBase） | 因为它跟具体世界无关，只是改 CueManager 的目录列表 |
| `DirectoryPathsToAdd` | `TArray<FDirectoryPath>`，`RelativeToGameContentDir` + `LongPackageName` |
| `GetDirectoryPathsToAdd()` | 给外部（观察者）读取用 |
| `IsDataValid()`（编辑器） | 路径为空就报错 |

**说明**：注意它**没有** `OnGameFeatureActivating` / `Deactivating` 的实现 —— 真正添加目录的是 `LyraGameFeaturePolicy.cpp` 里的 `ULyraGameFeature_AddGameplayCuePaths` 观察者，在插件 **Registering** 阶段做的。
