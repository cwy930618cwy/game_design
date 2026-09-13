# `LyraUIManagerSubsystem.cpp` 速览

> 69 行，四个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `Initialize()` | `FTSTicker::GetCoreTicker().AddTicker(...)` |
| `Deinitialize()` | 移除 Ticker |
| `Tick()` | 调 `SyncRootLayoutVisibilityToShowHUD()` 并返回 `true`（继续） |
| `SyncRootLayoutVisibilityToShowHUD()` | ⭐ 见下 |

## 同步逻辑

```
对每个本地玩家：
    默认 bShouldShowUI = true
    取它的 PlayerController → HUD
    如果 HUD 存在且 bShowHUD == false → bShouldShowUI = false

    取 RootLayout（PrimaryGameLayout）
    目标可见性 = bShouldShowUI ? SelfHitTestInvisible : Collapsed
    只在不一样时才 SetVisibility
```

> 💡 **这就是 `showhud` 控制台命令能在 Lyra 里生效的原因**：引擎的 `AHUD::bShowHUD` 只管 AHUD 自己的 Canvas，管不到用 CommonUI 搭的 UMG 界面。Lyra 加了这一层，把 UMG 根布局的可见性跟 `bShowHUD` 绑起来。

**优先级**：`SyncRootLayoutVisibilityToShowHUD`
