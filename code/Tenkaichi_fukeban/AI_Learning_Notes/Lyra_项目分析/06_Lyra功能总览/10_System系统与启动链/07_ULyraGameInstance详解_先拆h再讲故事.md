# 07 — `ULyraGameInstance` 详解：先逐行拆 `.h`，再讲故事配代码

> **定位**：`UGameInstance`（06）给容器，`UCommonGameInstance`（03~05）当管家，这篇到 **`ULyraGameInstance`**——Lyra 自家经理（`.h` 只有 43 行）。
>
> **顺序**：先别急着看它"干嘛"，先把 43 行 `.h` 每个成员认一遍（这是读一切 UE 类的固定动作），再用故事把它们串起来。

---

## 〇、它在链条上的位置

```
 UGameInstance（引擎：跨图容器，06 篇）
   ▲
 UCommonGameInstance（CommonGame：接线管家，03~05）
   ▲
 ULyraGameInstance（本篇：Lyra 自家经理）
   ▲
 B_LyraGameInstance（蓝图，DefaultEngine.ini 最终指定）
```

---

## 一、先逐行拆 `.h`（43 行全解剖）

```
 LyraGameInstance.h（43行）                    │ 这一行是干嘛的
 ══════════════════════════════════════════════┼════════════════════
 L5  #include "CommonGameInstance.h"          │ 拉进"爹"的定义(见03)
 L7  #include "LyraGameInstance.generated.h"   │ UHT 生成物，必须最后
 L9  #define UE_API LYRAGAME_API               │ 导出宏(其他模块能用到)
 L14 UCLASS(MinimalAPI, Config = Game)         │ 声明这是个 UCLASS
 L15 class ULyraGameInstance : public UCommonGameInstance  │ 爹是谁
 L21 ULyraGameInstance(构造)                    │ 空构造（.cpp里空）
 │                                             │
 │ ── public（别人能调）──                      │
 L23 GetPrimaryPlayerController() const        │ ① 新增：便捷拿主控制器
 L25 CanJoinRequestedSession() override        │ ② 覆写爹的口子：能否进房
 L26 HandlerUserInitialized(...) override      │ ③ 覆写爹的口子：登录完成
 L28 ReceivedNetworkEncryptionToken(...)override│ ④ 覆写：服务端下发加密令牌
 L29 ReceivedNetworkEncryptionAck(...) override │ ⑤ 覆写：加密确认
 │                                             │
 │ ── protected（自己/子类用）──                │
 L33 Init() override                           │ ⑥ 覆写引擎钩子：启动初始化
 L34 Shutdown() override                       │ ⑦ 覆写引擎钩子：关闭清理
 L36 OnPreClientTravelToSession(URL)           │ ⑧ 新增私有：进服前改 URL
 L39 DebugTestEncryptionKey                    │ ⑨ 私有：演示加密 key
 ══════════════════════════════════════════════┴════════════════════
```

**认 .h 的关键动作：分清"谁留下的"**——这个类里每个成员只有三种来源：

| 标记 | 来源 | 本文件例子 |
|---|---|---|
| `override` + 名字在父类 | **爹/引擎留的口子**，Lyra 来填 | Init、HandlerUserInitialized… |
| 不带 override、也没在父类 | **Lyra 自己新增**的工具/数据 | GetPrimaryPlayerController、DebugTestEncryptionKey |
| `#include` 拉来的 | 依赖的"亲戚" | CommonGameInstance.h |

> 于是 43 行里真正的"新东西"只有两个：`GetPrimaryPlayerController()` 和 `DebugTestEncryptionKey`，其余**全是填别人留的口子**。读 .h 抓到这条线，就不乱了。

---

## 二、故事：哥哥的次世代版《剑与魔法》（全程对照代码）

哥哥的新游戏基于 Lyra。他写这个 43 行的类时，只做了下面这几件事——每件都能在代码里找到。

### 现场 ① 上任第一天：立"全公司成长规矩"（对应 L33 `Init()` override）

