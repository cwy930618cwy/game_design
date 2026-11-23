# 15 — `SignificanceManager` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **15** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Performance`
> 官方描述：The significance manager plugin provides an extensible framework for allowing games to calculate the significance of an object and change behavior in response.
> 模块：`SignificanceManager`(Runtime)

---

## 一、一句话

重要性管理器 —— 给每个 Actor 算「重要性分数」，远处的降频更新。

## 二、它到底做什么

提供 `USignificanceManager` / `FSignificanceFunction`。每帧（或每 N 帧）调用你注册的打分函数，返回重要性值；业务代码（动画/AI/特效）按分数决定更新频率。

## 三、有它 vs 没它

不启用 → 所有 Actor 同频率更新，性能没优化空间。**功能上零影响，只是性能差。**

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs`（公共）+ `S21` 的 `LyraSignificanceManager.h/.cpp`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S21**。

## 六、注意

Lyra 的用法：按「距离 + 视野 + 是否队友」算分，分数低的不更新动画 Tick。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\SignificanceManager\SignificanceManager.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 15 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
