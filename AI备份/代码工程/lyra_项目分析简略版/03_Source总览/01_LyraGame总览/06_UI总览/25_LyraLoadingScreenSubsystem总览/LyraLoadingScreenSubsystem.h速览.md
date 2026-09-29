# `LyraLoadingScreenSubsystem.h` 速览

> 加载画面的"记忆层"（`GameInstance` 级子系统）。

| 成员 | 干嘛的 |
|---|---|
| `: UGameInstanceSubsystem` | ⭐ **GameInstance 级** —— 跨关卡切换不丢 |
| `FLoadingScreenWidgetChangedDelegate` | 加载界面变了时的广播 |
| `SetLoadingScreenContentWidget()` | 设置当前用哪个加载界面 |
| `GetLoadingScreenContentWidget()` | 读取 |
| `OnLoadingScreenWidgetChanged` | `BlueprintAssignable` 的委托 |
| `LoadingScreenWidgetClass` | 存的控件类 |

> 💡 类注释说明了为什么需要它：**"Tracks/stores the current loading screen configuration in a place that persists across map transitions"** —— 切地图时普通对象会被销毁，只有 GameInstance 级子系统能记住"该显示哪个加载界面"。
>
> 这也是 `LyraUserFacingExperienceDefinition::LoadingScreenWidget` 能被用上的原因。

**优先级**：`SetLoadingScreenContentWidget`
