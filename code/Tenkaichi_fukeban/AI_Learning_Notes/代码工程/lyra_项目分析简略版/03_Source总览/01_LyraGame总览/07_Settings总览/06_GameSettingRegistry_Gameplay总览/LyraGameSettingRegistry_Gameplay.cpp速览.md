# `LyraGameSettingRegistry_Gameplay.cpp` 速览

> 103 行，**最小的分类文件**。只有 `InitializeGameplaySettings` 一个函数。

## 两个分组

| 分组 | 设置项 |
|---|---|
| **Language** | `ULyraSettingValueDiscrete_Language`（自定义类） |
| **Replays** | 自动录制回放（开关）、保留回放数量（0~20，0 表示无限） |

> 💡 **两个值得注意的地方**：
>
> 1. **编辑器下的特殊提示**：语言设置在 PIE 里改了不生效，所以源码在 `WITH_EDITOR && GIsEditor` 时把描述文字换成一条黄色警告，告诉你要用 `-game` 启动或在编辑器偏好里改 PIE 语言。
>
> 2. **回放设置受平台限制**：用 `ULyraReplaySubsystem::GetPlatformSupportTraitTag()` 判断平台支不支持，不支持就整个隐藏。

**优先级**：`InitializeGameplaySettings`
