# P06 组件化重构 · 总概念 — 为什么拆组件 + InitState 四阶段是什么

> **玩家此刻**：角色行为和 P04/P05 一模一样（能移动、有动画），但内部结构要变——输入逻辑从"角色类里"搬到"独立组件里"。玩家看不到，但代码变"Lyra 味"了。
>
> **本 md 定位**：只讲概念和全景，不写代码。你看完说"懂了/下一步"，我再拆 `.h` 和 `.cpp` 分开教。

---

## 一、先讲清楚"组件化"到底解决什么问题

P04 的时候，为了快点看到效果，我们把输入逻辑（`Input_Move`、`Input_LookMouse`、`SetupPlayerInputComponent`）**直接写在了角色类 `ATenkaichiCharacterWithAbilities` 里**。

这有个隐患：**角色类会越来越胖**。以后还要加：
- 血量/死亡逻辑（`HealthComponent`）
- 装备/武器逻辑（`EquipmentManagerComponent`）
- 相机逻辑（`CameraComponent`）
- 技能授予（GAS）

如果全塞进角色类，这个类会变成几千行的"上帝类"，难维护、难复用。

**Lyra 的解法：把不同功能拆成一个个独立组件（Component），角色类只当"容器"**。

```
之前（P04）：                     之后（P06）：
┌──────────────────┐            ┌──────────────────┐
│ ATenkaichiChar... │            │ ATenkaichiChar... │  ← 角色 = 空壳容器
│  ├ 输入逻辑       │            │  ├ PawnExtension  │  ← 总指挥
│  ├ 移动/转视角    │   变成     │  ├ HeroComponent  │  ← 输入+相机
│  └ (越来越胖...)  │ ───────▶   │  └ (以后还能加...) │
└──────────────────┘            └──────────────────┘
```

对应 Lyra：`ALyraCharacter`（角色壳）+ `ULyraPawnExtensionComponent`（总指挥）+ `ULyraHeroComponent`（输入/相机）。

---

## 二、组件一多，就冒出个新问题：谁先初始化？

拆成多个组件后，立刻有个难题：**这些组件不是各自为政的，它们之间有依赖顺序**。

比如：
- `HeroComponent`（管输入）要等 `PawnExtensionComponent` 说"数据准备好了"才能开始绑输入；
- 以后 `HealthComponent` 要等 ASC（能力系统组件）就位才能初始化血量。

**如果每个组件各管各的、谁先 `BeginPlay` 谁先跑，顺序就会乱，出现"我用你的时候你还没好"的空指针崩溃。**

Lyra 的解法：**用 InitState 四阶段来协调**——所有组件遵守同一个"初始化阶段"协议，由 `UGameFrameworkComponentManager` 统一调度，保证"该等的时候等，该走的时候走"。

---

## 三、InitState 四阶段是什么（P06 的核心）

每个组件（实现 `IGameFrameworkInitStateInterface` 接口）都要走这四步：

| 阶段 | 名字 | 含义 | 类比 |
|------|------|------|------|
| 1 | `Spawned` | 组件已生成（Actor Spawn 出来了） | 演员到场了 |
| 2 | `DataAvailable` | 数据已可用（比如 PawnData 已设置） | 剧本发到手了 |
| 3 | `DataInitialized` | 数据已初始化（依赖的数据都就位） | 演员记住了台词 |
| 4 | `GameplayReady` | 游戏逻辑就绪（可以开始干活了） | 可以上台开演了 |

**关键机制**：一个组件不能自己随便跳阶段，要由 `UGameFrameworkComponentManager` 检查"前置条件是否满足"才放行。

比如 `PawnExtensionComponent` 从 `DataAvailable` 走到 `DataInitialized` 之前，会检查**所有其他组件是否都到了 `DataAvailable`**（源码 `LyraPawnExtensionComponent.cpp` 第 262 行 `HaveAllFeaturesReachedInitState`）——大家都在同一进度，才一起往前走。

---

## 四、减法说明（层次 2：还原 InitState，但砍掉 PawnData 和 ASC avatar）

对照 Lyra 的 `ULyraPawnExtensionComponent`（312 行），层次 2 保留/砍掉如下：

