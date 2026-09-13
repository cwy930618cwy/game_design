# `LyraGame/` 目录速览

> 451 个文件 / 26 个目录 / 7 个根文件。逐个文件夹后面单独拆。

## 根文件

| 文件 | 干嘛的 |
|---|---|
| `LyraGame.Build.cs` | 声明依赖哪些库 |
| `LyraGameModule.cpp/.h` | 模块启动/关闭回调 |
| `LyraGameplayTags.cpp/.h` | 全局 GameplayTag 声明 |
| `LyraLogChannels.cpp/.h` | 自定义日志通道 |
| `README.md` | 说明 |

## 26 个目录，按四组看

### 骨架：怎么跑起来
| 目录 | 文件 | 一句话 |
|---|---|---|
| `System` | 27 | GameInstance / AssetManager / Session / ReplicationGraph |
| `GameModes` | 20 | Experience 机制，描述"一局游戏" |
| `GameFeatures` | 16 | `GameFeatureAction_*`，玩法挂进游戏的方式 |
| `Player` | 16 | Controller / PlayerState / 出生点 / 作弊 |

### 玩法：能干什么
| 目录 | 文件 | 一句话 |
|---|---|---|
| `AbilitySystem` | 51 | GAS 技能系统 |
| `Teams` | 22 | 队伍分配与敌我判定 |
| `Character` | 17 | Pawn / Character / PawnData |
| `Interaction` | 17 | 准星指向 + 按键交互 |
| `Inventory` | 16 | 背包与物品定义 |
| `Weapons` | 16 | 武器与开火 |
| `Equipment` | 12 | 装备管理 |

### 表现：看到听到什么
| 目录 | 文件 | 一句话 |
|---|---|---|
| `UI` | 79 | 界面最大一块 |
| `Feedback` | 19 | 受击反馈、飘字 |
| `Camera` | 12 | 相机模式 |
| `Cosmetics` | 11 | 角色外观部件 |
| `Messages` | 9 | 消息总线 |
| `Audio` | 4 | 音频混音 |
| `Animation` | 2 | 动画主要在蓝图里 |

### 基建
| 目录 | 文件 | 一句话 |
|---|---|---|
| `Settings` | 35 | 画质/音频/按键设置 |
| `Input` | 14 | 输入映射 |
| `Hotfix` | 6 | 热更新 |
| `Performance` | 6 | 性能统计 |
| `Development` | 6 | 开发者设置、作弊 |
| `Replays` | 4 | 录像回放 |
| `Tests` | 4 | 自动化测试 |
| `Physics` | 3 | 碰撞通道、物理材质 |

**优先级**：`System` → `GameModes` → `GameFeatures` → `AbilitySystem` → `UI` → `Character`
