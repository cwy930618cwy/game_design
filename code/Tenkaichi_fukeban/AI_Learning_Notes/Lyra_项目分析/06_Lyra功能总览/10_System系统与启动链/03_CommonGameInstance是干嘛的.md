# 03 — `UCommonGameInstance` 是干嘛的：CommonGame 插件的"增强版 GameInstance"

> **定位**：上一篇 `02` 里 `ULyraGameInstance` 继承了它，但没说它本身是谁。这一篇只讲一件事：**`UCommonGameInstance` 这个类到底在 Lyra 里起什么作用**。
>
> **一句话**：它是 Lyra 项目自带插件 **CommonGame** 提供的"**增强版 GameInstance**"——把 Lyra 的账号体系、会话体系、UI 体系**互相接上线**，Lyra 再继承它补自己的逻辑。

---

## 〇、先搞清楚：它是哪来的、源码在哪

**它不是 UE 引擎自带的**，是 **Epic 做 Lyra 时自己写的"通用游戏框架"插件**，随 Lyra 工程一起发给你。这类以 `Common` 开头的插件（CommonGame/CommonUser/CommonUI/CommonSession…）都是同一批产物，统一放在 Lyra 工程的 `Plugins/` 目录里。

本机源码位置（自己动手能找到）：

```
头文件：e:\code\lyra_fifty_six\LyraStarterGame\Plugins\CommonGame\Source\Public\CommonGameInstance.h
实现：  e:\code\lyra_fifty_six\LyraStarterGame\Plugins\CommonGame\Source\Private\CommonGameInstance.cpp
```

目录结构视角：

```
LyraStarterGame/                       ← 整个 Lyra 工程
├── Config/  Content/  Source/         ← LyraGame 主模块源码在这
└── Plugins/                           ← ★项目自带插件（共 17 个）
    ├── CommonGame/                    ← CommonGameInstance 住这
    │   ├── CommonGame.uplugin         ← 插件"身份证"
    │   └── Source/
    │       ├── Public/CommonGameInstance.h     ← 类声明
    │       └── Private/CommonGameInstance.cpp  ← 类实现
    ├── CommonUser/                    ← 账号子系统（同批插件）
    ├── CommonUI / UIExtension / ...
    └── （其余项目插件）
```

**三方关系**：

```
 UE 引擎（D:\ue5\Epic Games\UE_5.6）    ← 提供 UGameInstance（引擎不内置 CommonGame）
 Lyra 工程 e:\code\lyra_fifty_six\LyraStarterGame
   ├─ Plugins/CommonGame/               ← CommonGameInstance 源码（Epic 写、随 Lyra 发）
   └─ Source/LyraGame/System/LyraGameInstance.h
        └─ include "CommonGameInstance.h"  ← 跨过去包含插件里的头文件
```

**30 秒自查法**：在 VS/Rider 里打开 `LyraGameInstance.h`，光标放 `UCommonGameInstance` 上按 **F12** → 直接跳到上面那个 `.h` 路径——跳到的位置就是答案。

---

## 一、先看这张"总接线图"：它是谁、把什么线接在哪

```
  玩家平时碰到的事：登录 / 好友邀请 / 加本地玩家
            │（平台产生事件）
            ▼
 ┌──────────────────────────────────────────────────────────┐
 │  UGameInstance（引擎地基：通用跨图状态）                    │
 │      ▲ 被 CommonGame 插件扩展                            │
 │      │                                                  │
 │  UCommonGameInstance（★ 三条线的"总接线中枢"）            │
 │  ┌────────────────────────────────────────────────────┐  │
 │  │ 线① 账号线 ← CommonUser 子系统                       │  │
 │  │    · 登录完成 ──► HandlerUserInitialized()          │  │
 │  │    · 出错了   ──► 弹错误框                          │  │
 │  │    · 权限丢了 ──► 记日志                            │  │
 │  │ 线② 房间线 ← CommonSession 子系统                    │  │
 │  │    · 收到好友邀请 ──► 安排进房流程                    │  │
 │  │    · 平台要解散房间 ──► 回主菜单                      │  │
 │  │ 线③ 玩家线 ← 本地玩家增减                            │  │
 │  │    · 加玩家/删玩家 ──► 通知 UI 管理器                 │  │
 │  └────────────────────────────────────────────────────┘  │
 │      ▲ 被 Lyra 扩展                                      │
 │      │                                                  │
 │  ULyraGameInstance（02篇：登录后读玩家设置 + 注册状态机）   │
 └──────────────────────────────────────────────────────────┘

  它最常出场的 2 个现场：
   现场A · 玩家登录游戏
   平台登录 ──► CommonUser 登录完成 ──► HandlerUserInitialized()
                                        └─ Lyra：让玩家读磁盘设置

   现场B · 好友邀请进房
   好友邀请 ──► CommonSession 收到 ──► OnUserRequestedSession()
                                        ├─ 现在能进 → 直接进房
                                        └─ 不能进   → 先回主菜单，
                                                     等就绪再进
```

