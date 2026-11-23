# 66 — `SmartObjects` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **66** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Gameplay`
> 官方描述：Support for ambient life populating the game world
> 模块：`SmartObjectsModule`(Runtime), `SmartObjectsTestSuite`(UncookedOnly), `SmartObjectsEditorModule`(Editor)

---

## 一、一句话

智能对象 —— 世界里标记「可交互点」（可坐的椅子、可用的梯子）。

## 二、它到底做什么

2 个模块：`SmartObjectsModule`（Runtime）+ `SmartObjectsTestSuite`（UncookedOnly）。提供 `USmartObjectComponent`（标记）、`USmartObjectSubsystem`（查询/占用）、`USmartObjectDefinition`（行为定义数据资产）。

## 三、有它 vs 没它

不启用 → 关卡里不能标 SmartObject；AI 找不到「可用的位置」。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

UE5 的「环境交互 + AI 行为」体系核心；Lyra 只是启用。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\SmartObjects\SmartObjects.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 66 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
