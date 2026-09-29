# 06 — `UGameInstance` 详解：引擎里那个"跨地图不死的容器"

> **定位**：`03~05` 一直在讲 Lyra 的 `ULyraGameInstance` 和它爹 `UCommonGameInstance`。这一篇讲到根上：引擎的 **`UGameInstance` 本身**是什么、源码里长什么样、为什么它是"跨图状态"的最佳存放点。
>
> **本篇一句话**：它是引擎给每个游戏进程准备的**"高层总管对象"**——进程创建它、进程关闭才销毁，**地图换来换去它都不死**，所以跨关卡的玩家/会话/全局数据都该挂它身上。

---

## 一、引擎源码怎么定义它（先看官方注释原话）

`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Engine\Classes\Engine\GameInstance.h`

```cpp
/**
 * GameInstance: high-level manager object for an instance of the running game.
 * Spawned at game creation and not destroyed until game instance is shut down.
 * Running as a standalone game, there will be one of these.
 * Running in PIE (play-in-editor) will generate one of these per PIE instance.
 */
UCLASS(config=Game, transient, BlueprintType, Blueprintable, MinimalAPI)
class UGameInstance : public UObject, public FExec
{
	...
};
```

**把注释翻译成人话**：

| 注释原文 | 意思 |
|---|---|
| high-level manager object | 它是"高层总管对象"（比 World/Actor 高一层） |
| Spawned at game creation... not destroyed until shut down | **进程一创建它就在，进程关了它才死** |
| standalone game → one of these | 独立运行：一个进程一个 |
| PIE → one per PIE instance | 编辑器里跑 N 个 PIE 就各有一个（互不干扰） |

> 💡 **场景**：你在主菜单选了画质 → 进游戏地图 → 死亡回菜单 → 再进另一张图。整条过程 `UGameInstance` 一直活着，所以**"玩家上次选了什么设置/账号是谁"**能一路带过来——这就是它存在的最大意义。

---

## 二、它源码里的"家底"（真实成员）

```
 class UGameInstance : public UObject
 {                                │ 内部装了这些"家当"：
   WorldContext;                  │ 关联的 World 上下文（找当前世界用）
   LocalPlayers;                  │ ★本地玩家列表（TArray<ULocalPlayer*>）
   OnlineSession;                 │ 联机会话对象（做联机要用的）
   NotifyPreClientTravelDelegates │ ★"跳转服务器前"通知（可广播给听众）
   OnPawnControllerChangedDelegates│ ★Pawn 换了 Controller 的通知
   ...（输入设备连接变化等委托）
 };
```

几个要记住的点：
- **`LocalPlayers`**：游戏里所有本地玩家（支持分屏/多手柄）都装在这个数组——`CommonGameInstance` 里 `AddLocalPlayer` 覆写的就是在管它。
- **一堆 `...Delegates`（委托）**：它自己也会"广播事件"给别人听——比如 `PreClientTravel`（进服前）。还记得 `ULyraGameInstance` 的 `OnPreClientTravelToSession` 吗？Lyra 就是订阅了这类引擎/子系统事件。

---

## 三、它的"生命周期钩子"（源码里的 virtual）

引擎给自定义子类留了几个口子（真实声明）：

```cpp
ENGINE_API virtual void Init();          // 进程启动时调用 → 初始化你自己的东西
ENGINE_API virtual void Shutdown();      // 进程关闭前调用 → 清理
UFUNCTION(BlueprintImplementableEvent) void ReceiveInit();     // 蓝图版 Init
UFUNCTION(BlueprintImplementableEvent) void ReceiveShutdown(); // 蓝图版 Shutdown
```

注意那个 `ReceiveInit`：`BlueprintImplementableEvent` = **"这个函数让蓝图实现"**。也就是说：

```
 C++ 子类：覆写 Init()          ← 喜欢 C++ 就写这
 蓝图子类：实现 ReceiveInit()   ← 喜欢蓝图就写这（引擎会自动调）
```