> 继承关系一句话：**引擎给你毛坯房（UGameInstance）→ CommonGame 插件给你接好水电（UCommonGameInstance）→ Lyra 摆上自家家具（ULyraGameInstance）**。下面正文就是"接线中枢"里每一格的详解。

---

## 二、它到底干了什么：4 类活

### ① 当"接线员"（核心，`Init()` 里全干完）

游戏启动时，它把自己注册成**三个子系统的中转站**：

```
 UCommonGameInstance::Init()
 │  读取平台特性标签(PlatformTraits)
 │
 ├── 接上 CommonUserSubsystem（账号/权限）
 │      ├─ OnHandleSystemMessage ────► HandleSystemMessage()   （弹错误框）
 │      ├─ OnUserPrivilegeChanged ──► HandlePrivilegeChanged() （权限丢失处理）
 │      └─ OnUserInitializeComplete ► HandlerUserInitialized() （登录完成回调）
 │
 └── 接上 CommonSessionSubsystem（会话/进房）
        ├─ OnUserRequestedSessionEvent ► OnUserRequestedSession()  （平台邀请）
        └─ OnDestroySessionRequestedEvent ► OnDestroySessionRequested()（解散要求）
```

**逐个说这些回调默认干嘛**（都读它的 `.cpp` 核实过）：

| 回调 | 默认行为 |
|---|---|
| `HandleSystemMessage` | 收到严重错误消息（带 `SystemMessage_Error` 标签）→ 给一号玩家**弹错误对话框** |
| `HandlePrivilegeChanged` | 一号玩家失去 `CanPlay` 权限 → 打错误日志（子类可改成踢回主菜单） |
| `HandlerUserInitialized` | **空实现**，故意留给子类 → Lyra 在这里加"登录成功→读玩家设置" |
| `OnUserRequestedSession` | 收到平台邀请/进房请求 → 走下面的"入房流程" |
| `OnDestroySessionRequested` | 平台要求解散房间 → 默认 `ReturnToMainMenu()` |

### ② 当"玩家管理员"

| 函数 | 干嘛 |
|---|---|
| `AddLocalPlayer` | 记录"谁是一号玩家"（Primary Player）；通知 `GameUIManagerSubsystem`（UI 管理器）"有人进游戏了" |
| `RemoveLocalPlayer` | 清掉一号玩家记录；通知 UI 管理器"有人离开了" |

> 💡 **场景**：插了手柄加了个本地玩家 → UI 管理器收到通知，分屏/双人界面才跟着变。

### ③ 当"入房协调员"（好友邀请→进房流程）

```
 你在 Steam 点了好友的"加入游戏"邀请
   │
   ▼ 平台事件 → OnUserRequestedSession()
   │
   ▼ SetRequestedSession(目标房间)
   │
   ├─ CanJoinRequestedSession() == true？──► JoinRequestedSession()：直接进
   │                                          （默认总是 true，游戏可覆写）
   └─ 不能现在进？──► ResetGameAndJoinRequestedSession()
                       默认 = 先回主菜单，等能进时再由游戏调 Join
```

> 💡 **场景**：玩家还在主菜单/加载中就被邀请 → 系统先让他**回主菜单**，避免"半路硬切房间"出 bug。

### ④ 当"收尾员"

| 函数 | 默认行为 |
|---|---|
| `ReturnToMainMenu()` | 先 `ResetUserAndSessionState()`（清用户状态 + 清空会话），再回主菜单 |
| `ResetUserAndSessionState()` | `UserSubsystem->ResetUserState()` + `SessionSubsystem->CleanUpSessions()` |

> 💡 **场景**：一把打完、解散房间、断线重连前——全都先走这套"重置"，保证下一局是干净状态。

---

## 三、留给游戏去 override 的口子（这也是 Lyra 用它的原因）

| 虚函数 | 默认 | Lyra 做了什么 |
|---|---|---|
| `HandlerUserInitialized` | 空 | `ULyraGameInstance`：登录成功 → 玩家读磁盘设置 |
| `CanJoinRequestedSession` | 永远 true | `ULyraGameInstance`：保持 true（注释：将来检查玩家状态） |
| `Init / Shutdown` | 接完线就完 | 加自己的：注册组件状态机、订阅进服事件 |

> 也就是说：**通用逻辑它全写好了，你只需要继承它补 2~3 个口子**——这就是 Lyra 不直接继承引擎 `UGameInstance` 的原因。

---

## 四、本篇一句话

`UCommonGameInstance` = CommonGame 插件写的"**精装版 GameInstance**"：启动时自动把**账号（CommonUser）、会话（CommonSession）、UI（GameUIManager）**三条线接好，还管好"玩家增减"和"邀请进房"两件事，并把登录完成、能否进房等口子留给游戏。**Lyra 的 `ULyraGameInstance` = 在这套现成框架上补自己逻辑**——这也是为什么你学 Lyra 时总会看到 `UCommonXxx` 前缀：它们都是 Common 全家桶里的现成零件。
