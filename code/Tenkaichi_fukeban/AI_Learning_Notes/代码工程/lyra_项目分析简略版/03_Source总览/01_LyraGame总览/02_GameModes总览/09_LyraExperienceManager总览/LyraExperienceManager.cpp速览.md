# `LyraExperienceManager.cpp` 速览

> 55 行，**整个实现包在 `#if WITH_EDITOR` 里**，打包出去等于空实现。

| 函数 | 干嘛的 |
|---|---|
| `OnPlayInEditorBegun()` | `ensure` 当前表是空的，然后清空（防止上个 PIE 残留） |
| `NotifyOfPluginActivation()` | 只在 `GIsEditor` 时生效：找到本引擎子系统，把该插件 URL 的计数 +1 |
| `RequestToDeactivatePlugin()` | 计数 -1；**减到 0 才从表里移除并返回 true**，否则返回 false |

## 为什么需要它

```
多个 PIE 窗口同时跑
   ├─ 客户端 PIE → 要启用插件 ShooterCore
   └─ 服务器 PIE → 也要启用插件 ShooterCore
        ↓ 计数 = 2
其中一个结束了 → 计数 = 1 → 不能关（另一个还在用）
两个都结束     → 计数 = 0 → 才可以真的关掉
```

> 💡 注意 `-1` 用的是 `FindChecked`（不是 Find），所以**没登记过的插件来请求停用会直接断言失败**。正常流程下这不会发生，因为增和减是配对的。

**说明**：非编辑器构建下，`RequestToDeactivatePlugin` 恒为 true —— 因为打包后不存在多 PIE 抢插件的问题。
