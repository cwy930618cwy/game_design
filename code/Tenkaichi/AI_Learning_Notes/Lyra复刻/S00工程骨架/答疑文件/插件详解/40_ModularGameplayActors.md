# 40 — `ModularGameplayActors` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **40** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\ModularGameplayActors\`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `True`
> 分类：`Gameplay`
> 官方描述：Base classes designed to be used with the Modular Gameplay plugin.
> 模块：`ModularGameplayActors`(Runtime)

---

## 一、一句话

★ 模块化玩法 Actor 基类：`AModularPawn` / `AModularCharacter` / `AModularPlayerController` 等。

## 二、它到底做什么

15 个文件（7 个 `.h` + 7 个 `.cpp` + 1 个 `Build.cs`）。每个类都在 `BeginPlay` 时调用 `UGameFrameworkComponentManager` 的 `AddReceiver`，让 Actor 参与到「组件初始化状态机」里。

## 三、有它 vs 没它

**没有它 Lyra 一行都编不过** —— `ALyraPawn : public AModularPawn`（`LyraPawn.h:18`）、`ALyraCharacter : public AModularCharacter`。

## 四、Lyra 怎么用它

工程插件（`Plugins\ModularGameplayActors\`）+ `LyraGame.Build.cs` 公共依赖 + `CommonGame` 依赖。`EnabledByDefault: true`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonGame\CommonGame.uplugin` (x1)
- `Plugins\CommonGame\Source\CommonGame.Build.cs` (x1)
- `Plugins\ModularGameplayActors\ModularGameplayActors.uplugin` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S01（插件①）** 建源码 —— 这是最先要落地的 15 个文件。

## 六、注意

★ **S00 阶段编不过的头号原因就是它还没建**：`LyraGame.Build.cs` 公共依赖第 12 项写了 `ModularGameplayActors`，UBT 报 `Could not find definition for module 'ModularGameplayActors'`。到 S01 末才好。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\ModularGameplayActors\ModularGameplayActors.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 40 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