> 💡 **场景**：你要"游戏启动时自动尝试登录平台账号"——C++ 项目在 `Init()` 里调，蓝图项目在 `ReceiveInit()` 事件里连线即可。这是同一件事的两扇门。

---

## 四、它跟谁搭配用（一张图定边界）

```
 UGameEngine（进程总裁，01篇）
   │  进程级：World/视口/Tick
   ▼ 创建
 UGameInstance（本课：跨图容器）──> 放着：LocalPlayers / OnlineSession / 全局数据
   │  每进一张图 ↓ 创建
   ▼
 UWorld / UGameMode（当前这局的规则）
   │  每局里每个玩家 ↓ 创建
   ▼
 APlayerController → APawn（具体单位）
```

| 对象 | 生命周期 | 典型归属 |
|---|---|---|
| `UGameEngine` | 进程 = 最长的 | 引擎底层（总裁） |
| `UGameInstance` | 进程级、跨图不死 | **跨图数据 / 会话 / 玩家列表** |
| `UGameMode` | 一张图 = 换图就换 | 这局规则（得分/出生） |
| `APlayerController` | 一个玩家一场 | 玩家输入/视角 |
| `APawn` | 一个身体 | 角色的移动/血量 |

> 判断口诀：**数据要"换了地图还在"→ 放 GameInstance；只在这一张图有效 → 放 GameMode/World。**

---

## 五、引擎怎么决定"用哪个 GameInstance 子类"

不是硬编码，而是配置指定（Lyra 里真实配置）：

```ini
; Config/DefaultEngine.ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Game/B_LyraGameInstance.B_LyraGameInstance_C
```

这里指到的是一个**蓝图** `B_LyraGameInstance`。所以 Lyra 的完整链条是：

```
 UGameInstance（引擎 C++）
   ▲
 UCommonGameInstance（CommonGame 插件，05 篇）
   ▲
 ULyraGameInstance（LyraGame/System，02 篇）
   ▲
 B_LyraGameInstance（蓝图！DefaultEngine.ini 里最终指定的类）
```

> 关键认知：**你不一定要写 C++ 子类**——`UGameInstance` 标了 `Blueprintable`，纯蓝图项目直接在 `GameInstanceClass` 里填你的蓝图实例类就能用。Lyra 是"C++ 管逻辑 + 蓝图当配置"的双层做法。

---

## 六、本篇一句话

`UGameInstance` = 引擎给的**跨图不死的"高层总管对象"**：进程创建它、关了才销毁；内部装着本地玩家列表、联机会话、一堆可广播的事件；给你留了 `Init/Shutdown`（C++）和 `ReceiveInit/ReceiveShutdown`（蓝图）两套口子。**要换哪个子类由 `GameMapsSettings.GameInstanceClass` 指定**。Lyra 的整条链 `UGameInstance → UCommonGameInstance → ULyraGameInstance → B_LyraGameInstance`，每一层都在往这个"容器"里加自己那一层的职责。

---

## 七、那为什么不直接用 `UGameInstance`，还要叠 `UCommonGameInstance`？—— 讲个双胞胎工作室的故事

> 你可能会想：`UGameInstance` 都这么强了（跨图不死、还能装数据），Epic 干嘛再多写一层 `UCommonGameInstance`？不是多此一举吗？
> 要回答这个，先承认一个前提：**`UGameInstance` 强，但它是"空"的强**。

### 故事开始：老王家双胞胎兄弟

王家有两兄弟，哥哥做**《剑与魔法》**（奇幻 RPG），弟弟做**《星际枪神》**（太空射击）。两款游戏八竿子打不着，但都遇到了同一堆麻烦：

1. 玩家要登录平台账号 → 得等账号子系统回来说"登录完了"，再读他的设置；
2. 玩家插手柄/拔手柄 → UI 要跟着变；
3. 好友发来"加入房间"邀请 → 得处理"现在能不能进、不能进怎么办"；
4. 玩家打完一局要回主菜单 → 得把登录态和旧房间清干净。

第一版，兄弟俩各自在自己的 `UGameInstance` 子类里**手写**这些。哥哥的代码长这样：

