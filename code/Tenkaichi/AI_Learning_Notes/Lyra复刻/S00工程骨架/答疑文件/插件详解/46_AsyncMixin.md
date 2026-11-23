# 46 — `AsyncMixin` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **46** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\AsyncMixin\`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`UI`
> 官方描述：C++ utility class for managing asynchronous operations like loading.
> 模块：`AsyncMixin`(Runtime)

---

## 一、一句话

异步操作管理工具：`FAsyncMixin` —— 让 UObject 拥有「生命周期安全的异步链」。

## 二、它到底做什么

3 个模块文件（1 个 `.h` + 2 个 `.cpp` + 1 个 `Build.cs`）。核心 `FAsyncMixin`：`StartAsyncTask()` 拿到 `FAsyncTask`，支持 `Then()` 链式、取消、UObject 销毁时自动取消。

## 三、有它 vs 没它

不启用 → 异步代码只能用裸回调/委托，容易在对象销毁后崩。`LyraGame.Build.cs` 公共依赖它，**缺了编不过**。

## 四、Lyra 怎么用它

工程插件（`Plugins\AsyncMixin\`）+ `LyraGame.Build.cs` 公共依赖 + ShooterCore。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\AsyncMixin\AsyncMixin.uplugin` (x2)
- `Plugins\AsyncMixin\Source\Private\AsyncMixin.cpp` (x2)
- `Plugins\GameFeatures\ShooterCore\ShooterCore.uplugin` (x1)
- `Plugins\GameFeatures\ShooterCore\Source\ShooterCoreRuntime\ShooterCoreRuntime.Build.cs` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S01（插件①）**。

## 六、注意

它是 Lyra 里「加载资产 → 初始化 → 回调」这类流程的基石（`ULyraAssetManager` 的 `StartupJob` 就靠它）。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\AsyncMixin\AsyncMixin.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 46 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
