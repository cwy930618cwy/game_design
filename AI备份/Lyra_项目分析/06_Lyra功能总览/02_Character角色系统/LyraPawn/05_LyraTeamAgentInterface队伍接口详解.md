# LyraTeamAgentInterface —— 队伍接口详解（附真实源码+注释）

> **定位**：逐行拆解 `LyraTeamAgentInterface.h`——Lyra 让"任何单位都能拥有队伍归属"的接口。`LyraPawn.h` 第 36 行 `#include "Teams/LyraTeamAgentInterface.h"` 引入的就是它。
>
> **关联**：
> - [01_LyraPawn.h 详解](./01_LyraPawn.h详解.md) — 谁在用它（`ALyraPawn` 实现它）
> - [02_LyraPawn.cpp 详解](./02_LyraPawn.cpp详解.md) — 怎么用它（Set/Get 队伍、广播变化）
>
> **一句话**：这是一个"**队伍身份证**"接口。任何实现了它的 Actor，都能被 Lyra 的队伍系统统一管理——不用管它是 Pawn、Controller 还是 PlayerState，队伍系统都能问它："你属于哪队？"

---

## 一、为什么需要这个接口

Lyra 里很多对象都要知道"队伍归属"：
- **Pawn**（角色身体）——敌人识别靠队伍
- **Controller**（控制器）——玩家的队伍
- **PlayerState**（记分牌）——UI 显示队友

如果每个系统都去判断"这到底是 Pawn 还是 Controller"，代码会一团乱。

**接口的作用就是统一**：只要"你有队伍接口"，队伍系统就能用同一套方式访问，**不用管它具体是什么类**。

```cpp
// 没有接口：得分类讨论
if (是 Pawn) 队伍 = ((ALyraPawn*)X)->GetGenericTeamId();
else if (是 Controller) 队伍 = ((ALyraController*)X)->GetGenericTeamId();
... // 一堆 else if

// 有接口：一行搞定
队伍 = ILyraTeamAgentInterface::GetGenericTeamId(X);  // 统一调用
```

---

## 二、真实源码 + 逐段注释

### ① 依赖与委托声明

```cpp
#include "GenericTeamAgentInterface.h"   // ← 引擎的通用队伍接口（父接口）
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "LyraTeamAgentInterface.generated.h"

#define UE_API LYRAGAME_API

// ★ 核心：队伍变化的广播委托
// 三个参数：谁在变、旧队伍 ID、新队伍 ID
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnLyraTeamIndexChangedDelegate,
    UObject*, ObjectChangingTeam,      // 哪个对象在换队
    int32, OldTeamID,                  // 旧的队伍 ID
    int32, NewTeamID);                 // 新的队伍 ID
```

**要点**：
- `DYNAMIC_MULTICAST_DELEGATE` = 可绑定多个回调、可在蓝图里用的委托（"广播"型）。
- 当某对象的队伍变了，就广播这个委托，所有监听者（UI、特效、逻辑）都能收到通知。

---

### ② 两个类型转换小工具函数

```cpp
// 队伍 ID ↔ int32 互转（因为委托参数用的是 int32）
inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
    return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : (int32)ID;
}

inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
    return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId((uint8)ID);
}
```

**要点**：
- `FGenericTeamId` 是引擎的队伍 ID 类型，委托里传的是 `int32`，所以需要互转。
- 特殊约定：`NoTeam`（无队伍）↔ `INDEX_NONE`（-1）。

---

### ③ 接口声明（核心）

```cpp
// 接口只能挂在 UINTERFACE 类上（UE 反射要求）
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class ULyraTeamAgentInterface : public UGenericTeamAgentInterface
{
    GENERATED_UINTERFACE_BODY()
};

// 真正的接口方法在这里（C++ 类，I 开头）
class ILyraTeamAgentInterface : public IGenericTeamAgentInterface
{
    GENERATED_IINTERFACE_BODY()

    // 返回"队伍变化"委托的句柄（让别人能监听）
    virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

    // 广播队伍变化（静态函数，所有地方统一调用）
    static UE_API void ConditionalBroadcastTeamChanged(
        TScriptInterface<ILyraTeamAgentInterface> This,
        FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

    // 安全获取委托（获取失败就 assert）
    FOnLyraTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
    {
        FOnLyraTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
        check(Result);   // 如果没有实现这个函数，这里会断言崩溃（提醒开发者）
        return *Result;
    }
};
```

**要点**：
- UE 接口都是**一对**：`ULyraTeamAgentInterface`（UObject 包装，给反射/蓝图用）+ `ILyraTeamAgentInterface`（C++ 方法，给你重写）。
- `CannotImplementInterfaceInBlueprint` = 这个接口**不能在蓝图里实现**，只能 C++ 写。
- 继承 `IGenericTeamAgentInterface` → 自动获得 `SetGenericTeamId` / `GetGenericTeamId` 等基础方法的约定。

