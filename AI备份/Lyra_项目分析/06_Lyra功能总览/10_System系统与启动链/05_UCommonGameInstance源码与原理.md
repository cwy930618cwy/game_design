# 05 — `UCommonGameInstance` 源码与原理：它靠什么机制干活

> **定位**：`03` 说了它"是干嘛的"（4 类活）。这一篇往下一层：**它这些活是怎么做到的**？读源码，讲清背后的 3 个机制和完整调用链。
>
> **本篇一句话**：它工作的本质 = **① 覆写引擎回调 ② 用事件委托把自己绑到子系统上 ③ 留虚函数口子给子类**。三个机制配合，就成了 Lyra 的"账号/会话/UI 接线中枢"。

---

## 〇、先理关系：`UGameInstance` 和 `UCommonGameInstance` 是谁是谁

```
        引擎（Engine/Source/Runtime/Engine）
   UGameInstance     ← 地基：通用跨图状态
         ▲  被 CommonGame 插件扩展
         │
   UCommonGameInstance（★Abstract，只当模板，不能直接创建）
         ▲  被 LyraGame 模块扩展
         │
   ULyraGameInstance（Lyra 里真正实例化的那个）
```

| 对比 | `UGameInstance` | `UCommonGameInstance` |
|---|---|---|
| 谁写的 | Epic **引擎** | Epic 给 Lyra 写的 **CommonGame 插件** |
| 源码在哪 | `Engine/.../GameInstance.h`（装引擎就有） | `Lyra/Plugins/CommonGame/Source/...`（只在带插件的工程有） |
| 本质 | **基类**：跨图状态 + 生命周期（空壳框架） | **子类**：在这个基础上接好 账号/会话/UI 三条线 |
| 自带能力 | `Init/Shutdown`、管 LocalPlayer、回主菜单 | 上面全有 + 委托接线 + 邀请进房流程 + 玩家事件链 + 弹窗 |
| 能不能直接用 | 能（传统项目直接继承它写逻辑） | **不能**（`Abstract`），只能被继承，Lyra 再子类化 |
| 你该用哪个 | 没走 Common 全家桶的传统项目 | 要用 CommonUI/CommonUser 那套登录/UI 体系（如 Lyra） |

**一句话**：`UGameInstance` 是"毛坯房"（引擎通用）；`UCommonGameInstance` 是"**帮你接好水电的精装模板**"（CommonGame 插件提供）；二者是**父子关系**——父在引擎里，子只在带 CommonGame 插件的工程（Lyra）里。**本文件（05）下面拆的三大块机制，全是这个"子类"在父类基础上加的。**

---

## 一、全景：它的代码分三大块（先背结构）

```
 CommonGameInstance.cpp（211行实现）
 ═══════════════════════════════════════════════════════════════
 块A 引擎回调覆写（引擎到点"敲你门"）
    Init / AddLocalPlayer / RemoveLocalPlayer / ReturnToMainMenu
 块B 事件处理函数（被子系统"喊到"后执行）
    HandleSystemMessage / HandlePrivilegeChanged / HandlerUserInitialized
    OnUserRequestedSession / OnDestroySessionRequested
 块C 进房流程工具（自己内部编排）
    SetRequestedSession / CanJoinRequestedSession /
    JoinRequestedSession / ResetGameAndJoinRequestedSession
 ═══════════════════════════════════════════════════════════════
```

它之所以"又像总开关又像接线员"，是因为它同时站了三种位置：**引擎的"孩子"**（被引擎调用）、**子系统的"接线板"**（订阅事件）、**子类的"模板"**（留口子）。下面逐个机制拆。

---

## 二、机制①：覆写引擎回调——"引擎到点就敲你的门"

`UGameInstance` 本来就是引擎生命周期里的对象，引擎在特定时刻**主动调用**它的虚函数。CommonGameInstance 只需覆写几个：

| 引擎什么时候调 | 覆写的函数 | CommonGame 在里面干嘛 |
|---|---|---|
| 游戏进程启动 | `Init()` | 把子系统接上自己（见机制②） |
| 有人成为本地玩家 | `AddLocalPlayer()` | 记录一号玩家 + 通知 UI 管理器 |
| 本地玩家离开 | `RemoveLocalPlayer()` | 清一号玩家记录 + 通知 UI 管理器 |
| 要回主菜单 | `ReturnToMainMenu()` | 先重置用户+会话，再回菜单 |

> 💡 **场景原理**：你在主菜单点"退出到主菜单" → 引擎走 `ReturnToMainMenu` → 因为被覆写，实际先执行 CommonGame 这段：`ResetUserAndSessionState()`（清登录态+解散旧房）→ 保证下次开房是干净的。**你不需要知道引擎内部怎么调它，只要覆写对的函数**。

---

## 三、机制②：事件委托——"我把电话线接到子系统的分机上"

这是最核心的原理。看 `Init()` 真实代码：

