# `LyraInputModifiers.cpp` 速览

> 201 行。**四个修饰器共用一个辅助函数**。

| 内容 | 干嘛的 |
|---|---|
| `LyraInputModifiersHelpers::GetLocalPlayer()` | ⭐ 从 `PlayerInput` 反查 `ULyraLocalPlayer`（`GetOuter()` → PC → LocalPlayer） |
| `ULyraSettingBasedScalar::ModifyRaw_Implementation()` | ⭐ 见下 |
| `ULyraInputModifierDeadZone::ModifyRaw_Implementation()` | ⭐ 见下 |
| `...DeadZone::GetVisualizationColor_Implementation()` | 调试可视化：被吃掉显示红色 |
| `ULyraInputModifierGamepadSensitivity::...` | 取 Shared 里的档位（Normal 或 ADS 两套）→ 查表得倍率 → 乘上去 |
| `ULyraInputModifierAimInversion::...` | 按设置把 X 和/或 Y 取负 |

## `ULyraSettingBasedScalar` 的流程

```
1. 布尔类型的输入不支持（ensureMsgf 报错）
2. 取 LocalPlayer → GetSharedSettings()
3. 按属性名反射找 FProperty（★ 只找一次，之后用 PropertyCache）
4. 按输入维度（1D/2D/3D）依次取对应轴的缩放值
5. 每个轴钳到 [MinValueClamp, MaxValueClamp]
6. 原值 × 缩放向量
```

> ⚠️ **缓存逻辑有个小瑕疵**：`bHasCachedProperty = PropertyCache.Num() == 3`，但填充缓存的判断是 `PropertyCache.IsEmpty()`。如果某次查找只填了部分……实际上两者配合不会出问题，但读起来容易困惑。

## `ULyraInputModifierDeadZone` 的核心公式

```cpp
// 平移 + 缩放，把去掉死区后的输入重新映射到 0~1
Min(1, Max(0, |AxisVal| - LowerThreshold) / (UpperThreshold - LowerThreshold)) * Sign(AxisVal)
```

> 💡 **这个"平移再缩放"很关键**：如果只是简单地把小于阈值的输入归零，摇杆推过死区边界时会**突然跳变**。减掉阈值再除以剩余区间，就能平滑过渡。源码注释里还给了参考文章链接（Gamasutra 的《Doing Thumbstick Dead Zones Right》）。

**优先级**：`GetLocalPlayer` → `SettingBasedScalar` → 死区公式