---

## 三、接口家族谱（继承关系）

```
IGenericTeamAgentInterface（引擎提供，Plugins/GenericTeam 插件）
    │  定义了：SetGenericTeamId / GetGenericTeamId / GetTeamAttitudeTowards 等
    │
    └── ILyraTeamAgentInterface（Lyra 扩展）
            │  新增：GetOnTeamIndexChangedDelegate（队伍变化委托）
            │       + ConditionalBroadcastTeamChanged（统一广播）
            │
            └── 被这些类实现：
                  ├─ ALyraPawn（角色身体）
                  ├─ ALyraController（控制器）
                  └─ ALyraPlayerState（记分牌）
```

> `IGenericTeamAgentInterface` 是引擎自带的，路径在 `Plugins/Runtime/GenericTeam`。Lyra 只是在此基础上加了个"队伍变化广播"的委托。

---

## 四、它在 LyraPawn 里怎么用（配合回顾）

`LyraPawn.h` 里：

```cpp
class ALyraPawn : public AModularPawn,
                  public ILyraTeamAgentInterface   // ← 实现这个接口
{
    // 实现接口的三个方法：
    void SetGenericTeamId(...);                    // 设置队伍
    FGenericTeamId GetGenericTeamId() const;       // 获取队伍
    FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate();  // 拿委托
};
```

`LyraPawn.cpp` 里的配合：

```cpp
// PossessedBy：被控制时，跟着 Controller 走队伍 + 绑定监听
ControllerAsTeamProvider->GetTeamChangedDelegateChecked()
    .AddDynamic(this, &ThisClass::OnControllerChangedTeam);   // 监听 Controller 换队

// 队伍变化时统一广播
ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
```

---

## 五、一张图看懂"队伍系统"如何运转

```
┌──────────────────────────────────────────────┐
│  队伍系统（AI 寻敌、UI 显示队友…）             │
│      只认接口，不关心具体是哪种对象             │
└───────────────────┬──────────────────────────┘
                    │ 统一调用 ILyraTeamAgentInterface 的方法
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   ┌─────────┐ ┌──────────┐ ┌────────────┐
   │LyraPawn │ │LyraController│ │LyraPlayerState│
   │(身体)   │ │(控制器)     │ │(记分牌)      │
   └────┬────┘ └──────────┘ └────────────┘
        │ 队伍变化时
        ▼
   ConditionalBroadcastTeamChanged()  → 广播委托
        │
        ▼
   所有监听者收到通知（UI 刷新、特效、逻辑…）
```

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "接口里有具体逻辑" | ❌ 接口只声明方法，实现在 LyraPawn 等类里 |
| "GetOnTeamIndexChangedDelegate 默认返回有效值" | 默认返回 `nullptr`，所以要用 `GetTeamChangedDelegateChecked`（带 check） |
| "队伍 ID 随便改就行" | ❌ 只有服务器/未控制时能改（见 LyraPawn.cpp 的 SetGenericTeamId） |
| "这个接口是 Lyra 从零写的" | 它继承引擎的 `IGenericTeamAgentInterface`，Lyra 只加了广播委托 |
| "蓝图也能实现这个接口" | ❌ 标记了 `CannotImplementInterfaceInBlueprint`，只能 C++ |

---

## 七、总结速查

```
LyraTeamAgentInterface = "队伍身份证"接口

结构：
  ├─ ULyraTeamAgentInterface（UObject 包装，给反射）
  └─ ILyraTeamAgentInterface（C++ 方法，给你重写）
       ├─ 继承 IGenericTeamAgentInterface（引擎提供）
       ├─ GetOnTeamIndexChangedDelegate（拿队伍变化委托）
       └─ ConditionalBroadcastTeamChanged（统一广播）

核心价值：
  统一访问 —— 队伍系统不用管你是 Pawn/Controller/PlayerState
  广播通知 —— 队伍一变，UI/特效/逻辑都能收到

关键委托：FOnLyraTeamIndexChangedDelegate(对象, 旧队, 新队)
```

**一句话**：这个接口让 Lyra 的所有对象都能用**统一方式**管理队伍归属——队伍系统只管问"你属于哪队"，不用关心对方是 Pawn 还是 Controller；队伍一变就广播委托，让 UI、特效、逻辑自动响应。

---

## 八、下一步

- [01_LyraPawn.h 详解](./01_LyraPawn.h详解.md) — 看它如何被 LyraPawn 实现
- [02_LyraPawn.cpp 详解](./02_LyraPawn.cpp详解.md) — 看队伍同步的具体实现
- 引擎 `Plugins/Runtime/GenericTeam` — 了解父接口 `IGenericTeamAgentInterface` 的完整能力
