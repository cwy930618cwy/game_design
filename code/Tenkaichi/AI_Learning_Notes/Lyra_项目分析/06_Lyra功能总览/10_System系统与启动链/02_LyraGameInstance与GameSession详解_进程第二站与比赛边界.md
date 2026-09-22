# 02 — `LyraGameInstance` + `LyraGameSession` 详解：进程第二站与"一局"的边界

> **定位**：上一篇讲了进程第一站 `LyraGameEngine`（总裁）。这一篇讲它的两位"中层"：
> - **`ULyraGameInstance`** = 进程第二站、"总经理办公室"（跨关卡的全局状态）
> - **`ALyraGameSession`** = "一局比赛的边界"（开始/结束/登录的官方钩子）
>
> **一句话**：`GameEngine` 管进程，`GameInstance` 管"这个进程里所有局"，`GameSession` 管"正在打的这一局"。

---

## 一、先放位置：两人在启动链里的分工

```
 进程 → ULyraGameEngine::Init（总裁，01 篇）
           │
           ▼
      ULyraGameInstance（总经理，本文主角①）
       ├─ Init()：注册全项目组件状态机 + 订阅会话事件
       ├─ 玩家账号初始化成功 → 让本地玩家读设置
       └─ 进服前：处理跳转/加密
           │
           ▼
      每局比赛开始/结束（边界事件）
      ALyraGameSession（本文主角②，比赛裁判）
       └─ 自动登录被 GameMode 接管（这里只剩钩子）
```

继承链先看清：

```
 ULyraGameInstance : UCommonGameInstance : UGameInstance   ← 引擎自带的"游戏实例"
   （Lyra版）      （CommonGame插件 提供账号/UI支持）         （跨图状态保存处）

 ALyraGameSession : AGameSession : AInfo                   ← 引擎自带的"比赛会话"
   （Lyra版）        （引擎级，处理登录/开始/结束）
```

---

## 二、`ULyraGameInstance`：总经理在干 4 件事

### ① `Init()`（LyraGameInstance.cpp L88-115）——最值得看的一个函数

```cpp
void ULyraGameInstance::Init()
{
	Super::Init();   // 先跑引擎默认初始化

	// 注册"组件初始化状态机"的 4 个阶段 —— ★关键
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);
	ComponentManager->RegisterInitState(LyraGameplayTags::InitState_Spawned,          false, FGameplayTag());
	ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataAvailable,    false, LyraGameplayTags::InitState_Spawned);
	ComponentManager->RegisterInitState(LyraGameplayTags::InitState_DataInitialized,  false, LyraGameplayTags::InitState_DataAvailable);
	ComponentManager->RegisterInitState(LyraGameplayTags::InitState_GameplayReady,    false, LyraGameplayTags::InitState_DataInitialized);

	// 填一个"演示用"加密 key（256字节，非安全）
	...
	// 订阅"客户端跳转进服务器前"事件
	SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
}
```

**它干了三件事**：

| 做的事 | 解释 | 跟你的关系 |
|---|---|---|
| **注册 InitState 四阶段** | 通过 `UGameFrameworkComponentManager` 注册 `Spawned → DataAvailable → DataInitialized → GameplayReady` 这套组件初始化状态机 | 这就是 Lyra 全部组件/Pawn"按顺序初始化"的总开关源（`08` 章讲过的 InitState） |
| 准备演示加密 key | 填死一个 AES key 数组 | 仅演示，正式版要走 HTTPS 取真实 key |
| 订阅"进服前"事件 | 从 CommonSession 收 `OnPreClientTravelEvent` | 进服务器前能改 URL（加加密 token 等） |

> 💡 **场景记忆**：`Init()` = 总经理上任第一天要做的三件事：**① 把全公司的规章制度（组件状态机）在公告栏登记 ② 准备一把测试钥匙 ③ 让前台（SessionSubsystem）进会议室前先通知我**。

### ② `HandlerUserInitialized`（L143-158）——登录成功后的动作

```cpp
// 账号登录流程结束会回调这里
void ULyraGameInstance::HandlerUserInitialized(... bSuccess ...)
{
	Super::...;   // 先让引擎处理
	if (bSuccess && UserInfo)   // 如果登录成功了
	{
		ULyraLocalPlayer* LocalPlayer = ...;   // 找到本地玩家
		if (LocalPlayer)
		{
			LocalPlayer->LoadSharedSettingsFromDisk();   // 让他读硬盘上的设置
		}
	}
}
```

