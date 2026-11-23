# 11 — `WinDualShock` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **11** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Input Devices`
> 官方描述：InputDevice plugin for the PS4 DualShock controller in Windows
> 模块：`WinDualShock`(RuntimeNoCommandlet)

---

## 一、一句话

Windows 上的 PS4/PS5 手柄（DualShock / DualSense）支持。

## 二、它到底做什么

模块类型 `RuntimeNoCommandlet`（运行时可用，命令行工具里不加载）。把 PS 手柄的按键/摇杆/触摸板映射到 UE 的标准输入键。

## 三、有它 vs 没它

不启用 → 插 PS 手柄时只能被识别成通用 XInput 设备（部分特性如触摸板、自适应扳机拿不到）。**键鼠开发完全无感。**

## 四、Lyra 怎么用它

只在 `.uproject` 启用（带 `SupportedTargetPlatforms:[Win64]`）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)

## 五、本复刻线什么时候碰它

永远不用管（除非要做 PS 手柄适配）。

## 六、注意

Xbox 手柄是引擎内置 XInput 支持，不需要插件。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\Windows\WinDualShock\WinDualShock.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 11 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
