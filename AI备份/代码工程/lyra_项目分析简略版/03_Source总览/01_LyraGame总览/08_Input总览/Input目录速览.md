# `Input/` 目录速览

> 7 个类，14 个文件，**没有子目录**。建立在 **EnhancedInput** 之上。

## 目录结构

```
Input/
├── LyraInputConfig           ← 数据资产：InputAction ↔ GameplayTag 的映射表
├── LyraInputComponent        ← 绑定器：把 config 里的动作绑到函数/技能
├── LyraInputModifiers        ← 4 个输入修饰器（灵敏度、死区、反转）
├── LyraAimSensitivityData    ← 灵敏度档位表（1~10 档 → 浮点倍率）
├── LyraInputUserSettings     ← 改键设置（玩家自定义按键）
├── LyraPlayerMappableKeyProfile ← 改键配置档
└── LyraPlayerInput           ← 在按键时打"延迟标记"
```

## 7 个类

| 类 | 一句话 |
|---|---|
| `LyraInputConfig` | ⭐⭐ 数据资产，**把 InputAction 和 GameplayTag 绑在一起** |
| `LyraInputComponent` | ⭐ 组件：按 Tag 绑定原生动作 + 批量绑定技能 |
| `LyraInputModifiers` | ⭐ 4 个修饰器：按设置缩放、死区、手柄灵敏度、反转轴 |
| `LyraAimSensitivityData` | 灵敏度档位 → 浮点倍率的数据表 |
| `LyraInputUserSettings` | ⚠️ 改键用户设置 —— **只有一个 Super 调用** |
| `LyraPlayerMappableKeyProfile` | ⚠️ 改键配置档 —— **只有两个 Super 调用** |
| `LyraPlayerInput` | 按键时触发延迟闪烁标记（配合 Reflex 测延迟） |

## 补充说明

| 点 | 说明 |
|---|---|
| 输入到技能是**走 Tag** 的 | `AbilityInputActions` 里每个动作配一个 `InputTag`，技能那边用同样的 Tag 匹配 —— 改键不影响任何技能代码 |
| 两套动作列表 | `NativeInputActions`（C++ 手动绑）+ `AbilityInputActions`（自动绑给技能） |
| 修饰器都去 Shared 设置取值 | 死区、灵敏度、反转全都从 `ULyraSettingsShared` 读，所以能按玩家存、能云同步 |
| 有 3 个类是"空壳/占位" | `LyraInputUserSettings` / `LyraPlayerMappableKeyProfile` / `LyraInputComponent` 的映射函数 —— 都只调 Super，源码留了注释让你自己扩展 |

**优先级**：`LyraInputConfig` → `LyraInputComponent` → `LyraInputModifiers` → `LyraAimSensitivityData`