```cpp
// 哥哥《剑与魔法》第一版：UMyRPG_GameInstance : public UGameInstance
void UMyRPG_GameInstance::Init()
{
	Super::Init();
	// 【麻烦1】手动绑"账号登录完成"事件
	GetSubsystem<UCommonUserSubsystem>()->OnUserInitializeComplete.AddDynamic(
		this, &UMyRPG_GameInstance::Handle_MyLoginDone);
	// 【麻烦3】手动绑"好友邀请进房"事件
	GetSubsystem<UCommonSessionSubsystem>()->OnUserRequestedSessionEvent.AddUObject(
		this, &UMyRPG_GameInstance::Handle_MyInvite);
	// 【麻烦2】手动绑手柄增减事件...
}

void UMyRPG_GameInstance::AddLocalPlayer(ULocalPlayer* P, FPlatformUserId Uid)
{
	Super::AddLocalPlayer(P, Uid);
	// 【麻烦2】手动通知自己的 UI 管理器"来了个玩家"...
}
```

弟弟的《星际枪神》……**原封不动复制粘贴了这 80 行**，只改类名。俩人第一版都能跑，就是觉得哪里不对：

> "凭什么每个游戏都要重复写'登录完等回调、进房前先判断、玩家增减通知 UI'？这三件事跟**剑和魔法**还是**星际枪神**有关系吗？——完全没有！是**任何要联机、要 UI 的游戏**都得处理的公共活。"

### 于是他们把"公共活"抽了出来

兄弟俩把这堆重复代码整理成一包，取名 **CommonGame 插件**（就是 Lyra/Plugins 里那个），基类叫 `UCommonGameInstance`。哥哥的第二版变成：

```cpp
// 哥哥《剑与魔法》第二版：UMyRPG_GameInstance : public UCommonGameInstance
// Init() 里那些"手动接线"全没了——爹在 05 篇那个 Init() 里已经接好线了！

void UMyRPG_GameInstance::HandlerUserInitialized(...) override
{
	// 爹只留了这一个口子：登录完成后我自家要干嘛？→ 读玩家设置
	LoadMySettings();
}
```

**代码对比一看就懂**：

| | 手写版（没 CommonGame） | 继承版（有 CommonGame） |
|---|---|---|
| 登录接线 | 每次自己 `AddDynamic` 绑一遍 | 爹在 `Init()` 里绑好（05 篇代码） |
| 玩家增减通知 UI | 每次自己写 | 爹在 `AddLocalPlayer` 里写好 |
| 好友邀请进房 | 每次自己写状态机 | 爹提供 `SetRequestedSession` 全家（05 篇时序） |
| 留给你的口子 | 无 | `HandlerUserInitialized` 等虚函数等你填 |
| 你还要写的 | **一堆基础设施** | **只有自家逻辑**（读设置/队伍/存档） |

### 回到引擎视角：为什么引擎不直接内置这些？

因为引擎的 `UGameInstance` 要**对全世界所有游戏通用**——它不知道你是手游还是主机、要不要平台账号、UI 怎么做。所以它只保证"容器 + 生命周期"（前面 1~6 节那些），把"游戏行业里高频的那套账号/UI/会话模板"交给 Epic 自己的工程实践（CommonGame 插件）提供。**引擎给的是水泥地基，CommonGame 给的是通好的水电，Lyra 再摆自家家具。**

**所以最终答案**：
- 不是"`UCommonGameInstance` 比 `UGameInstance` 厉害"，而是 **"它替你省掉了每个联机游戏都要重复写的 80 行公共代码"**；
- `UGameInstance` 的跨图容器 + `UCommonGameInstance` 的账号/UI/会话模板 + Lyra 的自家逻辑 = **三层各司其职**；
- 你要是做一个**纯单机、无账号、无复杂 UI 流**的小游戏，直接用 `UGameInstance` 完全没问题——**什么时候你需要 CommonGame？当你的游戏要登录、要联机、要弹窗管理，开始手写"绑账号事件"那一刻，就说明你该继承它了。**
