# 02 — GameFeature 详解（游戏特性/玩法插件）

> **定位**：GameFeature 是 **UE5 引擎的能力**（不是 Lyra 独有的），但 Lyra 大量用它把玩法拆成模块。它是 Lyra 模块化的核心。
>
> **一句话**：GameFeature = **把"一块玩法/内容"封装成可插拔插件**，能按需加载/卸载。这样游戏可以像"拼积木"一样，按不同玩法加载不同模块。
>
> **文件**：`Engine/Plugins/GameFeatures/`（引擎机制）、Lyra `LyraGame/Plugins/`（用法）

---

## 一、GameFeature 是什么

### 一句话

**GameFeature（游戏特性）= 一个"玩法插件"**。它把一整块玩法（比如"团队死斗"、"枪械系统"）打包成插件，可以**按需加载、按需卸载**。

```
GameFeature：GF_团队死斗（一个玩法插件）
  ├─ 代码（GameMode、技能、规则）
  ├─ 资产（蓝图、数据、地图）
  └─ 能：加载它 → 这个玩法生效
           卸载它 → 这个玩法移除
```

### 类比

```
GameFeature = "可插拔的游戏模块"
  就像手机的 APP：
    装个"相机APP" → 有拍照功能
    装个"地图APP" → 有导航功能
    不需要了 → 卸载

GameFeature 同理：
    加载"团队死斗模块" → 有团队死斗玩法
    加载"大逃杀模块" → 有大逃杀玩法
    换玩法 → 换加载的模块
```

---

## 二、GameFeature 解决什么问题（为什么需要）

### 痛点：传统项目玩法全混在一起

```
❌ 传统项目：所有玩法写在一个游戏里
  一个大包里塞了：死斗 + 大逃杀 + 单机 + 合作
  → 代码臃肿、内存占用高、难维护
```

### GameFeature 解决：玩法拆成模块，按需加载

```
✅ GameFeature：玩法拆成独立模块
  GF_死斗 / GF_大逃杀 / GF_合作
  每个模块独立，用哪个加载哪个
  → 代码清晰、内存省、可维护
```

**核心价值**：
| 好处 | 说明 |
|------|------|
| **模块化** | 玩法拆成独立插件 |
| **按需加载** | 用哪个加载哪个，省内存 |
| **可组合** | 自由组合玩法模块 |
| **热更新** | 可动态加载/卸载（打包补丁） |

---

## 三、GameFeature 和普通插件的区别

| | 普通 Plugin | GameFeature |
|---|---|---|
| 加载时机 | 引擎启动就加载 | **运行时按需加载** |
| 卸载 | 基本不卸载 | **可卸载** |
| 用途 | 引擎/编辑器工具 | **玩法内容** |
| 打包 | 全打进去 | **可单独打包/补丁** |

**关键区别**：GameFeature 能**在运行时动态加载/卸载**，普通插件不能。这正是玩法模块化的关键。

---

## 四、GameFeature 怎么工作（核心概念）

### GameFeature 的三个核心：

```
GameFeature
  ├─ 内容（代码 + 资产 + 数据）
  ├─ 状态机（加载流程：安装→激活→开始）
  └─ 和 Experience 配合（谁加载它）
```

### 加载流程（状态机）：

```
GameFeature 加载流程：
  Registered（注册）
    → Downloaded（下载）
      → Installed（安装）
        → Loaded（加载）
          → Active（激活）
            → Started（开始）
```

### 和 Experience 配合：

```
Experience（规则剧本）
  └─ 指定加载哪些 GameFeature（玩法模块）
       ├─ GF_团队系统
       ├─ GF_枪械
       └─ GF_阶段

加载 Experience → 加载它指定的 GameFeature → 玩法生效
```

---

## 五、具体场景：Lyra 怎么用 GameFeature

**场景：Lyra 拆了多个 GameFeature，一个玩法一个模块**

```
Lyra 的 GameFeatures（Plugins/GameFeatures/）：
  ├─ GameFeatures_TeamDeathmatch（团队死斗玩法）
  ├─ GameFeatures_Control（占点玩法）
  ├─ GameFeatures_TopDownArena（俯视角玩法）
  └─ ...（各种玩法模块）

每个模式 = 一个 GameFeature
玩家选模式 → 加载对应 GameFeature → 那个玩法生效
```

**这就是 Lyra 支持"三种游戏模式"的原因**——每种模式是一个 GameFeature，切换就是加载不同的 GameFeature。

---

## 六、GameFeature 和 Experience 的关系（关键）

这是 Lyra 里最容易混的两个概念，要分清：

| | Experience | GameFeature |
|---|---|---|
| 是什么 | **规则数据资产**（剧本） | **玩法插件**（模块） |
| 作用 | 决定"这局怎么玩" | 提供"玩法内容" |
| 关系 | **指定**加载哪些 GameFeature | **被** Experience 加载 |
| 类比 | 剧本 | 演员/道具 |

```
Experience（剧本）
  └─ 加载 → GameFeature（演员/道具）
       └─ 演员按剧本演出 = 游戏按 Experience 规则跑
```

**一句话**：Experience 是"**剧本**"（定规则），GameFeature 是"**演员道具**"（玩法模块）。Experience 决定加载哪些 GameFeature。

---

## 七、总结速查

```
GameFeature = 玩法插件（可插拔，按需加载）
  ├─ 内容：代码 + 资产 + 数据
  ├─ 加载：运行时动态加载/卸载
  └─ 和普通插件区别：能运行时加载/卸载

价值：
  模块化（玩法拆开）
  按需加载（省内存）
  可组合（自由拼）
  热更新（可补丁）

和 Experience：
  Experience = 剧本（定规则）
  GameFeature = 演员道具（玩法模块）
  Experience 加载 GameFeature
```

