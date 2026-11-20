# 02 — Lyra 如何扩展 UE5 底层

> **定位**：深入总览里的核心认知——"**Lyra 不改引擎，是在引擎上'加东西'**"。这篇讲 Lyra 具体怎么在引擎上加（继承/扩展/组合）。
>
> **一句话**：Lyra 对 UE5 底层做三件事——**继承引擎类**（改造）、**扩展引擎功能**（增强）、**组合引擎能力**（整合）。这三招就是 Lyra 全部"区别"的来源。
>
> **对照**：总览第 15-22 行

---

## 一、Lyra 加东西的三种方式（核心框架）

Lyra 对 UE5 底层就做**三件事**：

```
① 继承引擎类（改造）—— 继承 GameMode/ACharacter/Controller，加自己的逻辑
② 扩展引擎功能（增强）—— 在引擎类上挂组件/加系统（如 Experience）
③ 组合引擎能力（整合）—— 把 GAS/CommonUI/EnhancedInput 组合成一套玩法
```

**这三招就是 Lyra 全部"区别"的来源**。下面逐个展开。

---

## 二、方式一：继承引擎类（改造）

Lyra 大量**继承引擎类**，在子类里加自己的逻辑。

### 例：LyraGameMode 继承 AGameMode

```cpp
// UE5 底层：AGameMode（游戏规则基类）
class AGameMode : public AInfo { ... };

// Lyra：继承 AGameMode，加自己的规则逻辑
UCLASS()
class ALyraGameMode : public AGameMode {
    GENERATED_BODY()
public:
    // 在引擎 GameMode 基础上，加 Lyra 特有的逻辑
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    // 用 Experience 来初始化规则（而不是直接写死）
};
```

**核心**：Lyra 不改 `AGameMode` 源码，而是**继承它**，在子类里加逻辑。**引擎保持原样，Lyra 在子类做自己的事。**

### 例：LyraCharacter 继承 ACharacter

```cpp
// UE5 底层：ACharacter（人形角色）
// Lyra：继承 ACharacter，加扩展组件
UCLASS()
class ALyraCharacter : public ACharacter {
    GENERATED_BODY()
public:
    // 挂 Lyra 的扩展组件（协调初始化）
    UPROPERTY()
    ULyraPawnExtensionComponent* PawnExtComponent;
};
```

**方式一的本质**：`LyraXxx : public UEngineXxx`——**继承引擎类，加自己的成员和逻辑。**

---

## 三、方式二：扩展引擎功能（增强）

Lyra 在引擎类上**挂组件、加系统**，增强引擎原本的能力。

### 例：Experience 系统（在 GameMode 上扩展）

UE5 的 GameMode 规则是"写死的"，Lyra 通过挂一个组件来扩展成"数据驱动"：

```cpp
// UE5 底层：GameMode 规则写死
// Lyra 扩展：GameMode 上挂一个 ExperienceManager 组件
UCLASS()
class ALyraGameMode : public AGameMode {
    GENERATED_BODY()
public:
    // 挂一个组件来管理 Experience（规则数据资产）
    UPROPERTY()
    ULyraExperienceManagerComponent* ExperienceComponent;
};
```

**核心**：Lyra 不重写 GameMode 的全部，而是**挂组件**增强它——把"写死规则"升级成"数据驱动规则"。

### 例：PlayerController 挂 HeroComponent

```cpp
// Lyra 在 PlayerController 上挂 HeroComponent
// 负责输入绑定 + 相机控制
UCLASS()
class ALyraPlayerController : public APlayerController {
    GENERATED_BODY()
public:
    UPROPERTY()
    ULyraHeroComponent* HeroComponent;   // 扩展输入/相机
};
```

**方式二的本质**：**挂组件、加子系统**，增强引擎类原本的能力，而不重写它。

---

## 四、方式三：组合引擎能力（整合）

Lyra 把引擎的散装能力（GAS、CommonUI、EnhancedInput、GameFeature）**组合成一套完整玩法**。

### 例：一个 Lyra 角色 = 组合多种引擎能力

```
一个 LyraCharacter = 组合：
  ├─ ACharacter（引擎：人形角色）
  ├─ GAS（引擎：技能系统）→ 放技能
  ├─ EnhancedInput（引擎：输入）→ 操作
  ├─ CommonUI（引擎：UI）→ 显示
  └─ GameFeature（引擎：模块化）→ 加载玩法
```

### 例：输入和技能通过 Tag 组合

```cpp
// Lyra 把"输入动作"和"技能"通过 Tag 组合起来
// 按"跳跃"输入 → 触发 Tag = "Ability.Jump" 的技能
```

**核心**：Lyra 不发明新引擎能力，而是**把引擎已有的能力组装成游戏系统**。这是"组合"而非"发明"。

---

## 五、三种方式对比（什么时候用哪个）

| 方式 | 做法 | 例子 | 本质 |
|------|------|------|------|
| **① 继承** | 继承引擎类加逻辑 | LyraGameMode 继承 GameMode | 改造 |
| **② 扩展** | 挂组件/子系统 | Experience 挂 GameMode 上 | 增强 |
| **③ 组合** | 组合多个引擎能力 | 角色 = GAS+Input+UI | 整合 |

**记忆**：
```
① 继承 = 换个"更强的 GameMode"
② 扩展 = 给 GameMode"加个零件"
③ 组合 = 把好几个零件"装成一台机器"
```

---

## 六、为什么 Lyra 要这样（不改引擎的好处）

Lyra 不直接改引擎源码，而是用继承/扩展/组合，因为：

| 好处 | 说明 |
|------|------|
| **升级兼容** | 引擎升级，Lyra 代码不用大改 |
| **不污染引擎** | 引擎保持通用，Lyra 的改动只在自己工程 |
| **可复用** | Lyra 的封装能复制到新项目 |
| **最佳实践** | 展示"生产级游戏该怎么做" |

---

## 七、总结速查

```
Lyra 加东西的三招：
  ① 继承引擎类（改造）→ LyraGameMode : public AGameMode
  ② 扩展引擎功能（增强）→ 挂 Experience/PawnExtension 组件
  ③ 组合引擎能力（整合）→ 角色 = GAS + Input + UI

记忆：继承=更强的类，扩展=加零件，组合=装机器
价值：不改引擎，升级兼容，可复用，最佳实践
```

**一句话**：Lyra 对 UE5 底层做**三件事——继承（改造）、扩展（增强）、组合（整合）**。这就是它"区别"的全部来源。**它不改引擎源码，而是在引擎类上继承、挂组件、组合能力**，形成一套可复用的生产级游戏框架。

---

## 八、下一步

理解了 Lyra 怎么"加东西"，下一步深入**最核心的 Experience 系统**——它是 Lyra "扩展"UE5 GameMode 的最佳范例，也是 Lyra 最大的区别。
