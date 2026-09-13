# 00a — 按 Lyra 写法开发前的准备

> **定位**：在正式动手写代码之前，先搞清楚"要用 Lyra 的方式做游戏，我需要准备什么、理解什么、配置什么"。
>
> **一句话**：Lyra 不是"拿来就能用"的框架，而是一套**架构范式**。要按它的写法开发，得先在思想上和工程上都做好准备。

---

## 一、思想准备：接受 Lyra 的核心理念

Lyra 的写法和你可能习惯的"直接堆功能"完全不同。开始前，先认同这几条：

| 理念 | 含义 | 你要做的改变 |
|------|------|-------------|
| **模块化组件** | 功能不堆在角色类里，而是拆成独立 Component | 忍住别往 Character 里塞逻辑，学会写 XxxComponent |
| **数据驱动** | 行为由 DataAsset 配置，不硬编码 | 习惯"先建数据资产，再写读取逻辑" |
| **GameplayTag 优先** | 用 Tag 标识一切（状态/类型/事件），不用 bool/枚举 | 放弃 `bIsDead`、`EWeaponType`，改用 Tag |
| **GAS 是一切战斗的基础** | 技能/伤害/属性/Buff 全走 GAS | 必须啃下 GAS（这是最大门槛） |
| **Experience 系统** | 游戏模式可热替换、数据驱动加载 | 理解"玩法是一套配置，不是一个关卡" |
| **GameFeature 插件化** | 功能以插件形式按需加载 | 接受"多项目录、多模块"的工程结构 |

> ⚠️ **最重要的一条**：Lyra 前期学习曲线陡峭（尤其 GAS）。不要指望"照着抄就能懂"，要做好"先理解概念，再动手"的心理准备。

---

## 二、知识准备：你该先学什么

按优先级排序，**没学完就别急着开工**：

### 1. UE5 引擎基础（必须）
- Actor / Pawn / Character 继承体系
- UPROPERTY / UFUNCTION / UCLASS 宏
- 组件（Component）机制
- 网络复制基础（Replication）
- 参考：本仓库 `UE5.6_深入理解/`

### 2. Gameplay Ability System / GAS（必须，最难）
Lyra 的战斗、属性、伤害、Buff、冷却**全部基于 GAS**。不懂 GAS = 看不懂 Lyra 一半以上的代码。
- ASC（AbilitySystemComponent）
- GameplayAbility / GameplayEffect / AttributeSet
- GameplayTag / GameplayTagContainer
- 参考：本仓库 `Lyra_项目分析/` 中 GAS 相关章节

### 3. Enhanced Input System（必须）
Lyra 用的是新版增强输入，不是旧版轴映射。
- InputAction / InputMappingContext
- 触发器（Trigger）与修饰器（Modifier）

### 4. Modular Gameplay 框架（建议）
Lyra 的角色/装备/物品都是 `ModularXxx` 派生，理解模块化框架才能看懂它的组合方式。
- ModularActor / ModularPawn / ModularCharacter
- GameFeature 插件机制

---

## 三、工程准备：环境配置清单

### 1. 安装 UE5
- 通过 Epic Games Launcher 安装 **UE 5.3+**（Lyra 官方推荐版本）
- 勾选 **C++ 桌面开发**工作负载（需要 Visual Studio）

### 2. 获取 Lyra 源码
- Lyra 是 **GitHub 上的示例项目**（需关联 Epic 账号才能访问）
- 从 GitHub 克隆，**不要用商城的空壳模板**（空壳不含完整源码）

### 3. 编译 Lyra
- 右键 `.uproject` → Generate Visual Studio project files
- 打开 `.sln`，选 **Development Editor / Win64**，编译
- 能成功运行 Lyra 的 PIE（Play In Editor）才算环境 OK

### 4. 建立你自己的工程
两种起点（推荐第①种）：
- ① **基于 Lyra 改**：复制 Lyra 工程改名，删掉不需要的内容
- ② **从空白 C++ 工程起**：完全自己搭，逐步引入 Lyra 的类

---

## 四、规范准备：养成 Lyra 的编码习惯

Lyra 有严格的命名和结构规范，**从一开始就遵守**，否则后期会非常混乱：

| 规范 | 规则 | 示例 |
|------|------|------|
| 类名前缀 | 项目名 + 类型前缀 | `ALyraCharacter`、`ULyraHealthComponent` |
| 布尔变量 | `b` 开头 | `bIsDead`、`bHasWeapon` |
| Getter 函数 | `Get{类型名}` | `GetLyraPlayerController()` |
| 蓝图覆写点 | `K2_` 前缀 | `K2_ActivateAbility()` |
| 网络回调 | `OnRep_` 前缀 | `OnRep_Health()` |
| 输入处理 | `Input_` 前缀 | `Input_Move()` |
| 静态查找组件 | `Find{类型}Component` | `FindHealthComponent()` |
| 目录结构 | 按功能分子目录 | `Character/`、`Ability/`、`Inventory/` |

---

## 五、心理准备：预期会遇到的坑

| 坑 | 说明 | 应对 |
|----|------|------|
| **GAS 劝退** | 概念多、调试难、初期看不到效果 | 先跑通最小 Demo（一个技能造成一次伤害）再扩展 |
| **初始化顺序** | Lyra 有 InitState 四阶段，乱序会崩 | 理解 Spawned→DataAvailable→DataInitialized→GameplayReady |
| **指针为空** | 网络环境下 Controller/State 可能晚到 | 永远判空，善用 `CastChecked` + `NullAllowed` |
| **依赖一堆插件** | Lyra 功能分散在多个 GameFeature | 别急着删，先搞清每个插件干什么 |
| **蓝图与 C++ 交织** | 很多逻辑在蓝图里 | 既要会读 C++，也要会看蓝图 |

---

## 六、开工前的自检清单

正式写第一行代码前，确认以下都打勾：

- [ ] UE5 已安装，Lyra 能编译运行
- [ ] 理解 Actor/Pawn/Character 区别
- [ ] 理解 UPROPERTY/UFUNCTION 基本用法
- [ ] 知道 GAS 四大件（ASC/GA/GE/AttributeSet）是什么
- [ ] 会用 Enhanced Input 绑定一个移动输入
- [ ] 接受"功能拆组件、数据用 DataAsset"的开发方式
- [ ] 记住了 Lyra 的命名规范

> 全部打勾 → 进入 **01_环境准备与工程搭建**，正式开始。
> 没打勾 → 回到对应章节补齐，磨刀不误砍柴工。

---

## 七、下一步

准备好之后，从 **01_环境准备与工程搭建** 开始，一步步从 0 搭起你的 Lyra 风格游戏。
