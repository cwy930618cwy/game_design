# 39 — `CommonLoadingScreen` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **39** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\CommonLoadingScreen\`）
> 当前状态：`"Enabled": true`
> 分类：`Gameplay`
> 官方描述：Loading screen manager handling creation and display of a project-specified loading screen widget
> 模块：`CommonLoadingScreen`(Runtime), `CommonStartupLoadingScreen`(ClientOnly)

---

## 一、一句话

★ 加载屏管理器 —— 切图/加载时显示进度界面。

## 二、它到底做什么

2 个模块：`CommonLoadingScreen`（Runtime，提供 `ULoadingScreenManager` 与 `ILoadingProcessInterface`）、`CommonStartupLoadingScreen`（ClientOnly，启动期的预加载屏，`FCommonPreLoadScreen`）。

## 三、有它 vs 没它

不启用 → 加载时黑屏，没有任何进度提示。`LyraGame.Build.cs` 公共依赖它，**缺了编不过**。

## 四、Lyra 怎么用它

工程插件（`LyraStarterGame\Plugins\CommonLoadingScreen\`）+ `LyraGame.Build.cs` 公共依赖 + `DefaultGame.ini` 的 `[/Script/CommonLoadingScreen.CommonLoadingScreenSettings]`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonLoadingScreen\CommonLoadingScreen.uplugin` (x2)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S02（插件②）** 建源码；S08 的 `ULyraExperienceManagerComponent` 实现 `ILoadingProcessInterface` 与它对接。

## 六、注意

源码已在 `Tenkaichi\Plugins\CommonLoadingScreen\`（你之前拷过来的）；S02 会逐个文件讲 `LoadingScreenManager.cpp` 等。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\CommonLoadingScreen\CommonLoadingScreen.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 39 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
