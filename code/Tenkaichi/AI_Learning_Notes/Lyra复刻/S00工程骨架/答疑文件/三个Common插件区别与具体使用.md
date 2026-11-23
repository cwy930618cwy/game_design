# CommonUI / CommonUser / CommonGame —— 三个插件到底怎么分、怎么用

> 触发：`.uproject` 第 18 / 42 / 43 项，注释分别写着「引擎插件」「S02 建源码」「S03 建源码」，看着像三套重复的东西。
> 定位：答疑 md（**不进教学台账**）。逐项详解见 `插件详解\18_CommonUI.md`、`42_CommonUser.md`、`43_CommonGame.md`。

---

## 一、先解晕：一句话

**不是三套重复的 UI 系统，是三层各管一段。**

| 插件 | 管什么 | 一句话比喻 |
|---|---|---|
| `CommonUI` | 控件怎么显示、输入怎么走、焦点落在谁身上 | 舞台的灯光 / 音响 / 轨道 —— 不管台上站的是谁 |
| `CommonUser` | 玩家是谁、登没登录、在哪个会话里 | 检票口 —— 验身份，发座位号 |
| `CommonGame` | 把上面两个装进游戏骨架，并管理 UI 层栈 | 剧场经理 —— 拿检票口的信息，指挥灯光音响给谁用 |

最关键的一句：**`CommonGame` 的 `.uplugin` 里同时把 `CommonUI` 和 `CommonUser` 列为依赖**，官方描述原话就是 "Generic gameplay classes that use the other Common plugins." —— 它就是「把另外两个用起来」的汇聚层。所以你觉得三个都在讲 UI，其实是层层往上组装。

---

## 二、为什么分这么多：来源是来源，职责是职责

困惑通常来自把两个**独立维度**混着看。

### 维度 1：来源不同 —— 决定你要不要敲源码

