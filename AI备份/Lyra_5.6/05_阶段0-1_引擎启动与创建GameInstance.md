# 阶段 0-1：引擎启动 + 创建 GameInstance

> **定位**：把 [04_启动全流程](./04_从打开游戏那一刻_启动全流程.md) 的**阶段 0（进程启动）和阶段 1（创建 GameInstance）**单独展开细讲。这是"打开游戏"的最前面两步——引擎怎么起来、第一个全局对象怎么诞生。
>
> **一句话**：双击图标后，引擎先启动（读 `DefaultEngine.ini` 知道"用哪些类"），然后按配置创建 `ULyraGameInstance` 这个**全局单例**——它是整个游戏"跨关卡一直存在"的大管家，`Init()` 里注册了 Lyra 的**四阶段初始化状态机**（Spawned→DataAvailable→DataInitialized→GameplayReady）。

---

## 一、这两步在干什么（一句话）

```
阶段 0：引擎启动      → 读配置，知道"这个游戏用哪些类"
阶段 1：创建 GameInstance → 按配置 new 出全局单例，调它的 Init()
```

> **类比**：阶段 0 = 打开一家店，先看"店铺手册"（ini）知道店长是谁、用什么规则。阶段 1 = 把"店长"（GameInstance）请上岗，店长一上任就开始做开店准备（`Init()`）。

---

## 二、阶段 0：进程启动（引擎起来 + 读配置）

### 2.1 双击图标后

```
双击游戏图标
  → 操作系统加载游戏进程
  → 引擎(UE)启动：
     - 加载核心模块（Core/CoreUObject/Engine...）
     - 初始化渲染、RHI（图形接口）、物理、内存分配器...
     - 这一大坨是引擎的固定启动流程，所有 UE 游戏都一样
```

### 2.2 关键：读 DefaultEngine.ini（决定"用哪些类"）

引擎启动时会读 `Config/DefaultEngine.ini`，里面配了 Lyra 用哪些**自定义类**。看真实配置：

```ini
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/LyraGame.LyraGameViewportClient
AssetManagerClassName=/Script/LyraGame.LyraAssetManager      ← 资产管理器（第 01 篇讲过）
WorldSettingsClassName=/Script/LyraGame.LyraWorldSettings

[/Script/EngineSettings.GameMapsSettings]
GlobalDefaultGameMode=/Game/B_LyraGameMode.B_LyraGameMode_C   ← 默认 GameMode
GameInstanceClass=/Game/B_LyraGameInstance.B_LyraGameInstance_C  ← GameInstance（重点！）
GameDefaultMap=/Game/System/FrontEnd/Maps/L_LyraFrontEnd.L_LyraFrontEnd  ← 默认进主菜单地图
```

**这些配置的含义**：

| 配置项 | 值 | 含义 |
|--------|-----|------|
| `GameInstanceClass` | `B_LyraGameInstance` | 用哪个 GameInstance 类（蓝图，父类是 C++ 的 `ULyraGameInstance`） |
| `AssetManagerClassName` | `LyraAssetManager` | 用哪个资产管理器（第 01 篇讲过） |
| `GlobalDefaultGameMode` | `B_LyraGameMode` | 默认 GameMode |
| `GameDefaultMap` | `L_LyraFrontEnd` | 启动后默认进的地图（主菜单） |

> **要点**：引擎是"配置驱动"的——它本身不知道 Lyra 用什么类，全靠 ini 告诉它。这就是为什么 Lyra 能用自己的 GameInstance/GameMode/AssetManager 替换引擎默认的。

### 2.3 为什么 GameInstance 是蓝图（`B_` 前缀）？

注意 `GameInstanceClass=/Game/B_LyraGameInstance...`——这是个**蓝图资产**，不是直接 C++ 类。

```
B_LyraGameInstance（蓝图）
    └─ 父类：ULyraGameInstance（C++，真正的逻辑在这）
```

- 蓝图只是"壳"——可以在编辑器里指定一些默认值、配置。
- **真正的逻辑在 C++ 父类 `ULyraGameInstance`** 里（`Init()` 等）。
- 这是 UE 常见做法：C++ 写逻辑，蓝图做配置壳，方便策划/美术在编辑器调。

---

## 三、阶段 1：创建 GameInstance（全局单例诞生）

### 3.1 引擎按配置创建它

```
引擎读到 GameInstanceClass = B_LyraGameInstance
  → 用反射系统创建这个对象（NewObject，不是普通 new）
  → 调用它的构造函数 ULyraGameInstance()
  → 调用它的 Init()（初始化入口）
```

