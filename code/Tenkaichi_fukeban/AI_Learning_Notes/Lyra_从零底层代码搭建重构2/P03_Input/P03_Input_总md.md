# P03 Input —— 总 md（为什么 + 全景 + 依赖分析）

> 依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\`（14 文件，8 个类）
> 你新建：`Source/TenkaichiGame/Input/` 下对应文件
> 前缀替换：`Lyra` → `Tenkaichi`（`ULyraInputModifiers`→`UTenkaichiInputModifiers`、`LyraAimSensitivityData.h`→`TenkaichiAimSensitivityData.h` 等，铁律 24/41）。
> **依赖结论（铁律 30，已查 `.h`+`.cpp` 真实 include）**：P03 全部依赖 **EnhancedInput 插件**（引擎自带，第 0 步已启用）+ 大量 **P18 Settings**（`LyraSettingsShared`/`LyraSettingsLocal`）+ **P12 Player**（`LyraLocalPlayer`）。P18/P12 未就位 → 相关 `.cpp` 段用 `#if 0` 挂起。

---

## 一、这个阶段解决什么问题（为什么）

**输入是玩家到游戏逻辑的第一站**。UE 默认输入系统是"轴映射/动作映射"（老式），Lyra 用 **EnhancedInput（增强输入）插件**重写，实现：

1. **`InputConfig`**（P03.1，已教）：把"输入动作"映射到"游戏逻辑标签"——玩家按"跳跃键"触发 `InputTag_Move`，具体逻辑由 `Tag` 驱动。
2. **`InputComponent`**（P03.1，已教）：把 `InputConfig` 绑定到 `EnhancedInputComponent`，监听动作并调用回调。
3. **`InputModifiers`**（P03.2，本阶段）：输入**修饰器**——在输入值到达游戏逻辑前做处理：死区（DeadZone）、摇杆灵敏度、瞄准反转、轴缩放。
4. **`AimSensitivityData`**（P03.2，本阶段）：灵敏度数据资产——把"灵敏度档位"映射到具体 float 值。
5. **`PlayerInput`**（P03.3，本阶段）：自定义玩家输入类——扩展 EnhancedInput，加输入延迟标记（latency flash）。
6. **`PlayerMappableKeyProfile`**（P03.3，本阶段）：按键配置档案——玩家自定义按键映射的"档案"。
7. **`InputUserSettings`**（P03.3，本阶段）：输入用户设置——按键映射/灵敏度等可存档设置。

> 🔑 **一句话**：EnhancedInput 插件 + Lyra 封装的输入配置/修饰器/按键映射，让"玩家按键 → 输入值 → 游戏逻辑"整条链路可配置、可调灵敏度、可自定义按键。

---

## 二、依赖全景（铁律 30 查证，按被依赖在下）

```
P03 Input
├─ P03.1 InputConfig / InputComponent（✅ 已教，重构版1）
├─ P03.2 InputModifiers / AimSensitivityData（🟡 本阶段）
│    ├─ EnhancedInput 插件（引擎，第 0 步已启用）✅
│    ├─ AimSensitivityData → Settings/LyraSettingsShared（P18）⚠️
│    └─ InputModifiers.cpp → LyraLocalPlayer（P12）+ LyraSettingsShared（P18）⚠️
└─ P03.3 PlayerInput / PlayerMappableKeyProfile / InputUserSettings（🟡 本阶段）
     ├─ EnhancedInput 插件（引擎）✅
     └─ PlayerInput.cpp → LyraSettingsLocal（P18）⚠️
```

### 具体每个文件的真实依赖（`.cpp` include 为准）

| 文件 | `.h` 依赖 | `.cpp` 依赖 | 状态 |
|------|-----------|------------|------|
| `LyraInputConfig` | 引擎（InputAction/InputTag） | 引擎 | ✅ P03.1 已教 |
| `LyraInputComponent` | 引擎（EnhancedInputComponent）+ `LyraInputConfig` | 引擎 + `LyraLocalPlayer`(已落盘✅) + `LyraSettingsLocal`(P18⚠️冗余include) | ✅ P03.1-2 已教（2026-10-27 补齐，`.cpp` 仅 1 行 include 挂起登记㉚） |
| `LyraInputModifiers` | 引擎（InputModifiers） | `LyraAimSensitivityData`(本阶段✅) + `LyraLocalPlayer`(P12⚠️) + `LyraSettingsShared`(P18⚠️) | 🟡 `.cpp` 依赖 P12/P18 挂起 |
| `LyraAimSensitivityData` | 引擎（DataAsset）+ 前置声明枚举 `ELyraGamepadSensitivity` | `LyraSettingsShared`(P18⚠️) | 🟡 `.h` 可独立编译（前置声明枚举不阻塞编译）；`.cpp` 构造函数体 + 查询函数体 + include 挂起（登记注释备忘录 ⑰） |
| `LyraPlayerInput` | 引擎（EnhancedPlayerInput） | `LatencyMarkerModule`(引擎✅) + `LyraSettingsLocal`(P18⚠️) | 🟡 `.cpp` 依赖 P18 挂起 |
| `LyraPlayerMappableKeyProfile` | 引擎（EnhancedInputUserSettings） | 引擎 | ✅ 可独立编译 |
| `LyraInputUserSettings` | 引擎（EnhancedInputUserSettings/PlayerMappableKeySettings） | 引擎 | ✅ 可独立编译 |

### 挂起总结（登记注释备忘录）

