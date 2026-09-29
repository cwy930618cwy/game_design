# `LyraAbilitySourceInterface.cpp` 速览

> 11 行，**只有一个空构造函数**。

| 内容 | 干嘛的 |
|---|---|
| `ULyraAbilitySourceInterface` 构造函数 | UINTERFACE 机制要求的标配实现，什么都不做 |

**说明**：这个文件没有任何逻辑。接口的两个纯虚函数 `GetDistanceAttenuation` 和 `GetPhysicalMaterialAttenuation` 由**实现方**去写（比如 `LyraRangedWeaponInstance`）。
