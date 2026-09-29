# `LyraTaggedActor.cpp` 速览

> 31 行，两个函数。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `GetOwnedGameplayTags()` | `TagContainer.AppendTags(StaticGameplayTags)` |
| `CanEditChange()`（编辑器） | 如果正在编辑的属性是 `AActor::Tags`，返回 false 禁止编辑；其余交给 Super |

> 💡 **为什么禁止 `AActor::Tags`**：UE 的 Actor 自带一个 `Tags` 数组（FName 类型，不是 GameplayTag）。Lyra 特意把它屏蔽掉，避免开发者误用 —— **统一用 `StaticGameplayTags` 这一个 GameplayTag 容器**，避免出现两套 Tag 系统。
