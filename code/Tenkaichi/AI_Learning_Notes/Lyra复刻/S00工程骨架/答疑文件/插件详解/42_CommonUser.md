# 42 — `CommonUser` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **42** 项
> 来源：工程插件（`E:\ue5\game_design\code\Tenkaichi\Plugins\CommonUser\`）
> 当前状态：`"Enabled": true`
> 分类：`Gameplay`
> 官方描述：Provides gameplay code and blueprint wrappers for online and platform operations.
> 模块：`CommonUser`(Runtime)

---

## 一、一句话

★ 通用用户系统 —— 登录 / 会话 / 在线服务封装的统一层。

## 二、它到底做什么

13 个文件。核心：`UCommonUserSubsystem`（用户登录、玩家编号、平台用户）、`UCommonSessionSubsystem`（会话创建/加入）、`UCommonUserBasicPresence`（在线状态）、`UAsyncAction_CommonUserInitialize`（异步初始化）。

## 三、有它 vs 没它

不启用 → `CommonGame` 编不过（它依赖 `CommonUser`）；登录/会话能力不存在。

## 四、Lyra 怎么用它

工程插件（`Plugins\CommonUser\`）+ `CommonGame` 依赖 + `DefaultEngine.ini` 的 `[/Script/CommonUser.CommonSessionSubsystem] bUseLobbiesDefault=true`。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Plugins\CommonGame\CommonGame.uplugin` (x1)
- `Plugins\CommonGame\Source\CommonGame.Build.cs` (x1)
- `Plugins\CommonUser\CommonUser.uplugin` (x2)
- `Plugins\CommonUser\Source\CommonUser\Private\AsyncAction_CommonUserInitialize.cpp` (x1)
- `Plugins\CommonUser\Source\CommonUser\Private\CommonSessionSubsystem.cpp` (x1)
- `Plugins\CommonUser\Source\CommonUser\Private\CommonUserSubsystem.cpp` (x23)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

**S02（插件②）**。

## 六、注意

它的 `.uplugin` 还依赖 `OnlineSubsystem` / `OnlineSubsystemUtils` / `OnlineServices` 三个引擎插件。

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Plugins\CommonUser\CommonUser.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 42 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
