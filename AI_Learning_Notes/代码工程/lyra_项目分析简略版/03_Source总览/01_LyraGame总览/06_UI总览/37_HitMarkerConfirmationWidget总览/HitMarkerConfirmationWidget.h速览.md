# `HitMarkerConfirmationWidget.h` 速览

> **命中确认标记**（打中敌人时准星上跳出的 X 形）。UMG 包装，内部是手写的 `SHitMarkerConfirmationWidget`。

| 成员 | 干嘛的 |
|---|---|
| `: UWidget` | 直接继承 UWidget（不是 UserWidget） |
| `RebuildWidget()` / `ReleaseSlateResources()` | 建/销毁 Slate 控件 |
| `HitNotifyDuration` | 显示多久后淡出（默认 **0.4 秒**，单位强制为秒） |
| `PerHitMarkerImage` | 每次命中的标记图 |
| `PerHitMarkerZoneOverrideImages` | ⭐ `TMap<FGameplayTag, FSlateBrush>` —— **按命中部位换图**（爆头用不同图标） |
| `AnyHitsMarkerImage` | "只要有命中"就显示的图（画在准星中心） |
| `MyMarkerWidget` | 内部 Slate 控件 |

**优先级**：`RebuildWidget`