| Lyra 有的 | 我们做吗 | 说明 |
|-----------|---------|------|
| 继承 `UPawnComponent` + 实现 `IGameFrameworkInitStateInterface` | ✅ 保留 | P06 核心 |
| `CanChangeInitState` / `HandleChangeInitState` / `CheckDefaultInitialization` | ✅ 保留 | InitState 四阶段的主体 |
| `NAME_ActorFeatureName`（Feature 名） | ✅ 保留 | 组件在 Manager 里的身份标识 |
| `SetPawnData` / `GetPawnData` / `PawnData` | ❌ 砍 | PawnData 数据资产是 Lyra 的"角色配置数据"，P06 用不到，留到装备课时 |
| `InitializeAbilitySystem` / `UninitializeAbilitySystem`（ASC avatar） | ❌ 砍 | 依赖 GAS avatar 配对，P08 技能课时再引入 |
| `OnRep_PawnData` / 网络复制 | ❌ 砍 | 多人联机才需要，P17 再说 |
| `InitializeAbilitySystem` 委托、`HandleControllerChanged` 等 | ❌ 砍 | 同上，依赖 GAS |

> **一句话**：层次 2 只保留"InitState 四阶段协调初始化"这个骨架，砍掉一切依赖 PawnData / GAS avatar / 网络的肉。等后续课再按需把肉加回来。

---

## 五、前置条件（动手前必须先搭好的地基）

还原 InitState 四阶段，**不是只建一个 C++ 类那么简单**，它依赖三样 Lyra 里"隐式存在"的基础设施：

### 1. 依赖 `ModularGameplay` 插件

`IGameFrameworkInitStateInterface` 和 `UGameFrameworkComponentManager` **不在引擎核心，而在引擎自带的 `ModularGameplay` 插件里**：

```
D:\ue5\Epic Games\UE_5.6\Engine\Plugins\Runtime\ModularGameplay\Source\ModularGameplay\Public\Components\
  ├─ GameFrameworkInitStateInterface.h
  └─ GameFrameworkComponentManager.h
```

所以 Tenkaichi 需要：
- `.uproject` 启用 `ModularGameplay` 插件；
- `.Build.cs` 加 `"ModularGameplay"` 模块依赖。

> Lyra 的 `.Build.cs` 里也是这么依赖 `ModularGameplay` 的（这是 Lyra 架构的根基之一）。

### 2. 建 `InitState_*` 四个 GameplayTag

四个阶段要用 GameplayTag 表示：`InitState.Spawned`、`InitState.DataAvailable`、`InitState.DataInitialized`、`InitState.GameplayReady`。

Lyra 里定义在 `LyraGameplayTags.h/.cpp`（`UE_DECLARE_GAMEPLAY_TAG_EXTERN` + `UE_DEFINE_GAMEPLAY_TAG`）。Tenkaichi 需要**建一个对应的 Tag 定义文件**（比如 `TenkaichiGameplayTags.h/.cpp`）。

### 3. 理解 `UGameFrameworkComponentManager` 的角色

它是"总调度"——每个实现 InitState 接口的组件会 `RegisterInitStateFeature()` 向它登记，之后它统一协调所有组件的阶段流转。这个 Manager 由引擎在 Actor 身上自动提供，我们**不用手动建**，只需在组件里调用它的接口。

---

## 六、P06 三步全景（先看清楚，再动手）

| 步 | 干什么 | 是 C++ 还是配置 | 对应 Lyra |
|----|--------|----------------|-----------|
| 步 1 | 认识分工（本 md） | 概念 | — |
| 步 2 | 加 `ModularGameplay` 依赖 + 建 `InitState_*` 标签 | **配置 + C++ Tag 文件** | `Lyra.Build.cs` / `LyraGameplayTags` |
| 步 3 | 建 `TenkaichiPawnExtensionComponent`（InitState 四阶段） | **C++**（`.h` + `.cpp`） | `LyraPawnExtensionComponent` |
| 步 4 | 建 `TenkaichiHeroComponent`，把输入逻辑搬进来 | **C++**（`.h` + `.cpp`） | `LyraHeroComponent` |
| 步 5 | 角色挂组件 + 删旧输入逻辑 | **C++**（改角色类） | `LyraCharacter` |

> 注意：拆解 md 里原来写的是 4 步，我核实源码后把"前置依赖 + Tag"单列为步 2，因为这是 InitState 的必要地基，不能跳过。

---

## 七、下一步

看完这个总概念，请确认：

1. 理解"组件化"是**把功能从角色拆到组件**，角色当容器；
2. 理解 **InitState 四阶段**（Spawned→DataAvailable→DataInitialized→GameplayReady）是"组件间协调初始化顺序"的协议；
3. 接受**前置条件**：要加 `ModularGameplay` 插件依赖 + 建 `InitState_*` 标签（这是 Lyra 架构的根基，不能省）。

确认后回「**懂了/下一步**」，我从**步 2（加依赖 + 建标签）**开始教——这一步虽然不起眼，但它是整个 InitState 体系的底座。