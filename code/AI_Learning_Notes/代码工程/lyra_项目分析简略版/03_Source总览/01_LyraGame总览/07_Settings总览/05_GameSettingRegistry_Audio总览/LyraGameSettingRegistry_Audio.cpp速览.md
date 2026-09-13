# `LyraGameSettingRegistry_Audio.cpp` 速览

> 317 行。音频设置。**只有 `InitializeAudioSettings` 一个函数**（外加两个平台 Trait Tag）。

| 内容 | 干嘛的 |
|---|---|
| `TAG_Platform_Trait_SupportsChangingAudioOutputDevice` | 平台能不能换音频输出设备 |
| `TAG_Platform_Trait_SupportsBackgroundAudio` | 平台支不支持后台音频 |
| `InitializeAudioSettings()` | ⭐ 见下 |

## 两个分组

| 分组 | 设置项 | 存在哪 |
|---|---|---|
| **Volume** | 总音量、音乐、音效、对话、语音聊天（都是 0~1 滑条） | **Local** |
| **Sound** | 字幕子页（开关/大小/颜色/描边/背景不透明度）、音频输出设备、后台音频、3D 耳机、HDR 音频 | 字幕是 **Shared**，其余 Local |

> 💡 **注意字幕是一个 `UGameSettingCollectionPage`（子页面）**，不是平铺的设置项 —— 点进去是单独一页，符合常见游戏的习惯。

> ⚠️ 一处小瑕疵：3D 耳机那项的 **Getter 和 Setter 都填了 `bDesiredHeadphoneMode`**（应该是 `IsHeadphoneModeEnabled` / `SetHeadphoneModeEnabled`）。HDR 音频那项则是正常的 Get/Set 配对。

**优先级**：`InitializeAudioSettings`
