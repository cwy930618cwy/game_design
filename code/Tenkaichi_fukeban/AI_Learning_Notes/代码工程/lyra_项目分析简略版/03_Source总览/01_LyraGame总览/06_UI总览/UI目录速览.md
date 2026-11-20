# `UI/` 目录速览

> 40 个类，79 个文件，**8 个子目录**。全项目最大的目录。

## 目录结构

```
UI/
├── 根目录（9 个）        ← HUD 骨架与基类
├── Basic/            (1)  ← 材质进度条
├── Common/           (6)  ← 通用控件：列表、Tab、按钮
├── Foundation/       (5)  ← 基础组件：按钮基类、确认框、加载界面
├── Frontend/         (3)  ← 前端/主菜单
├── IndicatorSystem/  (6)  ← 屏幕标记（把世界中 Actor 投影成 HUD 图标）
├── PerformanceStats/ (2)  ← 性能面板
├── Subsystem/        (2)  ← UI 总管 + 消息
└── Weapons/          (6)  ← 准星、命中标记（含 Slate 手写版）
```

## 根目录 9 个类

| 类 | 一句话 |
|---|---|
| `LyraActivatableWidget` | ⭐⭐ **所有界面的基类**，决定打开时用什么输入模式 |
| `LyraHUD` | HUD Actor，注释说**基本不用动它**（HUD 内容用 Add Widget 加） |
| `LyraHUDLayout` | ⭐ HUD 整体布局：ESC 菜单 + 手柄断连提示 |
| `LyraTaggedWidget` | ⚠️ 按 Tag 显隐的控件 —— **功能没实现完** |
| `LyraGameViewportClient` | 决定用软件光标还是硬件光标 |
| `LyraSettingScreen` | 设置页 Tab 容器 |
| `LyraSimulatedInputWidget` | ⭐ 触摸模拟输入的基类（移动端虚拟摇杆的基础） |
| `LyraTouchRegion` | 触摸区域（按住就持续注入输入） |
| `LyraJoystickWidget` | 虚拟摇杆 |

## 8 个子目录

| 子目录 | 类数 | 内容 |
|---|---|---|
| `Basic/` | 1 | `MaterialProgressBar` |
| `Common/` | 6 | `LyraListView`、`LyraTabListWidgetBase`、`LyraTabButtonBase`、`LyraBoundActionButton`、`LyraWidgetFactory(_Class)` |
| `Foundation/` | 5 | `LyraButtonBase`、`LyraActionWidget`、`LyraConfirmationScreen`、`LyraControllerDisconnectedScreen`、`LyraLoadingScreenSubsystem` |
| `Frontend/` | 3 | `LyraFrontendStateComponent`、`LyraLobbyBackground`、`ApplyFrontendPerfSettingsAction` |
| `IndicatorSystem/` | 6 | `LyraIndicatorManagerComponent`、`IndicatorLayer`、`IndicatorDescriptor`、`IndicatorLibrary`、`SActorCanvas`、`IActorIndicatorWidget` |
| `PerformanceStats/` | 2 | `LyraPerfStatContainerBase`、`LyraPerfStatWidgetBase` |
| `Subsystem/` | 2 | `LyraUIManagerSubsystem`（UI 总管）、`LyraUIMessaging` |
| `Weapons/` | 6 | `LyraReticleWidgetBase`、`LyraWeaponUserInterface`、`HitMarkerConfirmationWidget`、`CircumferenceMarkerWidget` + 两个 Slate 版（`S` 开头） |

## 补充说明

| 点 | 说明 |
|---|---|
| Lyra 的 UI 完全建立在 **CommonUI** 插件上 | 几乎所有类都继承 `UCommonActivatableWidget` / `UCommonUserWidget` |
| 界面怎么被加进来 | 靠 `GameFeatureAction_AddWidgets`，按 GameplayTag（`UI.Layer.XXX` / 插槽 ID）定位 |
| 输入模式是界面自己声明的 | `LyraActivatableWidget::InputConfig` 决定"打开菜单时游戏还收不收按键" |
| 有 3 个文件是手写 Slate 的 | `Weapons/` 里的 `S` 开头两个 + `IndicatorSystem/SActorCanvas` —— 因为要每帧更新，UMG 太重 |

**优先级**：`LyraActivatableWidget` → `Subsystem/LyraUIManagerSubsystem` → `LyraHUDLayout` → `Common/LyraTabListWidgetBase`
