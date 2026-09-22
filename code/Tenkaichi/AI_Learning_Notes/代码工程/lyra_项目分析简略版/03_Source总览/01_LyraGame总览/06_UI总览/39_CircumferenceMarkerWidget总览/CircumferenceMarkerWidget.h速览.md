# `CircumferenceMarkerWidget.h` 速览

> **准星的环形刻度**（散布圈四周那几个小角标）。UMG 包装 + 手写 Slate 实现。

| 成员 | 干嘛的 |
|---|---|
| `: UWidget` | 直接继承 UWidget |
| `SynchronizeProperties()` | 改了属性后同步给 Slate 控件 |
| `RebuildWidget()` / `ReleaseSlateResources()` | 建/销毁 |
| `MarkerList` | ⭐ `TArray<FCircumferenceMarkerEntry>` —— 每个角标的角度和旋转 |
| `Radius` | 圆的半径（默认 **48**） |
| `MarkerImage` | 角标图片 |
| `bReticleCornerOutsideSpreadRadius` | 角标放在散布圈内侧还是外侧（源码留了 TODO 说该改成 0~1 的对齐值） |
| `SetRadius()` | ⭐ 运行时改半径（准星张开/收拢就靠它） |
| `MyMarkerWidget` | 内部 Slate 控件 |

**优先级**：`SetRadius` → `RebuildWidget`
