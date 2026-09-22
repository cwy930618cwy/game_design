# `Settings/` 目录速览

> 20 个类，35 个文件，**3 个子目录**。建立在 **GameSettings** 插件之上。

## 目录结构

```
Settings/
├── 根目录（3 个类 + 6 个分类 cpp）
│   ├── LyraSettingsLocal              ← 本机设置（画质/音量，存本地）
│   ├── LyraSettingsShared             ← 跨设备共享设置（改键/字幕，可云同步）
│   └── LyraGameSettingRegistry        ← 设置页的"组装图纸"
│       ├── _Video.cpp / _Audio.cpp / _Gameplay.cpp
│       └── _MouseAndKeyboard.cpp / _Gamepad.cpp / _PerfStats.cpp
│
├── CustomSettings/  (8)  ← 引擎没有的特殊设置项
├── Screens/         (2)  ← 亮度、安全区两个独立编辑页
└── Widgets/         (1)  ← 改键用的列表条目
```

## 核心设计：三件套

| 类 | 干嘛的 | 存在哪 |
|---|---|---|
| `LyraSettingsLocal` | 和**这台机器**绑定的设置（分辨率、画质、音量） | `UGameUserSettings`，本地 ini |
| `LyraSettingsShared` | 和**这个玩家**绑定的设置（改键、字幕、灵敏度） | `ULocalPlayerSaveGame`，**可云同步** |
| `LyraGameSettingRegistry` | 把上面两者组装成设置页的树 | 运行时 NewObject |

> 💡 **为什么要分成两个**：源码注释解释得很清楚 —— 改键这类偏好如果放在本机设置里，**同一台机器上所有玩家都会共享**。所以必须放在按玩家存储的 SaveGame 里，才能云同步、才能每人不同。

## 3 个子目录

| 子目录 | 类数 | 内容 |
|---|---|---|
| `CustomSettings/` | 8 | `LyraSettingValueDiscrete_Language`、`_Resolution`、`_OverallQuality`、`_MobileFPSType`、`_PerfStat`、`LyraSettingValueDiscreteDynamic_AudioOutputDevice`、`LyraSettingKeyboardInput`、`LyraSettingAction_SafeZoneEditor` |
| `Screens/` | 2 | `LyraBrightnessEditor`、`LyraSafeZoneEditor` |
| `Widgets/` | 1 | `LyraSettingsListEntrySetting_KeyboardInput` |

## 补充说明

| 点 | 说明 |
|---|---|
| 设置项怎么知道读写哪个字段 | 靠两个宏：`GET_LOCAL_SETTINGS_FUNCTION_PATH` / `GET_SHARED_SETTINGS_FUNCTION_PATH` |
| 设置项能被平台条件禁用 | 用 `FWhenPlatformHasTrait::KillIfMissing` 按平台 Trait 隐藏 |
| 设置项之间有依赖 | `AddEditDependency`（如"分辨率"依赖"窗口模式"） |
| 目录里有个 `README.md` | 内容只有一个词：`Todo` |

**优先级**：`LyraSettingsLocal` → `LyraSettingsShared` → `LyraGameSettingRegistry`