| 来源 | 本例 | 你要做什么 |
|---|---|---|
| 引擎插件 | `CommonUI`（第 18 项） | 引擎自带，**只读不改**，你只继承 / 调用 |
| 工程插件 | `CommonUser`（42）、`CommonGame`（43） | Lyra 自己的源码，**一比一复刻**到 `Tenkaichi\Plugins\` |

所以注释里「S02 建源码 / S03 建源码」只对**后两个**成立 —— 引擎插件没有源码给你建。

> 实查：Tenkaichi 里这两个插件的源码现在已经敲完并能编译，位置在
> `Plugins\CommonUser\Source\CommonUser\Public\`、`Plugins\CommonGame\Source\Public\`。

### 维度 2：职责不同 —— 决定什么时候用哪个

| 问题 | 找谁 |
|---|---|
| 「我要做一个带手柄导航的菜单 / 弹窗」 | `CommonUI` |
| 「我要判断玩家登录没有 / 建房 / 加房」 | `CommonUser` |
| 「我要在 HUD 上压一层界面 / 管 GameInstance」 | `CommonGame` |

---

## 三、三者依赖链（实查 `.uplugin` 的 `Plugins` 数组）

```
CommonGame  （工程插件 · 汇聚层）
├─ CommonUI                    （引擎插件）
│  ├─ EnhancedInput            （引擎插件）
│  └─ GameplayTagsEditor       （引擎插件）
├─ CommonUser                  （工程插件）
│  ├─ OnlineSubsystem          （引擎插件）
│  ├─ OnlineSubsystemUtils     （引擎插件）
│  └─ OnlineServices           （引擎插件）
├─ ModularGameplayActors       （工程插件）
└─ OnlineFramework             （引擎插件）
```

三条边界要记牢：

- `CommonUser` 只管身份 / 会话，**完全不碰 UI**。
- `CommonUI` 只管控件 / 输入 / 焦点，**完全不碰你是谁**。
- `CommonGame` 两头都吃，还额外吃 `ModularGameplayActors`（Actor 基类）和 `OnlineFramework`（在线服务公共代码）。

---

## 四、具体使用（按你写代码时的场景）

### 场景 A：做界面 → 用 `CommonUI`（引擎给，你只继承）

- 你的 Widget 继承 `UCommonActivatableWidget`（`Engine\Plugins\Runtime\CommonUI\Source\CommonUI\Public\CommonActivatableWidget.h:32`）。Lyra 的 `ULyraActivatableWidget` 就是这么写的（`Source\LyraGame\UI\LyraActivatableWidget.h:22`）。
- 打开 / 关闭靠 `DeactivateWidget`（同文件 `:44`）；容器栈靠 `CommonActivatableWidgetContainer.h:31` 的 `AddWidget`、`:68` 的 `RemoveWidget`。
- 焦点靠 `GetDesiredFocusTarget`（`CommonActivatableWidget.h:67`）—— 这就是「打开菜单自动选中第一个按钮」的来源。
- 判断当前是手柄还是键鼠，靠 `UCommonInputSubsystem::GetCurrentInputType`（`Source\CommonInput\Public\CommonInputSubsystem.h:56`）。

> ⚠️ 实查纠正：这个 UE 版本里输入路由类叫 **`UCommonUIActionRouterBase`**（`Source\CommonUI\Public\Input\CommonUIActionRouterBase.h:62`），**没有** `UCommonUIActionRouter`；也**没有** `SetInputMode`，对应能力是 `SetActiveUIInputConfig`（`:117`）。别按旧笔记里的名字去搜。

### 场景 B：登录 / 建房 / 加房 → 用 `CommonUser`（你复刻它的源码）

关键入口（`Plugins\CommonUser\Source\CommonUser\Public\CommonUserSubsystem.h`）：

| 方法 | 行号 | 什么时候调 |
|---|---|---|
| `TryToInitializeForLocalPlay` | `:283` | 本地游玩初始化玩家 |
| `TryToLoginForOnlinePlay` | `:294` | 联网登录 |
| `GetUserInfoForLocalPlayerIndex` | `:250` | 查某个本地玩家的信息 |
| `ResetUserState` | `:327` | 退回前端 / 重置状态 |

会话侧在 `CommonSessionSubsystem.h`：`HostSession`（`:264`）、`JoinSession`（`:272`）、`FindSessions`（`:276`）、`CleanUpSessions`（`:280`）。

Lyra 真实调用点：

- `Source\LyraGame\UI\Frontend\LyraFrontendStateComponent.cpp:134` 调 `TryToInitializeForLocalPlay`；`:97`、`:104` 调 `ResetUserState` / `CleanUpSessions`。
- `Source\LyraGame\GameModes\LyraGameMode.cpp:182` 在专用服务器调 `TryToLoginForOnlinePlay`；`:258` 调 `HostSession`。

### 场景 C：搭游戏骨架 / 往 HUD 压界面 → 用 `CommonGame`（你复刻它的源码）

- `UCommonGameInstance : UGameInstance`（`Plugins\CommonGame\Source\Public\CommonGameInstance.h:23`）。Lyra 的 `ULyraGameInstance` 继承它（`Source\LyraGame\System\LyraGameInstance.h:13`）。
- `UCommonLocalPlayer : ULocalPlayer`（`CommonLocalPlayer.h:18`）→ `ULyraLocalPlayer`（`Source\LyraGame\Player\LyraLocalPlayer.h:25`）。
- `ACommonPlayerController : AModularPlayerController`（`CommonPlayerController.h:13`）→ `ALyraPlayerController`（`Source\LyraGame\Player\LyraPlayerController.h:31`）。
- UI 层注册：`UPrimaryGameLayout::RegisterLayer`（`PrimaryGameLayout.h:36`，实现在 `.cpp:89`）。
- 业务代码推界面：`UCommonUIExtensions::PushContentToLayer_ForPlayer`（`CommonUIExtensions.h:22`，实现在 `.cpp:55-68`），沿 UIManager → Policy → RootLayout → `PushWidgetToLayerStack` 一路压栈。

**组装关系（谁创建谁）**：

```
Controller 广播 OnPlayerControllerSet
  → GameInstance::AddLocalPlayer 通知 UGameUIManagerSubsystem
    → UIManager::Initialize 里 NewObject<UGameUIPolicy>
      → Policy::CreateLayoutWidget 里 CreateWidget<UPrimaryGameLayout> 并加进视口
        → 业务代码 PushContentToLayer_ForPlayer 把具体界面压进层栈
