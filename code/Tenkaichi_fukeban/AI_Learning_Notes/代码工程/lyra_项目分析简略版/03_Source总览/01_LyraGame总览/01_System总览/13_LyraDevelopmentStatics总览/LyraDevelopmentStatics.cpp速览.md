# `LyraDevelopmentStatics.cpp` 速览

> 201 行，**一半以上在折腾"按名字找类"这件事**。

| 函数 | 干嘛的 |
|---|---|
| `ShouldSkipDirectlyToGameplay()` | 编辑器下返回 `!bTestFullGameFlowInPIE`，其余情况一律 false |
| `ShouldLoadCosmeticBackgrounds()` | 编辑器下返回 `!bSkipLoadingCosmeticBackgroundsInPIE`，其余一律 true |
| `CanPlayerBotsAttack()` | 编辑器下读 `bAllowPlayerBotsToAttack`，其余一律 true |
| `FindPlayInEditorAuthorityWorld()` | ⭐ 遍历所有 PIE 世界；**优先挑 `RunAsDedicated` 的那个**，没有就拿 NetMode 更小的那个 |
| `GetAllBlueprints()` | AssetRegistry 按 `UBlueprint` 类路径递归过滤 |
| `FindBlueprintClass()` | 先去掉名字末尾的 `_C`，再按资产名或对象路径匹配，最后校验是不是目标子类 |
| `FindClassByShortName()` | ⭐ 三级查找：先 `TryFindTypeSlow` 找原生/已加载类 → 找不到再用 AssetRegistry 找蓝图 → 找到后校验基类，不符就记警告并返回 nullptr |

> 💡 注意前三个函数**在打包出来之后行为是固定的**（false / true / true），它们纯粹是编辑器便利工具。

**优先级**：`FindClassByShortName` → `FindPlayInEditorAuthorityWorld`
