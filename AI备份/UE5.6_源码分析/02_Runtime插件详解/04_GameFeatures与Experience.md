# 04 - GameFeatures 与 Experience（动态加载机制）

> 涉及插件：`GameFeatures` (121 文件) + `CommonGame`
> Lyra 使用度：⭐⭐⭐ **核心**（三种游戏模式切换的秘密）

---

## 一、这是什么？

**GameFeatures** 是 Lyra 实现"玩法热插拔"的核心机制。它允许：
- 运行时**动态加载/卸载**一组资产和代码
- 不同游戏模式共享同一套基础工程
- 像 DLC 一样按需下载玩法

**Lyra 用它实现了**：
- FrontEnd 模式 → 加载大厅 GameFeature
- Elimination 模式 → 加载 ShooterCore GameFeature
- Control 模式 → 加载 TopDownArena GameFeature

---

## 二、核心概念

### 2.1 Experience Definition（体验定义）
**数据驱动的玩法配置**，继承 `ULyraExperienceDefinition`：

```cpp
// BP_Experience_Elimination（团队死斗）
UPROPERTY()
TArray<FGameFeatureActionEntry> Actions;  // 要执行哪些动作

UPROPERTY()
TArray<TSoftObjectPtr<UClass>> PawnDataClasses;  // 用什么 PawnData
```

### 2.2 GameFeature Action（动作）
**对游戏世界的修改操作**：

| Action | 作用 |
|--------|------|
| `AddAbilities` | 授予技能 |
| `AddWidgets` | 添加 UI 控件 |
| `AddInputBinding` | 绑定输入 |
| `SpawnActor` | 生成 Actor |
| `AddComponent` | 添加组件 |
| `SetWorldSettings` | 修改世界设置 |

### 2.3 GameFeature Plugin（插件）
**可动态加载的插件包**，包含：
- C++ 代码（可选）
- 蓝图资产
- 配置
- Content Bundle

---

## 三、工作流程

### 3.1 加载流程

```
1. GameMode 读取当前 Experience ID
2. ExperienceManagerComponent 加载 ExperienceDefinition
3. 遍历 Actions 数组
4. 对每个 Action：
   a. 加载对应 GameFeature 插件
   b. 挂载到游戏世界
   c. 执行 Action（添加技能/UI/输入等）
5. 所有 Action 完成 → 游戏就绪
```

### 3.2 网络同步

GameFeature 的加载是**服务器主导**的：
- 服务器决定加载哪个 Experience
- 客户端自动跟随（通过 GameState 复制）
- 确保所有客户端加载相同的 GameFeature

### 3.3 卸载流程

```
1. 游戏结束 / 切换模式
2. ExperienceManager 触发卸载
3. 逆序执行 Actions 的 Deactivate
4. 卸载 GameFeature 插件
5. 清理相关资产
```

---

## 四、目录结构

```
GameFeatures/
├── Source/
│   └── GameFeatures/
│       ├── Public/
│       │   ├── GameFeaturePluginSubsystem.h   ← 插件管理子系统
│       │   ├── GameFeatureAction.h            ← Action 基类
│       │   ├── GameFeatureData.h              ← 数据定义
│       │   └── ...
│       └── Private/
└── GameFeatures.uplugin
```

---

## 五、Lyra 中的实际例子

### 5.1 ShooterCore GameFeature

**包含内容**：
- 射击相关的 GA（跳跃、冲刺、射击）
- 武器定义（手枪、步枪、霰弹枪）
- UI 控件（准心、弹药计数）
- 输入绑定（射击、换弹、切换武器）

**激活时机**：玩家进入 Elimination 模式时

### 5.2 TopDownArena GameFeature

**包含内容**：
- 俯视角相机
- 炸弹人玩法逻辑
- 地图特定资产

**激活时机**：玩家进入 Control 模式时

---

## 六、与传统做法的对比

| 方面 | 传统做法 | GameFeature |
|------|---------|-------------|
| 玩法切换 | 重启游戏/加载关卡 | 运行时热切换 |
| DLC 分发 | 重新打包整个游戏 | 按需下载插件 |
| Mod 支持 | 难 | 易（插件化） |
| 内存占用 | 所有玩法都在内存 | 只加载当前玩法 |

---

## 七、学习建议

1. **先看 ExperienceDefinition** — 理解数据驱动
2. **跟踪一个完整流程** — 如从主菜单进入 Elimination
3. **看 ShooterCore 的结构** — 理解 GameFeature 如何组织
4. **动手实践** — 创建一个新的 GameFeature 插件

## 八、下一步

- [01_GameplayAbilities_GAS](./01_GameplayAbilities_GAS技能系统.md) — GAS 技能系统
- [02_CommonUI与UMG](./02_CommonUI与UMG.md) — UI 框架
- [03_ModularGameplay组件化](./03_ModularGameplay组件化.md) — 角色组件化
- [00_插件体系总览](./00_插件体系总览.md) — 回到总览
