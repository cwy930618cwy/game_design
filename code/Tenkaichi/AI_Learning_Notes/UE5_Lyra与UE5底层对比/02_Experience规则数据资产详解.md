# 02 — Experience 规则数据资产详解

> **定位**：Lyra 最核心的概念——**Experience（体验/规则数据资产）**。它是 Lyra 和 UE5 底层最大的区别。
>
> **一句话**：Experience = 用**数据资产（DataAsset）**定义的一整套游戏规则——用什么 GameMode、什么 Pawn、加载哪些 GameFeature、什么玩法。**换 Experience 就换整套规则**，比死板的 GameMode 灵活得多。
>
> **文件**：Lyra `LyraGame/Source/LyraGame/GameModes/`（`ULyraExperienceDefinition`）

---

## 一、先搞清：Experience 到底是什么

### 一句话

**Experience（体验）是一个数据资产**，它"打包"了一整套游戏规则。像一个"配方"或"剧本"，告诉 Lyra "这一局游戏该怎么玩"。

```
ULyraExperienceDefinition（数据资产）
  ├─ 用什么 GameMode
  ├─ 用什么 Pawn（玩家角色）
  ├─ 用什么 PlayerController
  ├─ 加载哪些 GameFeature（玩法模块）
  ├─ 用什么 UI
  └─ 什么规则（队伍/阶段）
```

### 类比

```
Experience = 一整套"游戏剧本"
  剧本A = 团队死斗（Team Deathmatch）
    角色：士兵
    规则：击杀得分，先到上限赢
    地图：城市
  剧本B = 大逃杀（Battle Royale）
    角色：幸存者
    规则：活到最后赢
    地图：荒岛

换剧本 = 换 Experience = 整局玩法全变
```

---

## 二、为什么 Experience 比 GameMode 好（核心对比）

这是 Lyra 和 UE5 底层最大的区别。

### UE5 底层：GameMode（规则写死）

```
AGameMode
  ├─ 规则写在代码里（改规则要改代码）
  ├─ 一个关卡 = 一个 GameMode
  └─ 换规则 = 换 GameMode（写死）
```

### Lyra：Experience（规则数据驱动）

```
ULyraExperienceDefinition（数据资产）
  ├─ 规则在数据资产里（改规则改数据，不改代码）
  ├─ 一个关卡可以换多个 Experience
  └─ 换规则 = 换 Experience（灵活）
```

| | UE5 GameMode | Lyra Experience |
|---|---|---|
| 规则在哪 | 代码写死 | **数据资产** |
| 改规则 | 改代码 | **改数据** |
| 换规则 | 换 GameMode | 换 Experience |
| 灵活度 | 低 | **高** |

---

## 三、Experience 里到底有什么（核心成员）

看 Lyra 源码 `ULyraExperienceDefinition`：

```cpp
// Lyra 源码（简化）
UCLASS()
class ULyraExperienceDefinition : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    // ① 这个体验用什么 GameMode
    UPROPERTY(EditDefaultsOnly, Category="Gameplay")
    TSubclassOf<AGameModeBase> GameModeClass;

    // ② 加载哪些 GameFeature 插件（玩法模块）
    UPROPERTY(EditDefaultsOnly, Category="Gameplay")
    TArray<ULyraExperienceActionSet*> ActionSets;

    // ③ 默认的地图/关卡
    UPROPERTY(EditDefaultsOnly, Category="Gameplay")
    TSoftObjectPtr<UWorld> DefaultMap;

    // ④ 规则（队伍、阶段等）—— 通过 GameFeature 加载
    // ...
};
```

**核心成员**：
| 成员 | 作用 |
|------|------|
| `GameModeClass` | 用哪个 GameMode |
| `ActionSets` | 加载哪些 GameFeature（玩法模块） |
| `DefaultMap` | 默认关卡 |
| 其他 | 队伍、阶段等规则 |

---

## 四、Experience 怎么工作（加载流程）

Experience 不是自己执行，而是由 **ExperienceManager** 加载并驱动：

```
游戏启动
  ↓
GameInstance → 创建 ExperienceManager
  ↓
ExperienceManager 加载指定的 Experience（数据资产）
  ↓
读取 Experience 内容：
  - 用指定的 GameMode
  - 加载指定的 GameFeature（玩法模块）
  - 生成玩家、建立规则
  ↓
整局游戏按这个 Experience 的规则运行
```

**核心**：**ExperienceManager（管理器）读取 Experience（数据资产）→ 按它配置启动游戏**。

---

## 五、具体场景：做两种玩法（换 Experience）

**场景：同一个地图，支持"团队死斗"和"大逃杀"两种玩法**

```
两个 Experience 资产：
  Experience_TeamDeathmatch（团队死斗）
    ├─ GameMode = TeamDeathmatchGameMode
    ├─ ActionSets = [击杀得分, 队伍系统]
    └─ 规则：先杀到 50 赢

  Experience_BattleRoyale（大逃杀）
    ├─ GameMode = BattleRoyaleGameMode
    ├─ ActionSets = [毒圈, 存活系统]
    └─ 规则：活到最后赢
```

**切换**：玩家选"大逃杀" → 加载 Experience_BattleRoyale → 整局按大逃杀规则跑。**不用改代码，只换数据资产。**

---

## 六、Experience 和 GameFeature 的关系（重要）

Experience 经常**搭配 GameFeature**（游戏特性插件）使用：

```
Experience（规则剧本）
  └─ 指定加载哪些 GameFeature（玩法模块）
       ├─ GameFeature_团队系统（队伍）
       ├─ GameFeature_枪械（武器）
       └─ GameFeature_阶段（游戏阶段）

换 Experience = 换加载的 GameFeature = 换玩法模块
```

**理解**：Experience 是"规则剧本"，GameFeature 是"可插拔的玩法模块"。Experience 决定加载哪些模块。

---

## 七、为什么说 Experience 是 Lyra 最大的区别

| 点 | 说明 |
|------|------|
| **数据驱动** | 规则在数据资产，不在代码 |
| **玩法可组合** | 用 GameFeature 组合不同玩法 |
| **换局灵活** | 换 Experience 换整局规则 |
| **生产级** | 展示 3A 游戏怎么做多玩法 |

**UE5 底层的 GameMode 做不到这么灵活**——这就是 Lyra 在引擎基础上"扩展"出的最大能力。

---

## 八、总结速查

```
Experience = 规则数据资产（Lyra 核心）
  ├─ 用什么 GameMode
  ├─ 加载哪些 GameFeature（玩法模块）
  ├─ 用什么 Pawn/UI
  └─ 什么规则

和 GameMode 区别：
  GameMode：规则写死代码，换规则换类
  Experience：规则数据驱动，换规则换数据

工作流程：
  GameInstance → ExperienceManager → 加载 Experience → 按规则启动

核心价值：数据驱动，玩法可组合，换局灵活
```

**一句话**：Experience 是 **Lyra 用数据资产定义的一整套游戏规则**（用什么 GameMode、加载哪些玩法模块、什么规则）。**比 UE5 死板的 GameMode 灵活**——换 Experience 就换整局玩法，配合 GameFeature 可以自由组合玩法模块。**这就是 Lyra 最大的区别。**

---

## 九、下一步

理解了 Experience，下一步可以深入 **ExperienceManager（怎么加载和驱动 Experience）** 或 **GameFeature（怎么把玩法拆成可插拔模块）**。
