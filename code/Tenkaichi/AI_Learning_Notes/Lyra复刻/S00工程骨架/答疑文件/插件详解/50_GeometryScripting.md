# 50 — `GeometryScripting` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **50** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Geometry`
> 官方描述：Geometry Script provides a library of functions for creating and editing Meshes in Blueprints and Python
> 模块：`GeometryScriptingCore`(Runtime), `GeometryScriptingEditor`(Editor)

---

## 一、一句话

几何脚本 —— 用蓝图/Python 程序化生成与编辑网格。

## 二、它到底做什么

2 个模块：`GeometryScriptingCore`（Runtime）+ `GeometryScriptingEditor`（Editor）。提供几百个蓝图节点（布尔、简化、UV 展开、LOD 生成）。

## 三、有它 vs 没它

不启用 → 蓝图里没有 `GeometryScript` 节点。游戏运行时若没用它则零影响。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

「程序化内容生成（PCG）」场景很有用；Lyra 只是启用未深用。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GeometryScripting\GeometryScripting.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 50 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
