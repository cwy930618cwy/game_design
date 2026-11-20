# 08 — `LyraGameInstance.cpp` 全解：用"上线第一周值班日志"串起 339 行

> **定位**：`07` 讲了 `.h`（43 行清单）。这篇讲它的 `.cpp`（339 行）——同一个类，另一半。**`.h` 是"菜单"，`.cpp` 是"后厨"**，339 行就是每个方法的后厨做法 + 顶部一大段"备料"。
>
> **先给全图：339 行其实就两大段**

```
 LyraGameInstance.cpp（339行）地图
 ═══════════════════════════════════════════════════════════════
 段① L23-81   顶部"备料"（namespace Lyra 工具箱）
               → 演示开关 CVar ×3 + 生成证书命令（占 60 行！）
 段② L83-339  各种方法的"做法"
   L83-86   构造（空）
   L88-115  Init          ★核心
   L117-125 Shutdown
   L127-141 GetPrimaryPlayerController / CanJoinRequestedSession
   L143-158 HandlerUserInitialized（登录）
   L160-311 网络加密两个回调 ★代码最长（被 DTLS 分支撑起来的）
   L313-339 OnPreClientTravelToSession（进服前）
 ═══════════════════════════════════════════════════════════════
 一眼结论：真正"天天跑"的逻辑只有 150 行左右，
           剩下 180 行全是"加密演示 + 测试工具"。
```

---

## 故事：哥哥的游戏上线第一周（值班日志 × 6）

### 日志 ①（周一）—— 打开控制台，看到一堆"工具开关"（L23-81）

哥哥的《剑与魔法》上线了。他打开游戏按 `~` 敲命令，发现自带一套**测试工具箱**（就是文件最上面那个 `namespace Lyra`）：

```cpp
static FAutoConsoleVariableRef CVarLyraTestEncryption(
	TEXT("Lyra.TestEncryption"), bTestEncryption,
	TEXT("If true, clients will send an encryption token... NOT SECURE..."));
// 命令： Lyra.TestEncryption 1   → 客户端进服带加密token
// 命令： Lyra.UseDTLSEncryption 1 → 改用 DTLS 证书加密
// 命令： GenerateDTLSCertificate MyCert → 生成测试证书(非Shipping才有)
```

> **讲人话**：这 60 行全是"演示/测试用的开关"，不是游戏逻辑。以后你在别人项目里看到这种 `FAutoConsoleVariableRef` 大段，**先划到"测试工具"，别当核心逻辑读**。

### 日志 ②（周二凌晨）—— 服务器开机，经理上班（`Init`，L88-115）

服务器拉起来，引擎调 `Init()`。哥哥的经理做了三件事（代码原样）：

```cpp
void ULyraGameInstance::Init()
{
	Super::Init();   // 先让爹接线

	// ① 立规矩：注册组件状态机 4 阶段（每个组件按此顺序发育）
	UGameFrameworkComponentManager* CM = GetSubsystem<UGameFrameworkComponentManager>(this);
	CM->RegisterInitState(LyraGameplayTags::InitState_Spawned,         false, FGameplayTag());
	CM->RegisterInitState(LyraGameplayTags::InitState_DataAvailable,   false, InitState_Spawned);
	CM->RegisterInitState(LyraGameplayTags::InitState_DataInitialized, false, InitState_DataAvailable);
	CM->RegisterInitState(LyraGameplayTags::InitState_GameplayReady,   false, InitState_DataInitialized);

	// ② 填一个 32 字节"演示 key"：DebugTestEncryptionKey[i] = i
	DebugTestEncryptionKey.SetNum(32);
	for (int32 i = 0; i < 32; ++i) DebugTestEncryptionKey[i] = uint8(i);

	// ③ 订阅"客户端跳转进服前"事件 → 进服前我会被喊到
	GetSubsystem<UCommonSessionSubsystem>()
		->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
}
```

> 与 `.h` 对上：就是 `Init()` override 里填的东西。③ 这行订阅，是后面日志⑤能用上的"提前预约"。

### 日志 ③（周三晚）—— 服务器维护，经理下班（`Shutdown`，L117-125）

```cpp
void ULyraGameInstance::Shutdown()
{
	if (auto* SS = GetSubsystem<UCommonSessionSubsystem>())
		SS->OnPreClientTravelEvent.RemoveAll(this);   // 把日志②的预约退掉
	Super::Shutdown();
}
```

> 守规矩的收尾：**在 Init 订了什么，在 Shutdown 就退什么**（否则对象死了事件还挂着，会崩）。这是 UE 生命周期的黄金习惯。

