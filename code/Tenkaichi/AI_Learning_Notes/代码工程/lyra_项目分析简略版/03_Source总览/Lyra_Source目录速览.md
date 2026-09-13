# `Source/` 目录速览

> 只到"目录是干嘛的"这一层，逐个文件后面单独拆。

## `Source/` 三部分

| | 文件数 | 一句话 |
|---|---|---|
| `LyraGame/` | 451 | 运行时模块，打包进游戏，**全部游戏逻辑在这** |
| `LyraEditor/` | 26 | 编辑器模块，打包时**不存在** |
| `*.Target.cs` | 10 | 编译目标（Client / Server / Editor / Game × Steam / EOS 组合） |

## `LyraGame/` 根目录的 7 个文件

| 文件 | 干嘛的 |
|---|---|
| `LyraGame.Build.cs` | 声明依赖哪些库 |
| `LyraGameModule.cpp/.h` | 模块启动/关闭回调 |
| `LyraGameplayTags.cpp/.h` | 全局 GameplayTag 声明 |
| `LyraLogChannels.cpp/.h` | 自定义日志通道 |
| `README.md` | 说明 |

## `LyraGame/` 的 26 个目录

### 骨架：怎么跑起来
| 目录 | 文件 | 一句话 |
|---|---|---|
| `System` | 27 | GameInstance / AssetManager / Session / ReplicationGraph |
| `GameModes` | 20 | Experience 机制，描述"一局游戏" |
| `GameFeatures` | 16 | 各种 `GameFeatureAction_*`，玩法往游戏里挂东西的方式 |
| `Player` | 16 | Controller / PlayerState / 出生点 / 作弊 |

### 玩法：能干什么
| 目录 | 文件 | 一句话 |
|---|---|---|
| `AbilitySystem` | 51 | GAS 技能系统 |
| `Teams` | 22 | 队伍分配与敌我判定 |
| `Character` | 17 | Pawn / Character / PawnData / PawnExtensionComponent |
| `Interaction` | 17 | 准星指向 + 按键交互 |
| `Inventory` | 16 | 背包与物品定义 |
| `Weapons` | 16 | 武器与开火 |
| `Equipment` | 12 | 装备管理（武器是装备的一种） |

### 表现：看到听到什么
| 目录 | 文件 | 一句话 |
|---|---|---|
| `UI` | 79 | 界面（HUD / 前端 / 通用控件 / 屏幕标记） |
| `Feedback` | 19 | 受击反馈：脚步声上下文特效 + 伤害飘字 |
| `Camera` | 12 | 相机模式与管理组件 |
| `Cosmetics` | 11 | 角色外观部件拼装 |
| `Messages` | 9 | 消息总线（击杀、通知广播） |
| `Audio` | 4 | 音频混音与设置 |
| `Animation` | 2 | `LyraAnimInstance`，动画主要在蓝图里 |

### 基建：给上面打杂
| 目录 | 文件 | 一句话 |
|---|---|---|
| `Settings` | 35 | 画质/音频/按键设置系统 |
| `Input` | 14 | 输入映射（基于 EnhancedInput） |
| `Hotfix` | 6 | 热更新（不改包改数值/文本） |
| `Performance` | 6 | 性能统计 |
| `Development` | 6 | 开发者设置、平台模拟、机器人作弊 |
| `Replays` | 4 | 录像回放 |
| `Tests` | 4 | 自动化测试入口 |
| `Physics` | 3 | 碰撞通道 + 带 Tag 的物理材质 |

## `LyraEditor/`（26 文件）

| 目录 | 一句话 |
|---|---|
| `Validation/` | 资源合规检查 |
| `Commandlets/` | 命令行批处理工具 |
| `Utilities/` | 一次性小工具 |
| `Private/` | 自定义资源类型 + 编辑器样式 |

## 速查：想改啥去哪

| 我想…… | 去哪 |
|---|---|
| 加 GameplayTag | `LyraGameplayTags.h` + `DefaultGameplayTags.ini` |
| 加技能 | 继承 `AbilitySystem/Abilities/LyraGameplayAbility.h` |
| 改伤害计算 | `AbilitySystem/Executions/LyraDamageExecution.cpp` |
| 加道具 | `Inventory/LyraInventoryItemDefinition` 数据资产 |
| 改游戏规则/地图/角色 | Experience 数据资产（见 `GameModes/`） |
| 加 HUD 面板 | `GameFeatureAction_AddWidget` |
| 改键 | `Input/LyraInputConfig` |
| 加设置项 | `Settings/LyraGameSettingRegistry_*` |
| 改第三人称相机 | `Camera/LyraCameraMode_ThirdPerson.cpp` |
| 加碰撞通道 | `Physics/LyraCollisionChannels.h` |
| 改启动加载 | `System/LyraGameInstance.cpp` / `LyraAssetManager.cpp` |
| 加第三方库 | `LyraGame.Build.cs` |

**优先级**：`System` → `GameModes` → `GameFeatures` → `AbilitySystem` → `UI` → `Character`