```cpp
void UCommonGameInstance::Init()
{
	Super::Init();   // 先让引擎默认初始化

	// 拿到"账号子系统"，把它的三个事件接给自己
	UCommonUserSubsystem* UserSubsystem = GetSubsystem<UCommonUserSubsystem>();
	UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &UCommonGameInstance::HandleSystemMessage);
	UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &UCommonGameInstance::HandlePrivilegeChanged);
	UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &UCommonGameInstance::HandlerUserInitialized);

	// 拿到"会话子系统"，把两个事件接给自己
	UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>();
	SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &UCommonGameInstance::OnUserRequestedSession);
	SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &UCommonGameInstance::OnDestroySessionRequested);
}
```

**这里的关键概念是"委托（Delegate）"**：

```
 AddDynamic(对象, &函数) 的含义：
 "将来子系统一广播这个事件，就替我调用这个对象的这个函数"

 子系统的委托            CommonGameInstance 的处理器
 OnUserInitializeComplete ───► HandlerUserInitialized()
 OnUserPrivilegeChanged  ───► HandlePrivilegeChanged()
 OnHandleSystemMessage   ───► HandleSystemMessage()
 OnUserRequestedSessionEvent ─► OnUserRequestedSession()
```

**所以登录的完整链路是**：

```
 玩家点"登录"
   │  CommonUserSubsystem 干活（向平台要登录）
   ▼
 登录完成，子系统广播 OnUserInitializeComplete
   │  ──委托─► 触发 UCommonGameInstance::HandlerUserInitialized()
   ▼
 里面默认什么都不干 → 但 Lyra 覆写了它 → 去读玩家设置
```

> 💡 **类比**：`AddDynamic` = 把自家电话接到子系统的总机分机上。以后平台登录一结束，子系统"叮铃铃"拨过来，接电话的正好是你登记好的那个函数。

> 小知识：`AddDynamic` 是 **UMulticastDelegate**（可绑多个听众、可反射）的用法；`.cpp` 里 `AddUObject` 给非 UFUNCTION 绑定。原理都是"事件发生时遍历听众挨个通知"。

---

## 四、机制③：虚函数口子——"有些事我留白，让游戏自己填"

CommonGame 故意把部分实现写成**空/默认**，注释直接写着 "Subclasses can override this"：

| 函数 | 默认 | 为什么留口子 |
|---|---|---|
| `HandlerUserInitialized` | 空 | 登录成功干什么，只有游戏自己知道（Lyra：读设置） |
| `CanJoinRequestedSession` | 恒 true | 能不能现在进房，游戏规则说了算 |
| `HandlePrivilegeChanged` | 只记日志 | 玩家失去权限怎么处理，交给游戏（可踢回菜单） |
| `HandleSystemMessage` | 弹错误框 | 系统消息怎么呈现，游戏可换成自己的弹窗 |

> 💡 **模式**：这套"父类把流程搭好 + 关键步骤留虚函数让子类填"叫 **模板方法模式**——Lyra 只覆写其中 2~3 个口子，就完成了"带自家逻辑的登录流程"。

---

## 五、完整时序：一次"好友邀请进房"到底经过哪些代码

```
 你在 Steam 点好友的加入邀请
   │
   ▼ ① 平台层解析 → CommonSessionSubsystem 广播 OnUserRequestedSessionEvent
   ▼ ② 委托触发 UCommonGameInstance::OnUserRequestedSession()
   │        if (邀请有效) SetRequestedSession(房间)
   │        else          HandleSystemMessage(弹"邀请失败")
   ▼ ③ SetRequestedSession() 内部编排：
   │
   ├─ CanJoinRequestedSession()？── 是 ──► ④ JoinRequestedSession()
   │                                      调用 SessionSubsystem->JoinSession(...)
   │                                      真正进房（清掉 RequestedSession）
   │
   └─ 否（游戏没就绪）──► ResetGameAndJoinRequestedSession()
                           默认 → ReturnToMainMenu()（先回主菜单）
                           等游戏就绪后再由代码主动调 Join
```

**观察这个设计**：
- CommonGame 把"进房"编排成小状态机（收到邀请→判断→执行），**具体的能不能进/进房后干嘛**都留给你；
- `RequestedSession` 是一个 `UPROPERTY()` 成员，用来暂存"想进的房间"，防止流程进行中事件重入。

---

## 六、一张图收尾：三个机制怎么叠起来

```
               引擎（启动/加人/回菜单时来敲门）
                        │  机制①：覆写回调
                        ▼
        UCommonGameInstance（中央）
        │  机制②：委托接线             │  机制③：留口子
        ▼                              ▼
  CommonUserSubsystem（账号）      游戏子类（Lyra 的
  CommonSessionSubsystem（会话）    ULyraGameInstance）
  GameUIManagerSubsystem（UI）      补"自家逻辑"
```

**本篇一句话**：`UCommonGameInstance` 不写具体玩法，它靠三条机制干活——**①覆写引擎生命周期回调**（启动/加人/回菜单时被引擎调用）、**②用委托把自己接到账号/会话子系统上**（事件一来就触发对应函数）、**③把关键步骤留成虚函数**（登录后干嘛、能不能进房由游戏填）。看它的源码只要抓住这三条线，其余都是细节。后续在 `UI/Subsystem/` 看 `ULyraUIManagerSubsystem` 怎么把这套 UI 装配接进 Lyra，就更能体会"父类搭架、子类填空"的威力。

