# `LyraGameSettingRegistry.cpp` 速览

> 91 行。**真正的设置在别的 cpp 里，这里只负责串起来**。

| 函数 | 干嘛的 |
|---|---|
| `Get()` | 先 `FindObject` 找现成的，没有就 `NewObject` + `Initialize` |
| `IsFinishedInitializing()` | ⭐ Super 为真后，**还要确认 `GetSharedSettings() != nullptr`** —— 因为共享设置是异步加载的 |
| `OnInitialize()` | ⭐ 见下 |
| `SaveChanges()` | 依次：`LocalSettings->ApplySettings(false)` → `SharedSettings->ApplySettings()` → `SaveSettings()` |

## `OnInitialize()` 的组装顺序

```
VideoSettings = InitializeVideoSettings(...)       (在 _Video.cpp)
InitializeVideoSettings_FrameRates(...)            (在 _Video.cpp)
  → RegisterSetting(VideoSettings)

AudioSettings     = InitializeAudioSettings(...)           (_Audio.cpp)
GameplaySettings  = InitializeGameplaySettings(...)        (_Gameplay.cpp)
MouseAndKeyboard  = InitializeMouseAndKeyboardSettings(...) (_MouseAndKeyboard.cpp)
GamepadSettings   = InitializeGamepadSettings(...)          (_Gamepad.cpp)
```

> 💡 **这种"一个类拆成 6 个 cpp"的做法值得注意**：`ULyraGameSettingRegistry` 的完整实现分散在 `_Video` / `_Audio` / `_Gameplay` / `_MouseAndKeyboard` / `_Gamepad` / `_PerfStats` 六个文件里。因为每个分类都有几百行配置代码，放一起会是个巨型文件。
>
> 想加一个视频选项 → 去 `_Video.cpp`；想加一个手柄选项 → 去 `_Gamepad.cpp`。

**优先级**：`OnInitialize` → `IsFinishedInitializing`
