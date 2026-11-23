# 01 — `ActorPalette` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **1** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Experimental`
> 官方描述：Allows creation of Actor Palettes based on existing levels to quickly select actors and drag them into the level editor
> 模块：`ActorPalette`(Editor)

---

## 一、一句话

给关卡美术用的「Actor 素材面板」——从已有关卡挑 Actor 直接拖进当前关卡。

## 二、它到底做什么

模块类型 `Editor`，只在编辑器加载。在「工具」菜单下注册 4 个标签页（`ActorPaletteTab1~4`），每页内嵌一个 `FPreviewScene` 显示某个关卡；点住视口里的 Actor 时，插件伪造一次 `FAssetDragDropOp` 拖放事件丢给 Slate，主编辑器关卡视口按原生拖放流程把 Actor 摆下去。还带「最近关卡 / 收藏」列表（`UActorPaletteSettings`，存 `EditorPerProjectUserSettings`）。

## 三、有它 vs 没它

**运行时零影响、打包零体积**（Editor 模块不参与打包）。只有编辑器里多一个菜单分组和 4 个标签页。

## 四、Lyra 怎么用它

只在 `.uproject` 里启用，Lyra 代码里 0 处引用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

实验性插件（`Category=Experimental`、`IsBetaVersion=true`）且 `EnabledByDefault=false` —— 靠 `.uproject` 那行才有；误删该项不报错，只是菜单消失。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\ActorPalette.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 1 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
