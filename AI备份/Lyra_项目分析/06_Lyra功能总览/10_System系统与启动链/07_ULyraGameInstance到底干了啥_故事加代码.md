# 07 — `ULyraGameInstance` 到底干了啥（故事 + 代码）

> **定位**：链路我们已经走完 `UGameInstance（06）→ UCommonGameInstance（03~05）`，现在到 **`ULyraGameInstance`**——Lyra 自己在最上层填的"自家逻辑"。
>
> **本篇一句话**：爷爷 `UGameInstance` 给容器，爹 `UCommonGameInstance` 当管家（接线/流程/弹窗），**`ULyraGameInstance` = 管家之上再请的"Lyra 自家经理"**：它只补 Lyra 需要、管家不管的 5 件事。

---

## 〇、它在链条上的位置

```
 UGameInstance（引擎，06 篇：跨图容器）
   ▲
 UCommonGameInstance（CommonGame 插件，03~05：接线管家）
   ▲
 ULyraGameInstance（本篇：Lyra 自家经理）← .h 只有 43 行
   ▲
 B_LyraGameInstance（蓝图：DefaultEngine.ini 最终指定的类）
```

> 注意 `.h` 只有 **43 行**——比爹（CommonGame，83 行）短多了。**它越短，说明管家干的活越多。**

---

## 一、先看它的 .h 声明了啥（43 行全貌）

```
 LyraGameInstance.h（43行）                   │ 批注
 ══════════════════════════════════════════════┼════════════════
 #include "CommonGameInstance.h"              │ 爹在这
 UCLASS(MinimalAPI, Config=Game)
 class ULyraGameInstance : public UCommonGameInstance
 {
 public:
   GetPrimaryPlayerController() const         │ ① 便捷取主控制器
   CanJoinRequestedSession() override         │ ② 覆写：能否进房
   HandlerUserInitialized(...) override       │ ③ 覆写：登录完成
   ReceivedNetworkEncryptionToken/Ack override │ ④ 覆写：网络加密应答
 protected:
   Init() override                            │ ⑤ 覆写：启动初始化
   Shutdown() override                        │ ⑥ 覆写：关闭清理
   OnPreClientTravelToSession(URL)            │ ⑦ 私有：进服前改 URL
 private:
   TArray<uint8> DebugTestEncryptionKey       │ ⑧ 演示加密 key
 };
```

**一眼结论**：它声明的东西**几乎全是 override**——也就是"爹留的口子，我来填"。具体填了什么？用故事讲。

---

## 二、故事：哥哥的第二款游戏上线了（这次带 LyraGameInstance）

哥哥的第二款游戏（还是《剑与魔法》，但这次是次世代版）**基于 Lyra 做**。他请了个"自家经理" `ULyraGameInstance`。上线日的四件事：

### 现场 ①：上任第一天，立"全公司初始化规矩" —— `Init()`

游戏启动，经理上任第一件事：**把全公司（所有组件/Pawn）的"成长阶段"登记进系统**，让大家按统一顺序发育：

```cpp
void ULyraGameInstance::Init()   // LyraGameInstance.cpp
{
	Super::Init();   // 先让爹（CommonGame）把它的线接好

	// Lyra 自家加的：注册"组件初始化状态机"的 4 个阶段
	UGameFrameworkComponentManager* CM = GetSubsystem<UGameFrameworkComponentManager>(this);
	CM->RegisterInitState(LyraGameplayTags::InitState_Spawned,         false, FGameplayTag());
	CM->RegisterInitState(LyraGameplayTags::InitState_DataAvailable,   false, InitState_Spawned);
	CM->RegisterInitState(LyraGameplayTags::InitState_DataInitialized, false, InitState_DataAvailable);
	CM->RegisterInitState(LyraGameplayTags::InitState_GameplayReady,   false, InitState_DataInitialized);

	// 然后填一个"演示加密 key"（32字节），并订阅"进服前"事件
	SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ULyraGameInstance::OnPreClientTravelToSession);
}
```

> **故事视角**：这就像公司立规矩——**每个新员工（组件）都得按顺序过四个阶段**：出生(`Spawned`) → 拿到资料(`DataAvailable`) → 资料初始化完(`DataInitialized`) → 可以上岗(`GameplayReady`)。以后所有组件按这套顺序干活，不会出现"血条都绑定了、角色还没生成"的乱象。

