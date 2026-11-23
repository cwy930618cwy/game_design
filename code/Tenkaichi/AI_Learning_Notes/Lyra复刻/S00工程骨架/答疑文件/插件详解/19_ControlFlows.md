# 19 — `ControlFlows` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **19** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`
> 分类：`Other`
> 官方描述：Tool to cleanly implement Asynchronous Operations
> 模块：`ControlFlows`(Runtime)

---

## 一、一句话

用节点图/代码干净地写异步操作（协程风格）。

## 二、它到底做什么

提供 `UControlFlow` / `UControlFlowTask` / `FControlFlow` 等。用节点串起「先做 A，再并行做 B 和 C，最后做 D」，失败能中断整条链。

## 三、有它 vs 没它

不启用 → 写异步只能嵌套回调（Callback Hell）。**功能上零影响，只是代码难写。**

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

S00 `Build.cs` 已依赖（不用改）；业务代码里用得少。

## 六、注意

Lyra 里主要用来串「加载资产 → 初始化 → 通知完成」这类流程。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ControlFlows\ControlFlows.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 19 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
