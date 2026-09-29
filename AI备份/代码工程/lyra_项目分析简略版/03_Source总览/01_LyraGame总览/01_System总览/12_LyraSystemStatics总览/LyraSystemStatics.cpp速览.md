# `LyraSystemStatics.cpp` 速览

> 113 行，全是短函数。

| 函数 | 干嘛的 |
|---|---|
| `GetTypedSoftObjectReferenceFromPrimaryAssetId()` | 先从 AssetManager 查类型信息，校验期望类型；**校验不了就返回 nullptr 而不是猜** |
| `GetPrimaryAssetIdFromUserFacingExperienceName()` | 用体验的类名做 Type，名字做 Name 拼一个 `FPrimaryAssetId` |
| `PlayNextGame()` | ⭐ 取当前 `LastURL` → **编辑器下剥掉 PIE 前缀** → 加 `SeamlessTravel` 选项 → 去掉 host/port → `ServerTravel` |
| 三个 `Set*ParameterValueOnAllMeshComponents()` | 统一用 `ForEachComponent<UMeshComponent>` 遍历（可选包含子 Actor） |
| `FindComponentsByClass()` | 包一层 `GetComponents` 返回数组 |

> 💡 `PlayNextGame` 里有两处典型的引擎坑处理：PIE 的地图前缀会让 ServerTravel 失败（所以 `StripPIEPrefixFromPackageName`），URL 里带 host/port 会让 ServerTravel 失败（所以 `RemoveFromStart`）。

**优先级**：`PlayNextGame`