游戏启动，经理第一件事：把全 Lyra 所有组件的"成长阶段"登记成规矩，大家按顺序发育：

```cpp
void ULyraGameInstance::Init()
{
	Super::Init();   // 先让爹把它的线接好
	// Lyra 自家加的：注册"组件初始化状态机"4 个阶段
	UGameFrameworkComponentManager* CM = GetSubsystem<UGameFrameworkComponentManager>(this);
	CM->RegisterInitState(InitState_Spawned,         false, FGameplayTag());
	CM->RegisterInitState(InitState_DataAvailable,   false, InitState_Spawned);
	CM->RegisterInitState(InitState_DataInitialized, false, InitState_DataAvailable);
	CM->RegisterInitState(InitState_GameplayReady,   false, InitState_DataInitialized);
}
```

> 对应 .h：L33 那个 `Init() override`。**"出生→拿资料→初始化→上岗"四阶段**就是 Lyra 组件不乱序的根基。

### 现场 ② 玩家登录成功：喊人读设置（对应 L26 override）

```cpp
void ULyraGameInstance::HandlerUserInitialized(...)
{
	Super::HandlerUserInitialized(...);     // 先让爹处理
	if (bSuccess && UserInfo)                // 登录真的成功了
		if (ULyraLocalPlayer* LP = Cast<...>(GetLocalPlayerByIndex(...)))
			LP->LoadSharedSettingsFromDisk();   // 读上次保存的设置
}
```

> 对应 .h：L26。爹把"登录完成"的电话接通，**接起来说什么由 Lyra 决定**：先读设置。

### 现场 ③ 好友邀请进房：先放行（对应 L25 override）

```cpp
bool ULyraGameInstance::CanJoinRequestedSession() const
{
	return Super::CanJoinRequestedSession();   // 现在恒 true，先放行
}
```

> 对应 .h：L25。爹把流程排好，问"能不能进"→ Lyra 暂时全批；以后想加"战斗中不能切房"，就改这一个函数。

### 现场 ④ 演示加密通道（对应 L28/29/36 + L39）

- L36 `OnPreClientTravelToSession`：进服前，若开关 `Lyra.TestEncryption=1` → 给 URL 塞 `?EncryptionToken=1`；
- L28/29 两个回调：服务器要加密 → 拿 L39 那个写死的 `DebugTestEncryptionKey` 回 Success；
- `.cpp` 注释原话：**NOT SECURE, do not use in production!**——全是把"加密流程"跑通的演示。

> 对应 .h：L28/29/36/39。经理的"测试小工具"，正式上线要换 HTTPS 取 key。

### 现场 ⑤ 便利小方法（对应 L23）

```cpp
ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}
```

> 对应 .h：L23。唯一一个"全新工具"：把主控制器强转成 Lyra 版，方便别处直接调 Lyra 接口。

---

## 三、收束：故事 ↔ 代码 ↔ .h 行号

| 故事 | 干了啥 | .h 行号 | 来源 |
|---|---|---|---|
| 立成长规矩 | 注册组件状态机 4 阶段 | L33 `Init` | 填引擎口子 |
| 登录后读设置 | `LoadSharedSettingsFromDisk` | L26 `HandlerUserInitialized` | 填爹口子 |
| 能不能进房 | 暂时放行 | L25 `CanJoinRequestedSession` | 填爹口子 |
| 演示加密 | 塞 token / 回 key | L28/29/36/39 | 填引擎口子+自加数据 |
| 便利方法 | 强转主控制器 | L23 | **Lyra 自加** |

**一句话**：`ULyraGameInstance` 43 行里 90% 是 **override（填别人留的口子）**，真正自加的只有取主控制器和一个演示 key。它存在的意义就是："容器引擎给、流程管家排，**Lyra 自家那点事（立规矩、读设置、进房规则）精准填在口子上**"——你读任何 Lyra 类都用这个套路：**先拆 .h 分清三种来源，再想每个口子填了什么。**
