# `LyraInputModifiers.h` 速览

> **一个头文件里放了 4 个输入修饰器**，都继承 `UInputModifier`。它们的作用都是"把玩家设置应用到原始输入上"。

| 类 | 显示名 | 干嘛的 |
|---|---|---|
| `ULyraSettingBasedScalar` | Setting Based Scalar | ⭐ 按 Shared 设置里的**三个浮点属性**分别缩放 X/Y/Z |
| `ULyraInputModifierDeadZone` | Lyra Settings Driven Dead Zone | ⭐ 死区，阈值来自 Shared 设置 |
| `ULyraInputModifierGamepadSensitivity` | Lyra Gamepad Sensitivity | ⭐ 手柄灵敏度倍率 |
| `ULyraInputModifierAimInversion` | Lyra Aim Inversion Setting | 反转 X/Y 轴 |

## 两个枚举

| 枚举 | 取值 |
|---|---|
| `EDeadzoneStick` | `MoveStick`（左摇杆）/ `LookStick`（右摇杆） |
| `ELyraTargetingType` | `Normal` / `ADS`（瞄准时的灵敏度） |

## `ULyraSettingBasedScalar` 的配置

| 字段 | 干嘛的 |
|---|---|
| `XAxisScalarSettingName` / `Y...` / `Z...` | ⭐ **填属性名字符串**（如 `MouseSensitivityX`），运行时反射去 Shared 设置上取 |
| `MaxValueClamp` / `MinValueClamp` | 缩放值的上下限（默认 0~10） |
| `PropertyCache`（protected） | ⭐ 缓存 `FProperty*`，避免每帧反射查找 |

## `ULyraInputModifierDeadZone` 的配置

| 字段 | 干嘛的 |
|---|---|
| `Type` | `EDeadZoneType`（Axial 分轴 / Radial 径向） |
| `UpperThreshold` | 超过多少就钳到 1（默认 1.0） |
| `DeadzoneStick` | 用哪个摇杆的死区设置 |

> 💡 **`ULyraSettingBasedScalar` 是最灵活的那个** —— 它不写死"要读哪个设置"，而是让你填属性名，运行时用 `FindPropertyByName` + `ContainerPtrToValuePtr` 反射取值。所以鼠标灵敏度、手柄灵敏度都能用同一个修饰器，只要配不同的属性名。

**优先级**：`ULyraSettingBasedScalar` → `ULyraInputModifierDeadZone`