### 日志 ④（周四）—— 玩家小明登录（L143-158 + L127-141）

小明输账号登录成功 → 爹的电话转给经理：

```cpp
void ULyraGameInstance::HandlerUserInitialized(...)
{
	Super::HandlerUserInitialized(...);
	if (bSuccess && ensure(UserInfo))    // 登录真的成功
		if (ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex)))
			LP->LoadSharedSettingsFromDisk();   // ★读上次的设置
}
```

经理还配了个"快速取人"小工具（`.h` L23 那个自加函数）：

```cpp
ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
	// 把引擎的主控制器强转成 Lyra 版，方便调用方直接用 Lyra 接口
}
```

> 小明一进游戏，画质/键位全是他上次调的——就是上面这两段配合的结果。

### 日志 ⑤（周五晚高峰）—— 好友拉小明进服（L313-339 + L160-311）

小明在游戏里收到好友邀请，客户端**要跳转进服务器**了。跳转前，经理被喊到（就是日志②订阅的那个事件）：

```cpp
void ULyraGameInstance::OnPreClientTravelToSession(FString& URL)   // L313
{
	if (Lyra::bTestEncryption)              // 如果开了测试加密开关
	{
		URL += TEXT("?EncryptionToken=1");   // 往跳转地址上塞一个 token
	}
}
```

服务器那头收到连接，要求加密，客户端引擎回调经理的两个"加密应答函数"（L160-311）。**这里有一段现实**：这两个函数加起来 150 行，但去掉 `#if UE_WITH_DTLS` 那段后只剩 20 行真逻辑：

```cpp
void ULyraGameInstance::ReceivedNetworkEncryptionToken(...)   // L160
{
	FEncryptionKeyResponse Response(EEncryptionResponse::Failure, ...);
	if (EncryptionToken.IsEmpty())          // token 是空的 → 拒绝
		Response.Response = EEncryptionResponse::InvalidToken;
	else {
#if UE_WITH_DTLS
		if (Lyra::bUseDTLSEncryption) { ...证书那一大套演示... }
		else
#endif
		{                                   // ★不带 DTLS 的真逻辑就这 3 行：
			Response.Response = EEncryptionResponse::Success;
			Response.EncryptionData.Key = DebugTestEncryptionKey;  // 回写死key
		}
	}
	Delegate.ExecuteIfBound(Response);      // 把结果交回引擎
}
```

> **读 .cpp 的偷懒技巧（很重要）**：看到 `#if UE_WITH_DTLS` 这种"条件编译大括号"——**先跳过去**。DTLS 是另一套加密演示（要证书/指纹/读写文件），正式游戏走普通 AES 分支就够了。注释里都写着 **"NOT SECURE, demonstration only"**。

### 日志 ⑥（周末）—— 玩家问"能不能现在进房"（L132-141）

```cpp
bool ULyraGameInstance::CanJoinRequestedSession() const
{
	if (!Super::CanJoinRequestedSession()) return false;  // 爹说不行就不行
	return true;                                          // 目前先放行
	// 注释：以后要检查玩家状态
}
```

> 经理暂时"全批"，想加规则（战斗中不能切）就在这一个函数改。

---

## 收束：339 行到底讲了啥

```
 段① L23-81  （60行）工具箱：CVar 开关 + 生成证书命令  → 测试用，别当核心
 段② 其余    （279行）8 个方法的实现
   真正天天跑的：Init 立规矩 / Shutdown 退订 /
                登录读设置 / 能否进房
   演示居多  ：两个"加密应答"回调（150行，DTLS撑的）
```

**故事 ↔ 代码段落对照**：

| 故事 | .cpp 位置 | 一句话 |
|---|---|---|
| 看工具箱 | L23-81 | CVar 开关、证书命令，演示专用 |
| 服务器开机 | L88-115 | `Init`：立规矩+填key+订阅进服事件 |
| 服务器维护 | L117-125 | `Shutdown`：退订（对称收尾） |
| 小明登录 | L143-158 | 登录成功→读玩家设置 |
| 好友拉进服 | L313-339 / L160-311 | 跳转前塞token；加密回调应答key |
| 能进房吗 | L132-141 | 暂时全放行 |

**本篇一句话**：这个 `.cpp` 看似 339 行很唬人，**结构只有"一段测试工具箱 + 8 个方法实现"**；其中加密两个回调又占了一半篇幅（还是 DTLS 演示撑的）。读它的正确姿势：**Init 和登录是重点，碰到 `#if UE_WITH_DTLS` 直接跳过，看到 "NOT SECURE / demonstration" 注释就明白是演示代码**——这样 339 行真正要消化的大概只有 60 行。
