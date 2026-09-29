# `LyraActorUtilities.h` 速览

> 只干一件事：**把 `ENetMode` 包装成蓝图能用的枚举**。

| 成员 | 干嘛的 |
|---|---|
| `EBlueprintExposedNetMode` | 蓝图可见枚举：`Standalone` / `DedicatedServer` / `ListenServer` / `Client` |
| `SwitchOnNetMode()` | 传入 WorldContext，返回上面的枚举；`ExpandEnumAsExecs` 让蓝图出一个**四分支执行针** |

> 💡 枚举顺序有讲究，注释明确写了：**所有小于 `Client` 的值都算某种服务器**，所以 `NetMode < NM_Client` 是判断"我是服务器"的惯用法。

**说明**：Lyra 到处在区分客户端/服务器，这个枚举就是让蓝图也能优雅地做这个判断。
