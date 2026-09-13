# 03 — Experience 详解（附真实源码）

> **定位**：Lyra 最核心的系统——**Experience（体验/规则数据资产）**。这篇用真实源码讲清楚它是什么、怎么实现、怎么工作。
>
> **一句话**：Experience = **用数据资产定义的一整套游戏规则**（用什么 Pawn、加载哪些 GameFeature、做什么动作）。由 `ExperienceManagerComponent` 加载驱动，换 Experience 就换整局玩法。
>
> **文件**：Lyra `Source/LyraGame/GameModes/`（`LyraExperienceDefinition.h`、`LyraExperienceManagerComponent.h` 等 12 个文件）

---

## 一、Experience 是什么（一句话）

**Experience（体验）是一个数据资产（DataAsset）**，它"打包"了一整套游戏规则。像一个"剧本"，告诉 Lyra"这一局怎么玩"。

```
Experience（剧本）
  ├─ 用什么 Pawn（玩家角色）
  ├─ 启用哪些 GameFeature（玩法模块）
  ├─ 做什么动作（Action：加技能/加UI）
  └─ 组合哪些 ActionSet（玩法包）
```

---

## 二、真实源码：LyraExperienceDefinition（数据资产定义）

**这是 Experience 的核心定义**（我读的真实源码 `LyraExperienceDefinition.h`）：

```cpp
// LyraExperienceDefinition.h（真实源码）
UCLASS(BlueprintType, Const)
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ① 这个 Experience 要启用哪些 GameFeature 插件
    UPROPERTY(EditDefaultsOnly, Category=Gameplay)
    TArray<FString> GameFeaturesToEnable;

    // ② 玩家默认用什么 Pawn
    UPROPERTY(EditDefaultsOnly, Category=Gameplay)
    TObjectPtr<const ULyraPawnData> DefaultPawnData;

    // ③ 加载/激活时要执行的动作（加技能/加UI等）
    UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
    TArray<TObjectPtr<UGameFeatureAction>> Actions;

    // ④ 要组合进来的 ActionSet（玩法包）
    UPROPERTY(EditDefaultsOnly, Category=Gameplay)
    TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;
};
```

**四个核心成员**（从源码看）：
| 成员 | 作用 |
|------|------|
| `GameFeaturesToEnable` | 启用哪些 GameFeature 插件 |
| `DefaultPawnData` | 玩家默认 Pawn |
| `Actions` | 执行的动作（加技能/加UI） |
| `ActionSets` | 组合的玩法包 |

---

## 三、真实源码：LyraExperienceManagerComponent（管理器）

**Experience 不是自己执行，由 ManagerComponent 加载驱动**。我读的真实源码 `LyraExperienceManagerComponent.h`：

### 加载状态机（源码第 18-27 行）

```cpp
// 加载状态（真实源码）
enum class ELyraExperienceLoadState
{
    Unloaded,              // 未加载
    Loading,               // 加载中
    LoadingGameFeatures,   // 加载 GameFeature
    LoadingChaosTestingDelay, // 测试延迟
    ExecutingActions,      // 执行动作
    Loaded,                // 加载完成
    Deactivating           // 停用中
};
```

### 加载接口（源码第 47-67 行）

```cpp
// 管理器接口（真实源码）
class ULyraExperienceManagerComponent final : public UGameStateComponent
{
    GENERATED_BODY()
public:
    // 设置当前 Experience
    void SetCurrentExperience(FPrimaryAssetId ExperienceId);

    // 加载完成后回调
    void CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate);

    // 当前 Experience 是否加载完成
    bool IsExperienceLoaded() const;

    // 获取当前 Experience
    const ULyraExperienceDefinition* GetCurrentExperienceChecked() const;

private:
    // 当前 Experience（网络复制）
    UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
    TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;

    // 当前加载状态
    ELyraExperienceLoadState LoadState = ELyraExperienceLoadState::Unloaded;
};
```

**关键**：
- `SetCurrentExperience()` 设置要加载的 Experience
- `LoadState` 记录加载进度（状态机）
- `IsExperienceLoaded()` 判断是否加载完
- `OnExperienceLoaded` 加载完成后的回调委托

---

## 四、Experience 怎么工作（加载流程）

结合源码的状态机，加载流程是：

```
SetCurrentExperience(ExperienceId)
  ↓
StartExperienceLoad()
  ↓ 状态：Unloaded → Loading
加载 Experience 数据资产
  ↓ 状态：Loading
启用 GameFeaturesToEnable（加载 GameFeature 插件）
  ↓ 状态：LoadingGameFeatures
执行 Actions（加技能/加UI等）
  ↓ 状态：ExecutingActions
状态：Loaded（加载完成）
  ↓
触发 OnExperienceLoaded 回调
```

**核心**：ManagerComponent 用**状态机**一步步加载 Experience → 启用 GameFeature → 执行 Action → 完成。

---

## 五、Experience + GameFeature + Action 的关系（源码视角）

从源码看，三者是这样的组合：

```
Experience（数据资产）
  ├─ GameFeaturesToEnable → 启用 GameFeature 插件
  ├─ Actions → UGameFeatureAction（具体动作：加技能/加UI）
  └─ ActionSets → 玩法包（更多动作）

加载 Experience = 启用 GameFeature + 执行 Actions
```

**具体场景：加载一个"团队死斗" Experience**

```
Experience_TeamDeathmatch（数据资产）
  ├─ GameFeaturesToEnable = ["GF_TeamSystem", "GF_Weapon"]
  ├─ DefaultPawnData = 士兵 Pawn
  ├─ Actions = [AddAbilities（加射击技能）, AddWidget（加HUD）]
  └─ ActionSets = [TeamActionSet]

加载它 → 启用队伍/武器插件 → 给玩家加射击技能 + HUD → 团队死斗生效
```

---

## 六、真实源码对照表

| 概念 | 真实类/文件 | 作用 |
|------|------------|------|
| Experience 定义 | `ULyraExperienceDefinition` | 数据资产（规则） |
| 管理器 | `ULyraExperienceManagerComponent` | 加载驱动 |
| 加载状态 | `ELyraExperienceLoadState` | 状态机 |
| 动作 | `UGameFeatureAction` | 具体行为（加技能等） |
| 玩法包 | `ULyraExperienceActionSet` | 组合动作 |
| 回调 | `FOnLyraExperienceLoaded` | 加载完成通知 |

---

## 七、总结速查

```
Experience = 规则数据资产
  ├─ GameFeaturesToEnable（启用哪些 GameFeature）
  ├─ DefaultPawnData（玩家 Pawn）
  ├─ Actions（执行的动作）
  └─ ActionSets（玩法包）

管理器 = LyraExperienceManagerComponent
  用状态机加载：
  Unloaded → Loading → LoadingGameFeatures → ExecutingActions → Loaded
  完成触发 OnExperienceLoaded

加载 = 启用 GameFeature + 执行 Action
```

**一句话（看源码后）**：Experience 是 **`ULyraExperienceDefinition` 数据资产**，定义了一整套规则（GameFeaturesToEnable/DefaultPawnData/Actions）。由 **`ULyraExperienceManagerComponent`** 用**状态机**加载（Unloaded→Loading→LoadingGameFeatures→ExecutingActions→Loaded），加载完成后触发 `OnExperienceLoaded` 回调。**加载 Experience = 启用 GameFeature + 执行 Action**，换 Experience 就换整局玩法。

---

## 八、下一步

理解了 Experience 源码，下一步可以深入 **Experience 的完整加载流程时序**（ManagerComponent 怎么一步步走状态机），或看 **GameMode 家族怎么和 Experience 配合**。