### 现场 ②：玩家登录成功，经理喊人读设置 —— `HandlerUserInitialized`

玩家登录完成，爹（CommonGame）的电话响了，转给经理——经理**override 了这个口子**，安排自家玩家去读上次保存的设置：

```cpp
void ULyraGameInstance::HandlerUserInitialized(...) override
{
	Super::HandlerUserInitialized(...);   // 先让爹处理
	if (bSuccess && UserInfo)             // 登录真的成功了
	{
		ULyraLocalPlayer* LP = Cast<ULyraLocalPlayer>(GetLocalPlayerByIndex(...));
		if (LP)
			LP->LoadSharedSettingsFromDisk();   // ★Lyra 自家活：读设置
	}
}
```

> **故事视角**：管家（CommonGame）把电话接通了就退到一边——**"接起来说什么"由经理决定**。经理说：先让玩家把上次调好的键位/画质加载出来。

### 现场 ③：好友邀请进房，经理拍板"能不能现在进"

爹把进房流程排好了，但"能不能进"是虚函数口子，默认 `true`。经理想了想，先保持能进（等以后做"战斗中不能切房"再改）：

```cpp
bool ULyraGameInstance::CanJoinRequestedSession() const override
{
	// 暂时：先放行（注释说以后要检查玩家状态）
	return Super::CanJoinRequestedSession();   // 目前 == true
}
```

> **故事视角**：管家问经理"现在有人邀请他进房，批不批？"经理现在懒得管，说"先全批"——以后想加规则（比如"战斗中不行"）就在这一个函数里改。

### 现场 ④：演示加密（经理的"测试小工具"）

`.h` 里那三个加密相关（`ReceivedNetworkEncryptionToken/Ack` + `OnPreClientTravelToSession`）+ `DebugTestEncryptionKey`，是经理搭的**演示加密通道**：

```
 进服前（OnPreClientTravelToSession）→ 若开关 Lyra.TestEncryption=1
   → 往跳转 URL 后面塞 ?EncryptionToken=1
 服务器要求加密（ReceivedNetworkEncryptionToken）
   → 经理拿写死的 key 回复 Success（.cpp 里代码注释：NOT SECURE，仅演示）
```

> ⚠️ 经理自己都写了注释：**这是教学演示，正式项目要把 key 换成像 HTTPS 这样的安全渠道**——你在源码里看到一大堆加密代码，先别慌，全是"跑通流程"用的。

---

## 三、回到问题："那它到底干嘛了？"

```
 爷 UGameInstance     ：给个能跨图不死的"空房子"
 爹 UCommonGameInstance：把 账号/UI/会话 的水电接好（03~05）
 哥 ULyraGameInstance  ：（本课）只补 Lyra 自家的事：
     ① Init() 立规矩 → 注册组件初始化 4 阶段状态机
     ② 登录完成    → 让玩家读磁盘设置
     ③ 进房口子    → 先放行，将来加规则
     ④ 演示加密    → 跑通"进服加密"流程（NOT SECURE）
     ⑤ 便利方法    → GetPrimaryPlayerController 强转一下
```

**故事 ↔ 代码对照表**：

| 故事 | 代码 |
|---|---|
| 上任立规矩 | `Init()` 里 `RegisterInitState` ×4（组件状态机） |
| 登录后喊人读设置 | override `HandlerUserInitialized` → `LoadSharedSettingsFromDisk` |
| 管家问"能不能进房" | override `CanJoinRequestedSession`（暂 true） |
| 测试加密通道 | 3 个网络回调 + `DebugTestEncryptionKey` |

**它为什么这么短？** 因为它**想做的事就这些**——通用活让爹干，专属活自己填，填完收工。往下还有一层 `B_LyraGameInstance`（蓝图），专门放"美术/策划不想动 C++ 就改我"的配置。**每层都只写自己这一层该写的东西**——这就是 Lyra 分层的全部意义。

---

**本篇一句话**：`ULyraGameInstance` = Lyra 在"容器(引擎)+管家(CommonGame)"之上请的**自家经理**：启动时立组件初始化规矩、登录完读玩家设置、进房规则先放行、再搭一套演示加密。`.h` 只有 43 行不是它没活干，而是**该干的活都精准地填在了爹留下的口子上**。
