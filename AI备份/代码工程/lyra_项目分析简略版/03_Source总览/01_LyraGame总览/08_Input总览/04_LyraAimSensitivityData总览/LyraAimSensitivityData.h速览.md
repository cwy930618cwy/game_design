# `LyraAimSensitivityData.h` 速览

> 一个很小的数据资产：**手柄灵敏度 10 档 → 浮点倍率** 的映射表。

| 成员 | 干嘛的 |
|---|---|
| `: UPrimaryDataAsset`（`BlueprintType, Const`） | 数据资产，只读 |
| `SensitivtyEnumToFloat()` | ⭐ 注意函数名**拼写少了个 i**（源码如此） |
| `SensitivityMap`（protected） | `TMap<ELyraGamepadSensitivity, float>` |

> 💡 它被 `ULyraInputModifierGamepadSensitivity` 引用（配在 `SensitivityLevelTable` 字段上）。
>
> 之所以做成数据资产而不是写死在代码里，是为了**让策划能调这 10 档分别对应多少倍率**，不用改代码。

**优先级**：`SensitivtyEnumToFloat`
