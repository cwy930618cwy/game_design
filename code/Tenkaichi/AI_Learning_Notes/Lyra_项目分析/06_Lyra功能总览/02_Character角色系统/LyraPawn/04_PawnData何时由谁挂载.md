# PawnData 什么时候、由谁挂到 LyraPawn 上？

> **定位**：讲清 Modular 框架装配链路的最后一环——**PawnData 这份"配方表"到底在什么时机、被谁塞进角色的扩展组件里**。这是理解"组件化角色如何诞生"的关键。
>
> **关联**：
> - [01_LyraPawn.h 详解（第六节）](./01_LyraPawn.h详解.md) — 继承 AModularPawn 后挂组件代码在哪
> - [02_PawnExtensionComponent](../02_PawnExtensionComponent.md) — 接收 PawnData 的中枢组件
> - [AModularPawn 到底是什么](../03_AModularPawn到底是什么.md) — 空壳基类的"按铃"机制
>
> **一句话**：PawnData 不是在角色自己身上配的，而是 **GameMode 在"造角色"那一刻，把配方表塞进角色的 PawnExtensionComponent 里**——服务器直接塞，客户端靠网络复制收到后自己塞。

---

## 一、直接答案

> **什么时候**：GameMode **Spawn 角色的那一刻**（`SpawnDefaultPawnAtTransform`）。
> **由谁**：**GameMode** 调用 `PawnExtensionComponent->SetPawnData()`。

```cpp
// LyraGameMode.cpp 第 345-366 行（真实源码）
APawn* ALyraGameMode::SpawnDefaultPawnAtTransform_Implementation(...)
{
    // 1. 造出空壳 Pawn
    if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
    {
        // 2. 找到它身上的扩展组件
        if (ULyraPawnExtensionComponent* PawnExtComp =
            ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
        {
            // 3. 拿到这个玩家的 PawnData
            if (const ULyraPawnData* PawnData = GetPawnDataForController(NewPlayer))
            {
                // 4. ★ 挂上去！★
                PawnExtComp->SetPawnData(PawnData);
            }
        }
        SpawnedPawn->FinishSpawning(SpawnTransform);
    }
}
```

---

## 二、完整时间线

```
① 玩家加入游戏 → GameMode 要给他造一个 Pawn
      ▼
② GameMode::GetDefaultPawnClassForController()   ← 先决定"用什么 Pawn 类"
      │  从 PlayerState / Experience 里拿到 PawnData
      │  返回 PawnData->PawnClass（这份配方造哪种 Pawn）
      ▼
③ GameMode::SpawnDefaultPawnAtTransform()        ← 真正 Spawn 的那一刻
      │  SpawnActor<APawn>(PawnClass)             ← 造出空壳 Pawn
      │       ↓
      │  找到它身上的 PawnExtensionComponent
      │       ↓
      │  PawnExtComp->SetPawnData(PawnData)       ← ★【就是这里！挂 PawnData】
      ▼
④ SetPawnData() 触发 OnRep_PawnData → 组件开始按 PawnData 装配其他组件
```

### 两个关键时机点

| 时机 | 函数 | 干了什么 |
|------|------|---------|
| **决定用哪个类** | `GetDefaultPawnClassForController` | 读 PawnData → 取 `PawnClass`（造哪种 Pawn） |
| **Spawn 那一刻** | `SpawnDefaultPawnAtTransform` | 造出 Pawn → `SetPawnData()` **挂配方** |

> 注意顺序：**先用 PawnData 决定类 → Spawn 出 Pawn → 再把同一份 PawnData 挂到组件上**。所以 PawnData 被用了两次：一次决定"造什么"，一次决定"怎么装"。

---

## 三、PawnData 从哪来（三级优先级）

`GetPawnDataForController()` 有个明确的查找顺序（`LyraGameMode.cpp` 第 45-77 行）：