**一句话**：GameFeature 是 **UE5 把"一块玩法"封装成可插拔插件的能力**，能按需加载/卸载，实现玩法模块化。**Experience（剧本）决定加载哪些 GameFeature（玩法模块）**。Lyra 大量用它把三种游戏模式拆成独立模块。

---

## 八、底层代码实现（真实源码实拍）

> 看 Lyra 工程 `Source/LyraGame/GameFeatures/` 的真实源码，理解 GameFeature 到底怎么实现。

### 8.1 底层三大件

GameFeature 底层由三部分实现：

```
GameFeature 底层：
  ├─ UGameFeatureAction（动作）：一个 GameFeature 里"要做的事"
  ├─ UGameFeaturesSubsystem（子系统）：管理 GameFeature 的加载/卸载
  └─ Observer（观察者）：监听加载/卸载事件，触发动作
```

### 8.2 UGameFeatureAction —— "动作"（核心）

**每个 GameFeature 由一堆"动作（Action）"组成**，每个动作在加载/卸载时执行。Lyra 里有很多种：

```
Lyra 的 GameFeatureAction（Source/LyraGame/GameFeatures/）：
  ├─ GameFeatureAction_AddAbilities（加技能）
  ├─ GameFeatureAction_AddInputBinding（加输入）
  ├─ GameFeatureAction_AddWidget（加 UI）
  ├─ GameFeatureAction_AddInputContextMapping（加输入映射）
  ├─ GameFeatureAction_AddGameplayCuePath（加技能特效路径）
  └─ ...
```

**每个动作继承 `UGameFeatureAction`，重写激活/停用回调**：

```cpp
// 真实源码：GameFeatureAction_AddAbilities.cpp 第 22 行
// 激活时调用（给角色加技能）
void UGameFeatureAction_AddAbilities::OnGameFeatureActivating(...) {
    // 激活：给 Actor 加技能
}

// 停用时调用（移除技能）
void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(...) {
    // 停用：移除技能
}
```

### 8.3 AddAbilities 动作怎么"加技能"（核心逻辑）

看真实源码 `AddActorAbilities`（第 161 行）——它给指定角色添加技能：

```cpp
// 真实源码（简化）：给 Actor 添加技能
void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, ...) {
    // 1. 找 Actor 的 ASC（技能组件）
    if (UAbilitySystemComponent* ASC = FindOrAddComponentForActor<UAbilitySystemComponent>(Actor, ...)) {
        // 2. 遍历配置里的技能，逐个添加
        for (const FLyraAbilityGrant& Ability : AbilitiesEntry.GrantedAbilities) {
            FGameplayAbilitySpec NewSpec(Ability.AbilityType.LoadSynchronous());
            ASC->GiveAbility(NewSpec);   // 给角色添加这个技能
        }
    }
}
```

**理解**：GameFeature 的"加技能"动作 = 找到角色的 ASC → 逐个 `GiveAbility()` 添加技能。加载 GameFeature 时执行，停用时移除。

### 8.4 GameFeaturesSubsystem 和 Observer（管理机制）

Lyra 的 `LyraGameFeaturePolicy.cpp` 里，用 Observer 监听加载事件：

```cpp
// 真实源码：LyraGameFeaturePolicy.cpp 第 19 行
void ULyraGameFeaturePolicy::InitGameFeatureManager() {
    // 注册观察者（监听加载/卸载事件）
    Observers.Add(NewObject<ULyraGameFeature_HotfixManager>());
    Observers.Add(NewObject<ULyraGameFeature_AddGameplayCuePaths>());
    // 把观察者加到子系统
    Subsystem.AddObserver(Observer);
}
```

**观察者（Observer）在 GameFeature 加载/卸载时被通知**，从而触发各种动作（加技能、加 UI、热修复等）。

### 8.5 完整底层流程（把源码串起来）

```
1. 创建 GameFeature（.uplugin + GameFeatureData 数据资产）
2. GameFeatureData 里配置一堆 Action（动作）
3. GameFeaturesSubsystem（子系统）加载 GameFeature
4. 子系统通知 Observer（观察者）
5. Observer 触发每个 Action 的 OnGameFeatureActivating
6. Action 执行具体工作（AddAbilities 给角色加技能等）
7. 停用时，触发 OnGameFeatureDeactivating，撤销工作
```

### 8.6 底层总结

```
GameFeature 底层 = Action（动作）+ Subsystem（子系统）+ Observer（观察者）

  Action（UGameFeatureAction）：一个 GameFeature 里"要做的事"
    例：AddAbilities（加技能）/ AddWidget（加UI）/ AddInputBinding（加输入）
  Subsystem（UGameFeaturesSubsystem）：管理加载/卸载
  Observer（观察者）：监听事件，触发 Action

加载流程：
  加载 GameFeature → 通知 Observer → 触发 Action → 执行（加技能等）
```

**一句话（看源码后）**：GameFeature 底层是 **`UGameFeatureAction`（动作）+ `UGameFeaturesSubsystem`（子系统）+ Observer（观察者）**。一个 GameFeature 由一堆 Action 组成，加载时 `GameFeaturesSubsystem` 通知 Observer，Observer 触发各 Action 的 `OnGameFeatureActivating` 执行具体工作（加技能/加 UI），停用时 `OnGameFeatureDeactivating` 撤销。**Lyra 的 `GameFeatureAction_AddAbilities` 等就是具体动作，通过给角色的 ASC `GiveAbility()` 实现"加技能"。**

---

## 九、下一步

理解了 GameFeature，下一步可以深入 **GameFeature 的加载流程/状态机**，或看 **Lyra 具体怎么用 GameFeature 拆玩法**。
