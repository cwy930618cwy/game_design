# 05-01 — CommonGame 详解（通用游戏框架）

> **定位**：Lyra 的 `CommonGame` 插件——**通用游戏框架**。它提供一套"可复用的游戏基础设施"，让不同游戏项目共用。
>
> **一句话**：`CommonGame` = **游戏常用的"骨架类"**（GameInstance/PlayerController/UI 管理等），封装了通用逻辑，让你不用每次从零写。
>
> **文件**：`e:\code\lyra_fifty_six\LyraStarterGame\Plugins\CommonGame/`

---

## 一、CommonGame 是什么

**CommonGame 是一套"通用游戏框架"插件**，提供游戏项目常用的骨架类：

```
CommonGame（通用游戏框架）
  ├─ 通用 GameInstance（CommonGameInstance）
  ├─ 通用 PlayerController（CommonPlayerController）
  ├─ 通用 LocalPlayer（CommonLocalPlayer）
  ├─ UI 管理（GameUIManager）
  └─ ...（各种常用类）
```

**价值**：这些是"任何游戏都要用"的基础类，Lyra 封装好，别的项目也能复用。

---

## 二、CommonGame 实际提供什么（我看的真实目录）

```
CommonGame/Source/Public/
├── CommonGameInstance.h        ← 通用游戏实例
├── CommonPlayerController.h    ← 通用玩家控制器
├── CommonLocalPlayer.h         ← 通用本地玩家
├── CommonPlayerInputKey.h      ← 玩家输入键
├── CommonUIExtensions.h        ← UI 扩展
├── GameUIManagerSubsystem.h    ← UI 管理子系统
├── GameUIPolicy.h              ← UI 策略
├── PrimaryGameLayout.h         ← 主 UI 布局
├── Actions/                    ← 异步动作
└── Messaging/                  ← 消息
```

---

## 三、核心类逐个讲（配场景）

### ① CommonGameInstance —— 通用游戏实例

继承 UGameInstance，加了通用逻辑（消息、UI 管理入口）。

```cpp
// 通用游戏实例（游戏启动就有）
UCLASS()
class UCommonGameInstance : public UGameInstance {
    GENERATED_BODY()
public:
    // 通用逻辑（处理消息、管理 UI）
};
```

**场景**：游戏实例里管理 UI、处理通用消息。

### ② CommonPlayerController —— 通用玩家控制器

继承 APlayerController，加了通用输入/UI 处理。

```cpp
// 通用玩家控制器
UCLASS()
class UCommonPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    // 通用输入/UI 逻辑
};
```

**场景**：所有玩家控制器继承它，获得通用能力。

### ③ GameUIManagerSubsystem —— UI 管理子系统

**管理游戏 UI 的显示/隐藏、层级**。

```cpp
// UI 管理器（管理所有 UI）
UCLASS()
class UGameUIManagerSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()
public:
    // 设置当前 UI 策略
    void SetCurrentUIPolicy(UGameUIPolicy* NewPolicy);
};
```

**场景**：集中管理 UI 的显示策略、层级。

### ④ PrimaryGameLayout —— 主 UI 布局

**游戏主 UI 的根布局**，UI 都挂在这上面。

```cpp
// 主 UI 布局（所有 UI 的容器）
UCLASS()
class UPrimaryGameLayout : public UCommonUserWidget {
    GENERATED_BODY()
public:
    // 把一个 UI 推到这个布局的某层
    void PushContentToLayer(...);
};
```

**场景**：所有 HUD/菜单都挂到主布局的层上。

---

## 四、CommonGame 和 CommonUI 的关系（注意别混）

| | CommonUI | CommonGame |
|---|---|---|
| 是什么 | UI 框架（控件） | 游戏框架（骨架类） |
| 管什么 | 控件、导航、输入 | GameInstance/Controller/UI 管理 |
| 例子 | CommonButtonBase | CommonGameInstance |

**关系**：
```
CommonGame（游戏骨架）
  ├─ 用 CommonUI（控件做 UI）
  ├─ 用 GameUIManager（管理 UI）
  └─ 用 CommonGameInstance（游戏实例）
```

**理解**：CommonGame 是"游戏骨架"，CommonUI 是"UI 零件"，CommonGame 用 CommonUI 拼 UI。

---

## 五、CommonGame 解决什么问题

| 问题 | CommonGame 解决 |
|------|----------------|
| 每次从零写 GameInstance | 提供 CommonGameInstance |
| UI 管理混乱 | GameUIManagerSubsystem 集中管 |
| UI 层级难控制 | PrimaryGameLayout 分布局层 |
| 玩家控制器重复代码 | CommonPlayerController |

**价值**：**通用逻辑封装好，复用性强**，Lyra 用它，别的项目也能用。

---

## 六、总结速查

```
CommonGame = 通用游戏框架插件
  ├─ CommonGameInstance（通用游戏实例）
  ├─ CommonPlayerController（通用玩家控制器）
  ├─ CommonLocalPlayer（通用本地玩家）
  ├─ GameUIManagerSubsystem（UI 管理）
  ├─ PrimaryGameLayout（主 UI 布局）
  ├─ CommonUIExtensions（UI 扩展）
  └─ Actions/Messaging（动作/消息）

和 CommonUI 区别：
  CommonUI = UI 控件框架
  CommonGame = 游戏骨架（用 CommonUI 拼 UI）

价值：通用逻辑封装，可复用
```

