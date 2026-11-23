# 44 — `GameSubtitles` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **44** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\GameSubtitles\`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`UI`
> 官方描述：Game support for playing subtitles attached to media player assets.
> 模块：`GameSubtitles`(Runtime)

---

## 一、一句话

字幕显示系统：把 MediaPlayer 资产上的字幕渲染到屏幕。

## 二、它到底做什么

11 个文件。核心：`USubtitleDisplaySubsystem`（字幕子系统）、`USubtitleDisplayOptions`（样式配置）、`UMediaSubtitlesPlayer`（从 MediaPlayer 取字幕）、`USubtitleDisplay` / `SSubtitleDisplay`（Slate 控件）。

## 三、有它 vs 没它

不启用 → 播视频时字幕不显示。`LyraGame.Build.cs` 私有依赖它，**缺了编不过**。

## 四、Lyra 怎么用它

工程插件（`Plugins\GameSubtitles\`）+ `LyraGame.Build.cs` 私有依赖 + ShooterCore 依赖。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Plugins\GameSubtitles\GameSubtitles.uplugin` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S04（插件④）**。

## 六、注意

依赖 `MediaAssets` 等媒体模块；不播视频的话功能上零影响。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\GameSubtitles\GameSubtitles.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 44 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
