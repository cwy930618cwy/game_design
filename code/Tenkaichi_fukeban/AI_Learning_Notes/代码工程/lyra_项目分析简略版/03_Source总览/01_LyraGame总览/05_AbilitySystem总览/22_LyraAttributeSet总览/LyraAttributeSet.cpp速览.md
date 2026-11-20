# `LyraAttributeSet.cpp` 速览

> 29 行，三个函数，**没有任何业务逻辑**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `GetWorld()` | 从 `GetOuter()` 上取 World —— 属性集本身不是 Actor，拿不到 World，这样补上 |
| `GetLyraAbilitySystemComponent()` | `Cast<ULyraAbilitySystemComponent>(GetOwningAbilitySystemComponent())` |

**说明**：看这个文件主要是确认"基类没藏东西"，实际属性逻辑都在子类（`LyraHealthSet` / `LyraCombatSet`）。