> 💡 **场景**：玩家在平台登录成功 → 立刻把他的画质/音量设置从本地读出来 → UI 显示的就是他上次存的配置。

### ③ 网络加密两个回调（L160-311）——"演示版加密应答"

`ReceivedNetworkEncryptionToken` / `ReceivedNetworkEncryptionAck` 是引擎在网络握手时调的。Lyra 的版本写得**故意很简单**：token 合法就直接回一个写死的 key（AES / 可选 DTLS）。注释反复强调：**这只是演示，正式游戏要在这里接 HTTPS 安全服务取 key**。

> ⚠️ 这个文件里最明显的"教学代码"气息：CVar `Lyra.TestEncryption` 一开，客户端就带着加密 token 进服——**只用于演示加密流程，千万别抄进正式项目**。

### ④ `GetPrimaryPlayerController` / `CanJoinRequestedSession`

- `GetPrimaryPlayerController()`：返回强转成 `ALyraPlayerController` 的主玩家控制器（别处频繁用，见下）。
- `CanJoinRequestedSession()`：目前永远 `true`（注释："以后要检查玩家状态"）——又一个预留口。

---

## 三、`ALyraGameSession`：一个"名存实亡"的裁判

```cpp
bool ALyraGameSession::ProcessAutoLogin()
{
	// 实际上自动登录已在 LyraGameMode::TryDedicatedServerLogin 里处理了
	return true;    // 直接把引擎默认的自动登录"关掉"
}

void ALyraGameSession::HandleMatchHasStarted() { Super::HandleMatchHasStarted(); }   // 空钩子
void ALyraGameSession::HandleMatchHasEnded()   { Super::HandleMatchHasEnded();   }   // 空钩子
```

**讲清楚它为什么这么空**：UE 引擎的 `AGameSession` 默认会处理"服务器自动登录、比赛开始/结束"这些事。但 Lyra 的玩法启动实际是走 **Experience（体验）+ GameMode** 那一套更现代的流程，所以：

- **自动登录** → 引擎默认行为被 `return true` 关掉，真正登录在 `LyraGameMode::TryDedicatedServerLogin`（GameModes 目录）做；
- **比赛开始/结束钩子** → 保留空 override，**以后想在一局开始/结束时插逻辑，就往这里写**。

> 💡 **场景记忆**：`GameSession` 像是会场里"带工牌的工作人员"——Lyra 告诉它"你不用管签到（自动登录）了，有人替你干"，但把"开场/散场"的喇叭留给它：以后想统一处理"开局清场、结算写榜"，就在这里 override。

---

## 四、一个图理清三个"谁管谁"

```
  ULyraGameEngine（进程总裁）
        │  每进程 1 个，管 World/视口/Tick
        ▼
  ULyraGameInstance（总经理）────────── 跨"局"的状态都在这：
        │  每进程 1 个               · 玩家账号设置
        │   · 注册组件状态机         · 会话事件订阅
        │   · 登录后加载设置         · 进服前改 URL
        ▼
  ALyraGameSession（当值裁判）───────── 单"局"边界：
        每局 1 个
         · 自动登录(已交给 GameMode)
         · 比赛开始/结束钩子(预留)
```

> 区分口诀：**GameEngine = 进程级；GameInstance = 跨局级；GameSession = 局级**。换图不换状态数据 → GameInstance；开局/终局做点事 → GameSession 的钩子（或干脆 GameMode）。

---

## 五、本篇一句话

`LyraGameInstance` 是进程第二站的真主角：`Init()` 里注册了全项目组件状态机（`Spawned→GameplayReady`）、订阅会话事件、登录成功加载玩家设置，还留了一堆"演示加密 + 预留判断"的代码；`LyraGameSession` 则因为 Lyra 的启动交给了 Experience/GameMode，基本只剩"自动登录已被接管 + 开局终局钩子"的空壳。**下一站往 `GameModes` 目录走**——看看真正开局的 GameMode / Experience 是怎么把总经理吩咐的事干完的。