| 文件 | 挂起内容 | 依赖 | 解开时机 |
|------|---------|------|---------|
| `LyraAimSensitivityData.cpp` | `SensitivityMap` 初始化表 + `Settings/LyraSettingsShared.h` include | P18 Settings | P18 `LyraSettingsShared` 就位 |
| `LyraInputModifiers.cpp` | 依赖 `LyraLocalPlayer`(P12) + `LyraSettingsShared`(P18) 的函数段 + include | P12 + P18 | P12 `LyraLocalPlayer` + P18 `LyraSettingsShared` 就位 |
| `LyraPlayerInput.cpp` | 依赖 `LyraSettingsLocal`(P18) 的函数段 + include | P18 Settings | P18 `LyraSettingsLocal` 就位 |

> **挂起原则（铁律 33/34）**：`.h` 多数可独立编译（前置声明规避 include）；只有 `.cpp` 的硬调用决定编译/链接（铁律 30）。能独立编译的 `.h` 先教、不挂起，`.cpp` 里 P18/P12 相关段 `#if 0` 挂起并登记注释备忘录。

---

## 三、全景：本阶段 8 个类的职责

| 类 | 属于 | 一句话职责 |
|----|------|-----------|
| `ULyraInputConfig` | P03.1 | 输入动作 ↔ 游戏标签 映射表 |
| `ULyraInputComponent` | P03.1 | 把 InputConfig 绑定到增强输入，触发回调 |
| `ULyraSettingBasedScalar` | P03.2 | 输入修饰器：按设置属性缩放 XYZ 轴 |
| `ULyraInputModifierDeadZone` | P03.2 | 输入修饰器：死区（摇杆不响应的小区间） |
| `ULyraInputModifierGamepadSensitivity` | P03.2 | 输入修饰器：手柄灵敏度（查 AimSensitivityData） |
| `ULyraInputModifierAimInversion` | P03.2 | 输入修饰器：瞄准轴反转 |
| `ULyraAimSensitivityData` | P03.2 | 灵敏度档位 → float 值 的数据资产 |
| `ULyraPlayerInput` | P03.3 | 自定义玩家输入：加输入延迟标记 |
| `ULyraPlayerMappableKeyProfile` | P03.3 | 按键映射档案（装备/卸下） |
| `ULyraInputUserSettings` | P03.3 | 输入用户设置 + 按键设置工具提示 |

> 注：`LyraInputModifiers.h` 里定义了 **4 个修饰器类**（SettingBasedScalar / DeadZone / GamepadSensitivity / AimInversion）；`LyraInputUserSettings.h` 里定义了 **2 个类**（InputUserSettings + PlayerMappableKeySettings）。

---

## 四、教学节奏（铁律 29/35）

- ✅ **本总 md**（P03.0）：概念 + 全景 + 依赖。
- ✅ **P03.1**：`InputConfig` 已从重构版1 迁移确认（`P03.1_TenkaichiInputConfig_总md/_h/_cpp` 三件套，工程已落盘）。
- ✅ **P03.1-2**：`InputComponent` 已教完（2026-10-27 补齐 P03 最后缺口：`P03.1-2_TenkaichiInputComponent_总md/_h/_cpp` 三件套；工程 `TenkaichiInputComponent.h` 已落盘核对通过，`.cpp` 待落盘；`.h` 零挂起，`.cpp` 仅 `Settings/TenkaichiSettingsLocal.h` include 挂起登记㉚待 P18）→ **P03 Input 14/14 全清**。
- ✅ **P03.2**：`LyraAimSensitivityData` 已教完（`P03.2-1_..._总md/_h/_cpp`，`.cpp` 挂起 P18 登记⑰）；`LyraInputModifiers` 已教完（`P03.2-2_..._总md/_h/_cpp` 三件套，工程 `TenkaichiInputModifiers.h/.cpp` 已落盘，`.cpp` 挂起 P12+P18 登记⑱）。**P03.2 全清**。
- ✅ **P03.3**：`LyraPlayerInput` / `LyraPlayerMappableKeyProfile` / `LyraInputUserSettings` 已教完（`P03.3_Input类_总md/_h/_cpp` 三件套，工程三个文件 h+cpp 已落盘；`PlayerInput.cpp` 挂起 P18 登记⑲，另两个空壳类不挂起）。**P03 Input 文件夹全清**。

> 每个独立类都按铁律 35 先建它自己的总 md → 停 → 教 `.h` → 停 → 教 `.cpp`。

---

## 五、衔接说明

- P03 依赖 **EnhancedInput 插件**（第 0 步已启用，Build.cs 已加 `EnhancedInput`）——无新增插件。
- P03.2 的 `LyraInputModifiers` 依赖 `LyraAimSensitivityData`（本阶段内部），所以先教 AimSensitivityData 再教 InputModifiers。
- P03 的 `.cpp` 多处依赖 **P18 Settings**（`LyraSettingsShared`/`LyraSettingsLocal`）和 **P12 Player**（`LyraLocalPlayer`）——这两类未就位，相关段 `#if 0` 挂起，到 P12/P18 再解。

---

## 待确认

1. P03 Input 总 md 看懂了：EnhancedInput 插件 + Lyra 封装的输入配置/修饰器/按键映射；**P03.1 InputConfig 已迁移完成（重构版1）**，P03.2/P03.3 本阶段教；多处 `.cpp` 依赖 P18 Settings / P12 Player 挂起。
2. 确认后我按 **P03.2 先教 `LyraAimSensitivityData`**（它被 InputModifiers 依赖，先教它）。

---

## 源码依据

- Lyra 文件夹：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Input\`（14 文件）
- 枚举 `ELyraGamepadSensitivity` 定义在 `Settings\LyraSettingsShared.h`（P18 Settings）
- 依赖：`LyraInputModifiers.cpp` → `LyraLocalPlayer.h` + `LyraSettingsShared.h`；`LyraAimSensitivityData.cpp` → `LyraSettingsShared.h`；`LyraPlayerInput.cpp` → `LyraSettingsLocal.h`