### 3.2 GameInstance 是什么？—— 跨关卡的全局大管家

```cpp
// LyraGameInstance.h
UCLASS(MinimalAPI, Config = Game)
class ULyraGameInstance : public UCommonGameInstance   // 继承 CommonGame 插件的 GameInstance
{
    GENERATED_BODY()
public:
    UE_API ULyraGameInstance(const FObjectInitializer& ObjectInitializer);

    UE_API ALyraPlayerController* GetPrimaryPlayerController() const;  // 拿主玩家控制器
    // ...
protected:
    UE_API virtual void Init() override;      // ← 初始化入口（重点）
    UE_API virtual void Shutdown() override;  // 销毁时
};
```

**GameInstance 的核心特性**：
- **全局单例**：整个游戏只有一个。
- **跨关卡不销毁**：玩家从 主菜单 → 游戏关卡 → 回主菜单，地图换了很多次，但 **GameInstance 始终是同一个**。
- **生命周期最长**：游戏一启动就创建，退出才销毁。

> **类比**：GameInstance 像"公司的 CEO"——员工（关卡/玩家）来来去去，部门（地图）换来换去，但 CEO 一直在。全局性的事（会话、登录状态、前端流程）都归它。

### 3.3 它继承自 `UCommonGameInstance`

```cpp
class ULyraGameInstance : public UCommonGameInstance
```

- `UCommonGameInstance` 来自引擎的 **CommonGame 插件**（之前讲 `SessionSubsystem` 时提过）。
- 它已经处理了"用户登录、会话请求、网络加密"等通用逻辑。
- Lyra 继承它，只加 Lyra 特有的东西。

看头文件里那些重写函数，就知道它管什么：
```cpp
virtual bool CanJoinRequestedSession() const override;        // 能否加入会话
virtual void HandlerUserInitialized(...) override;            // 用户登录完成处理
virtual void ReceivedNetworkEncryptionToken(...) override;    // 网络加密令牌
virtual void ReceivedNetworkEncryptionAck(...) override;      // 加密确认
```

> 这些都是"全局性、跨会话"的事——正是 GameInstance 该管的。

---

## 四、重点：`Init()` 里注册了初始化状态机 ★

`GameInstance` 一被创建，引擎就调它的 `Init()`。看真实实现：

```cpp
void ULyraGameInstance::Init()
{
    Super::Init();   // 先调父类（CommonGame 的初始化）

    // ★ 注册 Lyra 的四阶段初始化状态机
    UGameFrameworkComponentManager* ComponentManager =
        GetSubsystem<UGameFrameworkComponentManager>(this);

    if (ensure(ComponentManager))
    {
        // 注册 4 个初始化状态，串成一条链：
        ComponentManager->RegisterInitState(InitState_Spawned,         false, FGameplayTag());
        ComponentManager->RegisterInitState(InitState_DataAvailable,   false, InitState_Spawned);
        ComponentManager->RegisterInitState(InitState_DataInitialized, false, InitState_DataAvailable);
        ComponentManager->RegisterInitState(InitState_GameplayReady,   false, InitState_DataInitialized);
    }

    // 初始化调试加密密钥（AES256，32 字节，仅示例用，不安全）
    DebugTestEncryptionKey.SetNum(32);
    for (int32 i = 0; i < 32; ++i)
        DebugTestEncryptionKey[i] = uint8(i);

    // 绑定"客户端前往会话前"的回调
    if (auto* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
        SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &ThisClass::OnPreClientTravelToSession);
}
```

### 4.1 四阶段初始化状态机（呼应之前学的 InitState Tag）

这是 `Init()` 最重要的事——注册 Lyra 的**分阶段初始化状态机**：

```
Spawned（已生成）
   ↓
DataAvailable（数据可用）
   ↓
DataInitialized（数据已初始化）
   ↓
GameplayReady（游戏就绪）
```

**为什么需要它？**

- UE 的对象（Actor/Component）"被创建"和"真正准备好干活"是**两回事**。
- 一个组件可能 `Spawn` 出来了，但它依赖的数据还没加载、还没初始化完——这时不能让它干活。
- Lyra 用这 4 个状态，让各组件**按阶段推进**：只有到"数据就绪"，才进入"游戏就绪"，避免"东西还没准备好就被调用"的 bug。

> **呼应**：之前学 `LyraGameplayTags` 时讲过 `InitState.*` 这 4 个 Tag——就是这里注册的！配合"大管家"（GameFrameworkComponentManager）统一管理所有组件的初始化时机。

