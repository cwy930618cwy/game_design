# `LyraAimSensitivityData.cpp` 速览

> 37 行，**最有价值的是构造函数里那张默认表**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | ⭐ 填 10 档默认值 |
| `SensitivtyEnumToFloat()` | 查表，**查不到返回 1.0**（安全兜底） |

## 默认的 10 档倍率

| 档位 | 倍率 |
|---|---|
| Slow | 0.5 |
| SlowPlus | 0.75 |
| SlowPlusPlus | 0.9 |
| **Normal** | **1.0** |
| NormalPlus | 1.1 |
| NormalPlusPlus | 1.25 |
| Fast | 1.5 |
| FastPlus | 1.75 |
| FastPlusPlus | 2.0 |
| Insane | 2.5 |

> 💡 注意 Normal 是 1.0（基准），往上到 2.5 倍，往下到 0.5 倍 —— 是个**非对称**的区间，低速段给得更细。
>
> 这张表是构造时写进 `SensitivityMap` 的，但字段是 `EditAnywhere`，所以**数据资产里可以覆盖**。