**一句话**：`CommonGame` 是 **通用游戏框架插件**，提供 CommonGameInstance、CommonPlayerController、GameUIManager（UI 管理）、PrimaryGameLayout（主布局）等**游戏骨架类**。**它是"游戏骨架"，CommonUI 是"UI 零件"**，Lyra 用它们搭出完整游戏。

---

## 七、CommonGameInstance / CommonPlayerController 真实源码与场景

> 直接看这两个类的真实源码，理解它们具体有什么、用在什么场景。

### 7.1 CommonGameInstance（通用游戏实例）—— 管"会话/用户/邀请"

**继承** `UGameInstance`，核心加了**用户（CommonUser）、会话（Session）、邀请**逻辑。

**真实源码**（`CommonGameInstance.h`）：

```cpp
UCLASS(Abstract, Config=Game)
class UCommonGameInstance : public UGameInstance {
    GENERATED_BODY()
public:
    // ① 处理系统消息（用户/错误）
    virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

    // ② 处理用户权限变化
    virtual void HandlePrivilegeChanged(...);

    // ③ 会话邀请流程（重点）
    virtual void OnUserRequestedSession(...);   // 玩家点了邀请
    virtual bool CanJoinRequestedSession() const;  // 能否加入
    virtual void JoinRequestedSession();          // 加入会话
    virtual void ResetGameAndJoinRequestedSession(); // 重置游戏再加入

    // ④ 本地玩家管理
    virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
    virtual bool RemoveLocalPlayer(...) override;

private:
    TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;   // 主玩家
    TObjectPtr<UCommonSession_SearchResult> RequestedSession;  // 待加入的会话
};
```

**具体场景：玩家在平台界面点"接受好友邀请"**

```
玩家在平台（如 PS 主菜单）点"接受邀请"
  ↓
CommonGameInstance::OnUserRequestedSession 被调用
  ↓
检查能否立即加入（CanJoinRequestedSession）
  ├─ 能 → JoinRequestedSession（直接加入）
  └─ 不能（还在主菜单）→ ResetGameAndJoinRequestedSession
       ↓ 先回到能加入的状态，再加入
```

**核心**：CommonGameInstance 管**游戏全局的会话/用户管理**——跨关卡、跨场景都在，处理邀请、用户登录、加本地玩家。

---

### 7.2 CommonPlayerController（通用玩家控制器）—— 管"控制角色/同步状态"

**继承** `AModularPlayerController`，重写了**玩家接收、Possess 控制**等生命周期。

**真实源码**（`CommonPlayerController.h`）：

```cpp
UCLASS(config=Game)
class ACommonPlayerController : public AModularPlayerController {
    GENERATED_BODY()
public:
    // ① 玩家被接收（连接建立）时
    virtual void ReceivedPlayer() override;

    // ② 设置/控制 Pawn 时
    virtual void SetPawn(APawn* InPawn) override;
    virtual void OnPossess(APawn* APawn) override;   // 附身角色
    virtual void OnUnPossess() override;              // 离开角色

protected:
    // ③ PlayerState 更新时
    virtual void OnRep_PlayerState() override;
};
```

**具体场景：玩家进入游戏，控制自己的角色**

```
玩家连接 → ReceivedPlayer（连接建立）
  ↓
GameMode 生成玩家角色 → OnPossess（控制器附身到角色）
  ↓
OnRep_PlayerState（同步玩家状态：分数/名字）
  ↓
玩家开始操作
```

**核心**：CommonPlayerController 处理**玩家控制角色的生命周期**——连接、附身、同步状态。

---

### 7.3 两个类分工对比

| | CommonGameInstance | CommonPlayerController |
|---|---|---|
| 是什么 | 游戏全局实例 | 单个玩家控制器 |
| 数量 | 一个 | 每个玩家一个 |
| 管什么 | 会话/用户/邀请 | 控制角色/同步状态 |
| 场景 | 接受邀请、用户登录 | 附身角色、玩家状态 |

```
CommonGameInstance（全局：管会话/用户）
  ├─ 管：邀请、登录、加玩家
CommonPlayerController（单个玩家：管控制）
  ├─ 管：附身角色、玩家状态
```

**一句话（配场景）**：**CommonGameInstance** 是游戏全局的"管家"，管**会话/用户/邀请**（场景：玩家点"接受好友邀请"，它负责能否加入）；**CommonPlayerController** 是单个玩家的"大脑"，管**控制角色/同步状态**（场景：玩家连接后它 `OnPossess` 附身到角色）。**GameInstance 管"全局"，PlayerController 管"单个玩家"。**

---

## 七、下一步

理解了 CommonGame，下一步可以深入它的核心类（如 **GameUIManagerSubsystem UI 管理** 或 **PrimaryGameLayout 主布局**），或看它和 CommonUI 怎么配合。
