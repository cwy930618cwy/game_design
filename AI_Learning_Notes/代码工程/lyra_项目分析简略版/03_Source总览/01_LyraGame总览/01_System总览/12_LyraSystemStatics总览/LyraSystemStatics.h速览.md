# `LyraSystemStatics.h` 速览

> 给**蓝图**用的静态工具库（`: UBlueprintFunctionLibrary`），主要负责资源 ID 查询、切地图、批量改材质。

| 函数 | 干嘛的 |
|---|---|
| `GetTypedSoftObjectReferenceFromPrimaryAssetId()` | 由 `FPrimaryAssetId` 拿到软引用，**资源没加载也能拿到**；带 `DeterminesOutputType` 自动定型 |
| `GetPrimaryAssetIdFromUserFacingExperienceName()` | 把菜单里的体验名转成 PrimaryAssetId |
| `PlayNextGame()` | ⭐ 服务端跳下一张图（标了 `BlueprintAuthorityOnly`） |
| `SetScalarParameterValueOnAllMeshComponents()` | 给身上所有 Mesh 组件设标量材质参数 |
| `SetVectorParameterValueOnAllMeshComponents()` | 同上，向量参数 |
| `SetColorParameterValueOnAllMeshComponents()` | 同上，颜色参数 |
| `FindComponentsByClass()` | 按类找组件，含子 Actor，带 `DefaultToSelf` |

**优先级**：`PlayNextGame` → `GetTypedSoftObjectReferenceFromPrimaryAssetId`
