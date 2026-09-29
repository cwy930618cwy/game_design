# `LyraGameSettingRegistry_Gamepad.cpp` 速览

> 234 行。手柄设置。

| 内容 | 干嘛的 |
|---|---|
| 两个平台 Trait Tag | `Platform.Trait.Input.SupportsGamepad`、`...SupportsTriggerHaptics` |
| `InitializeGamepadSettings()` | ⭐ 见下 |

## 四个分组

| 分组 | 设置项 | 存在哪 |
|---|---|---|
| **Hardware** | 手柄型号（Xbox/PS 等）、震动、反转垂直、反转水平 | 型号是 **Local**，其余 **Shared** |
| **Controls** | ⚠️ **空分组** —— 只建了 Collection，没加任何设置项 |
| **Sensitivity** | 视角灵敏度、瞄准灵敏度（都是 1~10 预设） | Shared |
| **Controller DeadZone** | 左摇杆死区、右摇杆死区（0.05~0.95） | Shared |

## 两个细节

| 点 | 说明 |
|---|---|
| 手柄型号是动态枚举的 | 从 `UCommonInputPlatformSettings::GetControllerData()` 里挑出所有 `InputType == Gamepad` 的，**只有多于 1 个且平台允许切换时才显示这一项** |
| 灵敏度用自定义枚举 | `ELyraGamepadSensitivity` 有 10 档（Slow → Insane），显示文本写死在一个数组里（注意变量名还留着 `EFortGamepadSensitivity` 的 Fortnite 痕迹） |

> ⚠️ **"Controls" 分组是空的** —— 源码里建了 `GamepadBindingCollection` 但没往里加东西。和键鼠那边自动生成改键列表不同，**手柄改键界面没实现**。

**优先级**：`InitializeGamepadSettings`
