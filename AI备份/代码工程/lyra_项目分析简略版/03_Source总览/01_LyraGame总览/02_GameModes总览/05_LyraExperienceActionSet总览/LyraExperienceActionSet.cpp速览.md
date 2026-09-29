# `LyraExperienceActionSet.cpp` 速览

> 61 行，内容和 `LyraExperienceDefinition.cpp` 几乎一样，**少了继承检查**那一段。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `IsDataValid()`（编辑器） | 遍历 Actions，空的就报 "Null entry at index N"；每个 Action 递归调 `IsDataValid` 再合并结果 |
| `UpdateAssetBundleData()`（编辑器） | 遍历 Actions 调 `AddAdditionalAssetBundleData(AssetBundleData)` |

**说明**：没有像 ExperienceDefinition 那样检查"蓝图的蓝图"，因为它本来就是 `NotBlueprintable`。
