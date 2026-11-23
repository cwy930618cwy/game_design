# 68 — `GameplayBehaviorSmartObjects` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **68** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Other`
> 官方描述：Plugins for SmartObjects using GameplayBehavior as their default runtime behavior
> 模块：`GameplayBehaviorSmartObjectsModule`(Runtime)

---

## 一、一句话

玩法行为 + 智能对象桥接 —— 让 SmartObject 默认走 GameplayBehavior。

## 二、它到底做什么

模块：`GameplayBehaviorSmartObjectsModule`（Runtime）。把 69 号的 `UGameplayBehavior` 接进 66 号的 SmartObject 体系。

## 三、有它 vs 没它

不启用 → SmartObject 的默认行为不能是「GameplayBehavior」。

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

65 / 66 / 68 / 69 / 70 是「AI 环境交互」五件套，Lyra 全都启用了但基本没用。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\GameplayBehaviorSmartObjects\GameplayBehaviorSmartObjects.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 68 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
