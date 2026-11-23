# 47 — `GameplayMessageRouter` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **47** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\GameplayMessageRouter\`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `True`
> 分类：`Gameplay`
> 官方描述：A subsystem that allows registering for and sending messages between unconnected gameplay objects.
> 模块：`GameplayMessageRuntime`(Runtime), `GameplayMessageNodes`(UncookedOnly)

---

## 一、一句话

★ 玩法消息路由 —— 不相连的对象之间发消息。

## 二、它到底做什么

2 个模块：`GameplayMessageRuntime`（`UGameplayMessageSubsystem`：按 GameplayTag 广播/监听）、`GameplayMessageNodes`（UncookedOnly，蓝图异步节点 `UAsyncAction_ListenForGameplayMessage`）。

## 三、有它 vs 没它

不启用 → 蓝图里没有「监听消息」节点；Lyra 的击杀播报、任务进度、连杀提示全不工作。

## 四、Lyra 怎么用它

工程插件（`Plugins\GameplayMessageRouter\`）+ ShooterCore 依赖 + `S06` 的 `LyraVerbMessageHelpers` / `LyraVerbMessageReplication`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)

## 五、本复刻线什么时候碰它

**S01（插件①）**。

## 六、注意

它是「解耦」的利器：开枪的代码不需要知道谁在听，只管 `BroadcastMessage(TAG, Data)`。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\GameplayMessageRouter\GameplayMessageRouter.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 47 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
