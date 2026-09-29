# 01 — Lyra 与 UE5 底层对比总览

> **定位**：理清"**哪些是 UE5 引擎自带的（底层），哪些是 Lyra 自己加的（区别）**"。
>
> **一句话**：UE5 底层是**引擎提供的能力**（UObject/GameMode/GAS/CommonUI），Lyra 是**用这些能力搭出来的完整游戏**——它不改引擎，而是在引擎上**封装、扩展、组合**，形成一套可复用的"生产级游戏框架"。
>
> **文件**：Lyra 工程 `LyraGame/Source/` + `LyraGame/Plugins/`（对照 UE 引擎 `Engine/Source/`）

---

## 一、核心认知：Lyra 不改引擎，是在引擎上"加东西"

这是理解 Lyra 和 UE5 底层关系的关键：

```
UE5 引擎（底层）：提供机制（UObject/GameMode/GAS/CommonUI...）
    ↑ 不修改
Lyra（示例项目）：用这些机制搭完整游戏
    ├─ 继承引擎类（LyraGameMode 继承 GameMode）
    ├─ 扩展引擎功能（Experience 系统在 GameMode 上做）
    └─ 组合引擎能力（GAS + CommonUI + GameFeature）
```

**Lyra 的价值**：它展示了"**用 UE5 引擎做一个 3A 级游戏该怎么做**"——把引擎的散装能力，组装成一套完整、可复用、生产级的游戏框架。

---

## 二、对照表：UE5 底层 vs Lyra（核心区别）

| 系统 | UE5 底层 | Lyra 的区别 |
|------|---------|------------|
| **游戏规则** | `AGameMode`（一个关卡一个规则） | **Experience 系统**：数据驱动，换 Experience 换规则 |
| **玩家生成** | GameMode 直接生成 | `ULyraPlayerSpawningManagerComponent` 管理生成 |
| **技能** | GAS（ASC/GA/GE/AttributeSet） | 用 GAS，但封装 `ULyraGameplayAbility` 基类 |
| **输入** | Enhanced Input | 封装 `ULyraInputConfig`（Tag 映射输入） |
| **UI** | CommonUI | 封装 LyraUI + GameplayMessage 消息驱动 |
| **模块化** | GameFeature 插件 | 大量用 GameFeature 拆玩法/内容 |
| **Pawn** | APawn/ACharacter | `ALyraCharacter` + `ULyraPawnExtensionComponent` |

---

## 三、最核心的区别：Experience 系统（Lyra 最重要的"区别"）

**这是 Lyra 和 UE5 底层最大、最重要的不同。**

### UE5 底层：GameMode 定规则

```
UE5 默认：一个 GameMode = 一个关卡规则
  关卡1 → GameMode_A（规则A）
  关卡2 → GameMode_B（规则B）
  换规则 = 换 GameMode（较死板）
```

### Lyra：Experience 数据驱动规则

```
Lyra：Experience（体验）= 一组数据资产，定义了整套规则
  ULyraExperienceDefinition（DataAsset）
    ├─ 用什么 GameMode
    ├─ 用什么 Pawn
    ├─ 加载哪些 GameFeature
    ├─ 用什么输入/UI
    └─ 规则（Team/阶段）
  换 Experience = 换整套规则（数据驱动，灵活）
```

**对比**：

| | UE5 底层（GameMode） | Lyra（Experience） |
|---|---|---|
| 规则怎么定 | 代码里写死 | **数据资产**驱动 |
| 换规则 | 换 GameMode | 换 Experience（更灵活） |
| 能拆解 | 难 | 用 GameFeature 拆解 |

---

## 四、Lyra 的其他关键"区别"（概览）

### ① 玩家生成管理
```
UE5：GameMode 直接 ChoosePlayerStart
Lyra：ULyraPlayerSpawningManagerComponent（更灵活的生成管理）
```

### ② 输入系统
```
UE5：Enhanced Input（IA + IMC）
Lyra：ULyraInputConfig（用 GameplayTag 映射输入到动作）
   → 输入和 GA 通过 Tag 解耦
```

### ③ Pawn 扩展
```
UE5：APawn 挂组件
Lyra：ALyraPawn + ULyraPawnExtensionComponent（协调初始化）
   + ULyraHeroComponent（输入/相机绑定）
```

### ④ 模块化（GameFeature）
```
UE5：GameFeature 插件机制
Lyra：把玩法/内容拆成很多 GameFeature 插件（体验式模块化）
```

---

## 五、一张图：Lyra 在 UE5 之上的架构

```
┌─────────────────────────────────────────┐
│         Lyra（完整游戏框架）             │
│  Experience / GameFeature / LyraInput    │
│  LyraCharacter / LyraUI / GAS 封装       │
├─────────────────────────────────────────┤
│     UE5 引擎能力（底层，Lyra 复用）      │
│  UObject / GameMode / GAS / CommonUI     │
│  EnhancedInput / GameFeature / Pawn      │
├─────────────────────────────────────────┤
│        UE5 引擎核心（Core）              │
│  TArray / FString / 反射 / GC / 日志     │
└─────────────────────────────────────────┘
```

**分层**：
1. **Core（最底层）**：容器/字符串/反射/GC —— 你之前学的
2. **UE5 引擎能力**：GameMode/GAS/CommonUI —— 引擎提供
3. **Lyra（最上层）**：封装、扩展、组合 —— Lyra 的区别

---

## 六、学习 Lyra 该有的心态

**你之前学的是"引擎怎么用"（Core/CoreUObject/UI），Lyra 是"引擎怎么搭出一个游戏"**：

| 你学过的 | 对应 Lyra |
|---------|----------|
| AGameMode | LyraGameMode + Experience |
| ACharacter | LyraCharacter + PawnExtension |
| GAS | Lyra 的 GA/GE 封装 |
| CommonUI | LyraUI + GameplayMessage |
| UWorld | Lyra 的 GameInstance/Experience 加载 |

**所以**：你已经打好了引擎基础，学 Lyra 就是看"**Epic 官方怎么用这些引擎能力做一个完整游戏**"。

---

## 七、总结速查

```
Lyra vs UE5 底层：
  UE5 底层 = 引擎机制（GameMode/GAS/CommonUI）
  Lyra = 用机制搭游戏（Experience/封装/扩展）

核心区别：
  ① Experience（数据驱动规则，替代死板 GameMode）★最重要
  ② 玩家生成管理组件
  ③ InputConfig（Tag 映射输入）
  ④ PawnExtension（角色扩展）
  ⑤ GameFeature 模块化

关系：Lyra 不改引擎，在引擎上封装/扩展/组合
```

**一句话**：UE5 底层是**引擎机制**（GameMode/GAS/CommonUI），Lyra 是**用这些机制搭出来的完整游戏框架**。**最大区别是 Experience 系统**（数据驱动规则，比 GameMode 灵活），其他是各种封装和扩展。

---

## 八、下一步

理解了这个总览，下一步可以深入 Lyra 最核心的 **Experience 系统**（它是 Lyra 和 UE5 底层最大的区别），看它怎么用数据驱动取代死板的 GameMode。
