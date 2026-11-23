# 59 — `AssetSearch` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **59** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Editor`
> 官方描述：NONE
> 模块：`AssetSearch`(EditorNoCommandlet)

---

## 一、一句话

资产搜索 —— 编辑器里的高级资产搜索面板。

## 二、它到底做什么

模块：`AssetSearch`（EditorNoCommandlet）。提供跨内容目录的复杂条件搜索（按类、按属性、按引用关系）。

## 三、有它 vs 没它

不启用 → 编辑器里没有该搜索面板。**运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

替代「内容浏览器」里那个基础搜索。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Editor\AssetSearch\AssetSearch.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 59 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
