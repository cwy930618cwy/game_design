# 64 — `FunctionalTestingEditor` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **64** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Testing`
> 官方描述：NONE
> 模块：`FunctionalTestingEditor`(Editor)

---

## 一、一句话

功能测试编辑器 —— 在编辑器里录制/回放玩家操作做功能测试。

## 二、它到底做什么

模块：`FunctionalTestingEditor`（Editor）。配合 `FunctionalTesting` 运行时模块（引擎内置）使用。

## 三、有它 vs 没它

不启用 → 编辑器里没有「功能测试」菜单。**运行时零影响。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非写自动化 UI 测试）。

## 六、注意

与 5 号 Gauntlet 不同：这个录 UI 操作，Gauntlet 启动整个游戏进程。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Tests\FunctionalTestingEditor\FunctionalTestingEditor.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 64 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