```

---

## 五、一条完整链路串起来（从点「开始游戏」到菜单显示）

1. `LyraFrontendStateComponent` 调 `CommonUser` 的 `TryToInitializeForLocalPlay`，验明玩家身份。
2. 要联网时 `CommonSessionSubsystem::HostSession` / `JoinSession` 建或加会话。
3. 切图后 `ULyraGameInstance`（继承 `CommonGameInstance`）起来，`AddLocalPlayer` 通知 UI 管理器。
4. UI 管理器建 `GameUIPolicy`，Policy 建 `PrimaryGameLayout` 加进视口 —— **此时屏幕上还没有具体菜单**。
5. 业务代码调 `CommonUIExtensions::PushContentToLayer_ForPlayer`，把主菜单压进层栈。
6. 主菜单里每个 Widget 继承 `UCommonActivatableWidget`，接下来手柄导航、Esc 逐层退出、焦点环全由 `CommonUI` 接管。

看明白这条链，三个插件的边界就不会混了：**第 1-2 步是 CommonUser，第 3-5 步是 CommonGame，第 6 步是 CommonUI。**

---

## 六、本复刻线什么时候碰

| 阶段 | 动作 | 说明 |
|---|---|---|
| S02 | 敲 `CommonUser` 全部源码（13 文件） | 工程插件，一比一照抄，不改名不简化 |
| S03 | 敲 `CommonGame` 全部源码（29 文件） | 工程插件，一比一照抄 |
| S18 | 全程用 `CommonUI` | 引擎插件，只读；引用它的 Lyra UI 模块约 79 文件 |

这三个插件**相互之间没有任何需要你改的源码**，照抄即可；你要写的只是自己的类去**继承**它们。

---

## 七、三个最容易踩的误解

1. **以为 `CommonUI` 是 `CommonGame` 的一部分** —— 不是。前者引擎自带，后者是 Lyra 工程源码，只是后者依赖前者。
2. **以为 `CommonUser` 管 UI** —— 完全不碰。它只负责身份、登录、会话；你要显示什么界面跟它无关。
3. **按旧名字搜 `UCommonUIActionRouter` / `ULyraPrimaryGameLayout` 搜不到** —— 前者本版本叫 `UCommonUIActionRouterBase`；后者 **C++ 里不存在**，Lyra 只有蓝图子类，C++ 侧只有插件基类 `UPrimaryGameLayout`。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Runtime\CommonUI\CommonUI.uplugin`
- `E:\ue5\LyraStarterGame\Plugins\CommonUser\CommonUser.uplugin`
- `E:\ue5\LyraStarterGame\Plugins\CommonGame\CommonGame.uplugin`
- `E:\ue5\LyraStarterGame\Plugins\CommonGame\Source\Public\*.h` 与对应 `Source\Private\*.cpp`
- `E:\ue5\LyraStarterGame\Source\LyraGame`（`LyraGameInstance.h:13`、`LyraLocalPlayer.h:25`、`LyraPlayerController.h:31`、`LyraActivatableWidget.h:22`、`LyraFrontendStateComponent.cpp:134`、`LyraGameMode.cpp:182/258`）
- `E:\ue5\game_design\code\Tenkaichi\Plugins\CommonUser\CommonUser.uplugin`、`Plugins\CommonGame\CommonGame.uplugin`
