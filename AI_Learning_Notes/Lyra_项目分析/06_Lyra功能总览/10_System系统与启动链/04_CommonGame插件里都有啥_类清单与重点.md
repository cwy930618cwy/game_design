# 04 — CommonGame 插件里都有啥：类清单 + 哪些重要

> **定位**：`03` 讲了插件里的 `UCommonGameInstance`。这一篇把整个 **CommonGame 插件**摊开看：它包含哪些类、分别干嘛、哪些是重点。
>
> **一句话**：CommonGame = Epic 写的"**通用玩法胶水插件**"——它把 Common 全家桶（CommonUI/CommonUser）装进一套**游戏通用的类**里，重点提供两样东西：**① 玩家的增强基类**（GameInstance/LocalPlayer/PlayerController）**② 一套"UI 装配流水线"**（给每个玩家建根布局、按层堆 UI）。

---

## 一、先看插件档案（`CommonGame.uplugin`）

```
 FriendlyName: CommonGame
 Description:  "Generic gameplay classes that use the other Common plugins."
              （给其他 Common 插件提供通用玩法类的"胶水插件"）
 CreatedBy:   Epic Games, Inc.

 模块：CommonGame（Runtime，一个运行时模块）
 依赖的插件：CommonUI / CommonUser / ModularGameplayActors / OnlineFramework
```

> 它自己不带美术内容（`CanContainContent: false`），全是一堆 C++ 类。

---

## 二、内部结构总览（按"派系"分，★=重要性）

```
 Plugins/CommonGame/Source/Public/          ← 全部 12 个头文件
 ════════════════════════════════════════════════════════════════
 派系① 玩家增强基类（给 Lyra 的角色/玩家当爹）
   CommonGameInstance.h        UCommonGameInstance       ★★★（03已讲）
   CommonLocalPlayer.h         UCommonLocalPlayer        ★★★
   CommonPlayerController.h    ACommonPlayerController   ★★★
   CommonPlayerInputKey.h      （输入按键显示控件）         ★★
 派系② UI 装配流水线（给每个玩家建 UI 根 + 分层）
   GameUIManagerSubsystem.h    UGameUIManagerSubsystem   ★★★
   GameUIPolicy.h              UGameUIPolicy             ★★★
   PrimaryGameLayout.h         UPrimaryGameLayout        ★★★
 派系③ 弹窗/消息
   Messaging/CommonMessagingSubsystem.h   UCommonMessagingSubsystem  ★★
   Messaging/CommonGameDialog.h           UCommonGameDialog 描述器   ★★
 派系④ 蓝图便捷节点 & 工具
   CommonUIExtensions.h        UCommonUIExtensions 函数库 ★★
   Actions/AsyncAction_CreateWidgetAsync.h         ★★
   Actions/AsyncAction_PushContentToLayerForPlayer.h  ★★
   Actions/AsyncAction_ShowConfirmation.h         ★★
 派系⑤ 内部
   Private/LogCommonGame.h    日志频道                     ★（不用管）
 ════════════════════════════════════════════════════════════════
```

---

## 三、重点逐个说（★★★ 五件套）

### ① `UCommonLocalPlayer` + `ACommonPlayerController` —— 玩家就绪事件链

这对父子负责告诉你："**这个玩家到哪个阶段了**"。

| 类 | 提供的事件（收到就通知） |
|---|---|
| `UCommonLocalPlayer` | `OnPlayerControllerSet`（玩家拿到控制器）、`OnPlayerStateSet`（拿到状态）、`OnPlayerPawnSet`（拿到 Pawn）、`GetRootUILayout()` |
| `ACommonPlayerController` | 在这些时刻（`ReceivedPlayer`/`SetPawn`/`Possess`）**主动去广播**上面那些事件 |

> 💡 **场景**：你的 HUD 要等"玩家真的拿到 Pawn"才绑定血条——订阅 `OnPlayerPawnSet` 即可，不用自己瞎轮询。