### 4.2 其他初始化

- **调试加密密钥**：初始化一个 32 字节的 AES256 密钥（就是之前 `namespace Lyra` 里那些 DTLS 加密 CVar 用的测试密钥，注释强调"不安全，仅示例"）。
- **会话回调**：绑定"客户端前往会话前"的事件（联机跳转地图前的处理）。

---

## 五、GameInstance 的生命周期图

```
【游戏启动】
  引擎读 ini → GameInstanceClass = B_LyraGameInstance
        ↓
  创建 GameInstance（构造函数）
        ↓
  Init()（初始化）
     - 注册四阶段状态机
     - 初始化加密密钥
     - 绑定会话回调
        ↓
【运行期间：跨关卡一直存在】
  主菜单 ──切图──▶ 游戏关卡 ──切图──▶ 主菜单
     ↑                                    │
     └──── GameInstance 始终是同一个 ─────┘
     （地图/玩家来来去去，它一直在）
        ↓
【游戏退出】
  Shutdown()（清理：解绑回调等）
        ↓
  销毁
```

---

## 六、阶段 0-1 在整体流程中的位置

```
【阶段 0】引擎启动 + 读 ini          ← 本文
   ↓
【阶段 1】创建 GameInstance + Init()  ← 本文（注册状态机）
   ↓
【阶段 2】进入主菜单（GameDefaultMap = L_LyraFrontEnd）
   ↓
【阶段 3+】选模式 → 加载 Experience → ... （后续阶段）
```

> 阶段 0-1 是"地基"——GameInstance 这个全局大管家就位后，才有后面的主菜单、Experience 加载等。

---

## 七、常见疑问速答

| 疑问 | 答案 |
|------|------|
| 阶段 0 引擎干嘛了？ | 加载核心模块、初始化渲染/物理，读 ini 知道用哪些类 |
| ini 里 GameInstance 配在哪？ | `GameInstanceClass=/Game/B_LyraGameInstance...` |
| 为什么是蓝图不是纯 C++？ | 蓝图做配置壳，逻辑在 C++ 父类 `ULyraGameInstance` |
| GameInstance 是什么？ | 跨关卡一直存在的全局单例（大管家） |
| 它继承谁？ | `UCommonGameInstance`（CommonGame 插件） |
| Init() 最重要的事？ | 注册四阶段初始化状态机 |
| 四阶段状态机是什么？ | Spawned→DataAvailable→DataInitialized→GameplayReady |
| 为什么需要状态机？ | "被创建"≠"准备好干活"，分阶段推进避免时序 bug |
| GameInstance 什么时候销毁？ | 游戏退出时（Shutdown），全程唯一 |

---

## 八、总结速查

```
阶段 0（引擎启动）：
  双击图标 → 引擎加载核心模块 → 读 DefaultEngine.ini
  ini 告诉引擎：GameInstance = B_LyraGameInstance（蓝图壳）

阶段 1（创建 GameInstance）：
  引擎按 ini 创建 ULyraGameInstance（全局单例，跨关卡不销毁）
  继承 UCommonGameInstance（管会话/登录/加密）
  调 Init()：
    ★ 注册四阶段初始化状态机
       Spawned → DataAvailable → DataInitialized → GameplayReady
    + 初始化调试加密密钥
    + 绑定会话跳转回调

生命周期：
  启动创建 → 全程唯一（跨主菜单/游戏/切图）→ 退出销毁
```

**一句话**：阶段 0 引擎启动、读 `DefaultEngine.ini` 知道"GameInstance 用 `B_LyraGameInstance`"；阶段 1 引擎按配置创建这个**跨关卡不销毁的全局单例**（继承 `UCommonGameInstance`，管会话/登录/加密），其 `Init()` 里注册了 Lyra 的**四阶段初始化状态机**（`Spawned→DataAvailable→DataInitialized→GameplayReady`）——解决"对象被创建 ≠ 准备好干活"的时序问题，为后续启动流程打好地基。

---

## 九、下一步

- 阶段 2：进入主菜单（`L_LyraFrontEnd`），看前端流程（`LyraFrontendStateComponent`）。
- 深入四阶段状态机：看组件怎么按 `InitState` 推进、大管家怎么协调。
- 看 `UCommonGameInstance` 父类怎么处理用户登录/会话。
- 回到 [04_启动全流程](./04_从打开游戏那一刻_启动全流程.md) 看阶段 2 之后。