---

## 七、"它到底干了啥"—— 用《剑与魔法》上线日讲一遍（结合代码）

> 前面都在拆机制。也许你还在想：**说到底 UCommonGameInstance 到底干了啥？**
> 我们接着 `06` 篇那个双胞胎故事：哥哥第二版已经改成继承 `UCommonGameInstance`。今天是《剑与魔法》**上线日**，跟着玩家"小明"看它一天，就全懂了。

### 早上 8:00 —— 游戏启动，接线（它干的第一件事）

小明点开游戏，引擎创建一个 `UMyRPG_GameInstance`（它的爹就是 `UCommonGameInstance`）。启动那一刻，引擎调 `Init()`，但**你看到的大部分接线根本不用哥哥写**——在 `UCommonGameInstance::Init()` 里已经发生了：

```cpp
// 这是 CommonGame 自己写的（哥哥一行没写）
UCommonUserSubsystem* UserSubsystem = GetSubsystem<UCommonUserSubsystem>();
UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &UCommonGameInstance::HandlerUserInitialized);
//  ↑ 翻译：将来账号登录一完成，系统就会喊我一声
```

> **故事视角**：哥哥只做了一件事——游戏启动时**把电话线接到平台登录总机上**。这通电话不是打给他，是打给一个"以后登录完成会响"的铃声。

### 上午 9:00 —— 小明登录（它干活的第二个现场）

小明输账号点登录。平台验证通过后，`CommonUser` 子系统广播"登录完成"。因为早上接过线，铃声响起，自动触发：

```
 平台登录完成
   │ 广播 OnUserInitializeComplete
   ▼ 委托触发（机制②）
 UCommonGameInstance::HandlerUserInitialized(...)   ← 默认空的
   │                                                ← 但哥哥 override 了
   ▼
 UMyRPG_GameInstance::HandlerUserInitialized() { LoadMySettings(); }
   → 小明上次调好的画质/音量/按键全部生效
```

> **故事视角**：铃声响，接电话的是谁？——**哥哥家自己的分机**。CommonGame 只负责"电话接到总机、铃声会响"，接起来之后说什么话，全是哥哥（Lyra/你的代码）的事。

### 下午 3:00 —— 好友拉小明进房（它干活的第三个现场）

小明正被 BOSS 打，好友发来"加入我的房间"。平台把这请求送到会话子系统，`OnUserRequestedSessionEvent` 广播，委托再次响起：

```cpp
// CommonGame 内部（哥哥没写）：
void UCommonGameInstance::OnUserRequestedSession(...)
{
	SetRequestedSession(房间);   // 先把"想进的房间"存起来
	// SetRequestedSession 内部又问一句：
	if (CanJoinRequestedSession())      // ← 虚函数！默认 true，游戏可改
		JoinRequestedSession();          //    能进 → 真的调 JoinSession 进房
	else
		ResetGameAndJoinRequestedSession();  // 不能进 → 先回主菜单等就绪
}
```

> **故事视角**：小明正被 BOSS 追着砍，肯定不能现在切房——CommonGame 发现"进不去"就自动安排"先回主菜单"，等哥哥把小明送回安全状态，再由哥哥的代码调进房。**它像个懂规矩的管家：流程全编排好，但"现在能不能切"这种事它会先问主人（你的 override）。**

### 晚上 11:00 —— 小明下线（收尾现场）

小明关游戏/回主菜单 → 引擎调 `ReturnToMainMenu()` → CommonGame 覆写先执行 `ResetUserAndSessionState()`（清登录态 + 把旧房间会话清干净）→ 明天再开游戏时是干净状态。

### 讲完故事，一句话总结它"到底干了啥"

```
 UCommonGameInstance 全程没碰任何《剑与魔法》的内容（剑、魔法、Boss 它一概不知）。
 它只做了三件"管家活"：
   ① 启动时把 登录/会话/玩家 的电话线都接好（Init + AddDynamic）
   ② 事件一响，按编排好的流程走（登录完叫你 / 邀请进房先问能不能）
   ③ 把"接电话后说什么、能不能进房"的决定权，用虚函数还给你
```

**对照表（故事 ↔ 代码）**：

| 故事 | 代码 |
|---|---|
| 早上接电话线 | `Init()` 里 5 行 `AddDynamic/AddUObject` |
| 登录完成铃响 | 委托触发 → `HandlerUserInitialized()` |
| 接电话后哥哥说话 | 哥哥 override `HandlerUserInitialized` 读设置 |
| 好友邀请先问主人 | `CanJoinRequestedSession()`（虚函数口子） |
| 睡前收拾房间 | `ReturnToMainMenu → ResetUserAndSessionState` |

**所以别再问"它到底干了啥"了——它是你家那个"把线接好、流程排好、做决定前先问你"的管家。** 你家（游戏）的事它一件不碰，但没有它，这些线全要你自己一根一根接。
