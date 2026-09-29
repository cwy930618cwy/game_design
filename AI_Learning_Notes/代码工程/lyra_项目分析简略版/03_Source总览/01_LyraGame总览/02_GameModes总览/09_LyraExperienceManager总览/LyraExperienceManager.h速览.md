# `LyraExperienceManager.h` 速览

> 一个 `UEngineSubsystem`，**只在编辑器里真正干活** —— 协调多个 PIE 会话之间谁有权关闭某个 GameFeature 插件。

| 成员 | 干嘛的 |
|---|---|
| `: UEngineSubsystem` | 引擎级单例，跨世界存在 |
| `OnPlayInEditorBegun()` | PIE 开始时清空计数表 |
| `NotifyOfPluginActivation()` | 有人要启用某插件 → 计数 +1 |
| `RequestToDeactivatePlugin()` | 有人要停用某插件 → 计数 -1，**只有减到 0 才返回 true** |
| `GameFeaturePluginRequestCountMap` | `TMap<FString, int32>`：插件 URL → 当前有几个请求者在用 |

> ⚠️ 注意：非编辑器版本（`#else` 分支）里这两个函数是**空内联**——`NotifyOfPluginActivation` 什么都不做，`RequestToDeactivatePlugin` 恒返回 true。

**说明**：不要看名字误以为是加载 Experience 的管理器 —— **那个是 `ULyraExperienceManagerComponent`**。这个只是插件引用计数器。
