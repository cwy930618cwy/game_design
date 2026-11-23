# 49 — `ModelingToolsEditorMode` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **49** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Editor`
> 官方描述：Modeling Tools Mode includes a suite of interactive tools for creating and editing meshes in the Editor
> 模块：`ModelingToolsEditorMode`(Editor)

---

## 一、一句话

建模工具编辑器模式 —— 在编辑器里直接建/改网格体。

## 二、它到底做什么

模块：`ModelingToolsEditorMode`（Editor）。提供一套交互式建模工具（挤出、倒角、布尔、UV 编辑）。

## 三、有它 vs 没它

不启用 → 编辑器里没有「建模模式」。**运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非在编辑器里做原型网格）。

## 六、注意

纯美术工具。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Editor\ModelingToolsEditorMode\ModelingToolsEditorMode.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 49 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
