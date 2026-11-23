# 48 — `AssetReferenceRestrictions` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **48** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Editor`
> 官方描述：Apply project-specific restrictions to how content in different folders or plugins can be referenced
> 模块：`AssetReferenceRestrictions`(Editor)

---

## 一、一句话

资产引用限制 —— 按文件夹/插件规定「谁能引用谁」。

## 二、它到底做什么

模块：`AssetReferenceRestrictions`（Editor）。配置规则后，编辑器和资产校验会拦截「不该引用的引用」（比如 UI 模块引用玩法模块的资产）。

## 三、有它 vs 没它

不启用 → 没有引用约束，跨模块乱引用不会被发现。**运行时零影响、打包零体积。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管。

## 六、注意

属于「工程规范」类工具，大型团队才配规则；个人开发开着也无害。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Editor\AssetReferenceRestrictions\AssetReferenceRestrictions.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 48 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