```cpp
GetPawnDataForController(InController)
{
    // 优先级 1：PlayerState 上已经指定了（比如选择英雄时定的）
    if (LyraPS->GetPawnData() != nullptr)
        return LyraPS->GetPawnData();

    // 优先级 2：当前 Experience（关卡体验）配置的默认 PawnData
    if (Experience->DefaultPawnData != nullptr)
        return Experience->DefaultPawnData;

    // 优先级 3：全局兜底默认值
    return ULyraAssetManager::Get().GetDefaultPawnData();
}
```

| 优先级 | 来源 | 场景 |
|--------|------|------|
| 1 | **PlayerState** | 玩家在菜单里选了具体英雄/职业 |
| 2 | **Experience 配置** | 当前关卡模式指定的默认角色 |
| 3 | **全局默认** | 啥都没配时的兜底 |

---

## 四、客户端怎么办（网络复制）

`PawnData` 这个成员变量带了 `ReplicatedUsing = OnRep_PawnData`（见 `LyraPawnExtensionComponent.h`）：

```cpp
UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData)
TObjectPtr<const ULyraPawnData> PawnData;
```

- **服务器**：GameMode 直接调 `SetPawnData()`（权威）。
- **客户端**：服务器同步过来后自动触发 `OnRep_PawnData()` → 客户端也开始装配组件。

> 这就是为什么客户端的角色也能正确装上组件——靠的是属性复制。

---

## 五、一张图看懂全流程

```
┌─────────────┐
│  PawnData   │  数据资产（配方表）
└──────┬──────┘
       │ 被读取
       ▼
┌─────────────────────────────────────────┐
│  GameMode                                │
│   GetDefaultPawnClassForController()     │  ← 用 PawnData.PawnClass 决定造哪种 Pawn
│   SpawnDefaultPawnAtTransform()          │  ← Spawn 出空壳 Pawn
│        └─► PawnExtComp->SetPawnData()    │  ← ★【挂 PawnData 的真正时刻】
└───────────────────┬─────────────────────┘
                    │ 服务器直接调 / 客户端靠 OnRep 复制
                    ▼
┌─────────────────────────────────────────┐
│  LyraPawnExtensionComponent              │
│   SetPawnData() → OnRep_PawnData         │
│        └─► 按 PawnData 装配其他组件       │  ← 血量/相机/技能等组件在此挂载
└─────────────────────────────────────────┘
```

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "PawnData 是角色自己加载的" | ❌ 是 GameMode 在 Spawn 时塞进去的 |
| "在构造函数里挂 PawnData" | ❌ 构造函数空的，在 Spawn 后才 Set |
| "客户端拿不到 PawnData" | ❌ 靠 `OnRep_PawnData` 网络复制自动收到 |
| "PawnData 只用于决定 Pawn 类" | ❌ 用了两次：决定类 + 指导组件装配 |
| "只有服务器需要 PawnData" | ❌ 客户端也要，用来本地装配组件 |

---

## 七、总结速查

```
什么时候挂 PawnData？
  → GameMode Spawn 角色的那一刻（SpawnDefaultPawnAtTransform）

谁来挂？
  → GameMode 调用 PawnExtensionComponent->SetPawnData()

PawnData 从哪来（三级优先级）？
  → PlayerState 指定 > Experience 配置 > 全局默认

客户端呢？
  → 服务器复制过来，触发 OnRep_PawnData 自动装配
```

**核心记忆**：GameMode 造角色时，**先把配方表塞进角色的扩展组件**——服务器直接塞，客户端靠网络复制收到后自己塞。组件的真正装配，就发生在 `SetPawnData` 之后的流程里。

---

## 八、下一步

- [02_PawnExtensionComponent](../02_PawnExtensionComponent.md) — 看它收到 PawnData 后如何协调初始化
- [01_LyraPawn.h 详解](./01_LyraPawn.h详解.md) — 回到角色基类本身
- [AModularPawn 到底是什么](../03_AModularPawn到底是什么.md) — 空壳"按铃"机制