### ② UI 装配流水线：`UGameUIManagerSubsystem` → `UGameUIPolicy` → `UPrimaryGameLayout`

```
 UGameUIManagerSubsystem（UI 总管，GameInstance级）
   │  选择并持有当前"UI 策略"
   ▼
 UGameUIPolicy（策略：给每个本地玩家建"根布局"）
   │  分屏时：一个玩家一个根布局（PrimaryOnly/单切换/同屏）
   ▼
 UPrimaryGameLayout（每个玩家的 UI 根）
   │  里面分成多个 UI 层(Layer)，HUD/菜单/弹窗各占一层
   ▼
 玩家用 UIExtension / Push 动作把界面塞进对应层
```

| 类 | 一句话 |
|---|---|
| `UGameUIManagerSubsystem` | 游戏 UI 的"总开关"：管当前用哪个 UIPolicy；玩家加入/离开时通知（`NotifyPlayerAdded`）。Abstract，游戏要子类化 |
| `UGameUIPolicy` | 负责给每个 `ULocalPlayer` 创建一个根布局 `UPrimaryGameLayout`；管本地多人（分屏）模式 |
| `UPrimaryGameLayout` | **游戏 UI 的根**：一个玩家一个；把 HUD、菜单、弹窗推进不同 Layer 并管理显示/隐藏 |

> 💡 **场景**：主菜单按"开始游戏"→ 游戏把"游戏HUD"推入 `UPrimaryGameLayout` 的某个层、把主菜单 UI 隐藏——这套"推层/弹层"的骨架就是上面三件套搭的。

### ③ 其余值得知道（★★）

| 类 | 干嘛 |
|---|---|
| `CommonMessagingSubsystem` + `CommonGameDialog` | 弹**错误/确认框**的系统。还记得 03 里 `HandleSystemMessage` 弹错误框吗？就是它干的 |
| `CommonUIExtensions` | 蓝图函数库：把 Widget 推/弹出某层等常用操作，蓝图直接调 |
| `AsyncAction_CreateWidgetAsync` | 蓝图节点：**异步**加载并创建 Widget（不卡 UI 线程） |
| `AsyncAction_PushContentToLayerForPlayer` | 蓝图节点：把一个内容推进某玩家的 UI 层（常用！） |
| `AsyncAction_ShowConfirmation` | 蓝图节点：弹出确认框并等结果 |
| `CommonPlayerInputKey` | 显示"按键"的小控件（会按键鼠/手柄自动换图标，多用于"按 E 互动"这类提示） |

---

## 四、Lyra 是怎么"接住"这套基类的（实例对实例）

| CommonGame 给的基类 | Lyra 的子类（在 LyraGame 模块） |
|---|---|
| `UCommonGameInstance` | `ULyraGameInstance`（System/，02 已学） |
| `UCommonLocalPlayer` | `ULyraLocalPlayer`（Player/，加队伍/设置） |
| `ACommonPlayerController` | `ALyraPlayerController`（Player/） |
| `UGameUIManagerSubsystem` | `ULyraUIManagerSubsystem`（UI/Subsystem/） |

> 这个表就是学 Lyra 的"**对照图**"：看到一个 `CommonXxx`，就能猜到 Lyra 里大概率有个 `LyraXxx` 继承它。

---

## 五、本篇一句话

CommonGame 插件 = **一包"游戏通用 C++ 类"**：★核心是**玩家增强基类**（GameInstance/LocalPlayer/PlayerController，负责"玩家就绪事件链"）和 **UI 装配流水线**（UIManager→UIPolicy→PrimaryGameLayout，负责"给每个玩家建根、按层堆 UI"）；再配几个弹窗/异步加载的便捷工具。学它不用背全部，记住"**三个 Player 类 + UI 三件套 + 弹窗**"就抓住了主心骨，剩下的（AsyncAction 那几个）用到再查。
