# P16 UI —— 用户界面（阶段总 md）

> **定位**：本文件是 P16（UI）的**总入口 + 全景地图**。目标：理解并一比一还原 Lyra 在 `Source/LyraGame/UI/` 底下的全部 UI 代码。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\UI\`（79 个文件，约 40 个类）
> 命名映射：`Lyra`→`Tenkaichi`，其余一字不改（铁律 24）。
> **这是全项目最大的阶段**，按子阶段拆分教学。

---

## 〇、为什么先建这个总 md（铁律 35）

P16 是 40 个类的大块内容，必须先建阶段总 md（全景 + 依赖 + 子阶段规划），**停下等确认**，再逐个类教。

---

## 一、这一节解决什么问题（先讲为什么）

P15 里 `GameFeatureAction_AddWidgets` 要往 HUD 加 UI —— 那 HUD 是什么？布局层怎么组织？控件怎么叠？这些都归 P16。

P16 覆盖 Lyra 的整个 UI 体系：
1. **HUD 主体**：`ALyraHUD`（HUD Actor）+ `ULyraHUDLayout`（布局管理器）+ `ULyraActivatableWidget`（可激活控件基类）。
2. **UI 管理器**：`ULyraUIManagerSubsystem`（CommonUI 的 UI 管理器定制）+ `ULyraUIMessaging`（UI 消息）。
3. **前端（Frontend）**：`ULyraFrontendStateComponent`（主菜单状态机）+ `ALyraLobbyBackground`。
4. **基础控件库**：`Foundation/`（按钮/确认框/断线提示/加载屏）、`Common/`（标签页/列表/控件工厂）、`Basic/`（进度条）。
5. **指示器系统**：`IndicatorSystem/`（世界空间指示器：队友标记、目标指示）。
6. **武器 UI**：`Weapons/`（准星、命中标记、圆周标记）—— 呼应 P11 武器。
7. **其他**：`LyraJoystickWidget`（虚拟摇杆）、`LyraTouchRegion`（触屏区）、`LyraTaggedWidget`（tag 控件）、`LyraSettingScreen`（设置屏）、`PerformanceStats/`（性能统计）。

> **类比**：P16 是"整个游戏的界面层"——从 HUD 骨架（HUD/Layout）、到具体零件（按钮/进度条/准星）、到特殊系统（指示器/前端菜单）。

---

## 二、全景地图（按子目录）

### 根目录（8 个类）

| 类 | 作用 |
|----|------|
| `ALyraHUD` | HUD Actor（P15.7 依赖它！） |
| `ULyraHUDLayout` | HUD 布局管理器（管布局层） |
| `ULyraActivatableWidget` | 可激活控件基类（所有 UI 控件基类） |
| `ULyraGameViewportClient` | 游戏视口客户端（控件栈管理） |
| `ULyraJoystickWidget` | 虚拟摇杆 |
| `ULyraSimulatedInputWidget` | 模拟输入控件 |
| `ULyraTaggedWidget` | 带 GameplayTag 的控件 |
| `ULyraTouchRegion` | 触屏区域 |
| `ULyraSettingScreen` | 设置屏 |

### 子目录

| 目录 | 文件数 | 类 |
|------|-------|-----|
| `Basic/` | 2 | `UMaterialProgressBar` |
| `Common/` | 10 | `ULyraBoundActionButton`/`ULyraListView`/`ULyraTabButtonBase`/`ULyraTabListWidgetBase`/`ULyraWidgetFactory`/`ULyraWidgetFactory_Class` |
| `Foundation/` | 10 | `ULyraActionWidget`/`ULyraButtonBase`/`ULyraConfirmationScreen`/`ULyraControllerDisconnectedScreen`/`ULyraLoadingScreenSubsystem` |
| `Frontend/` | 6 | `ULyraFrontendStateComponent`/`ALyraLobbyBackground`/`UApplyFrontendPerfSettingsAction` |
| `IndicatorSystem/` | 12 | `IIActorIndicatorWidget`/`UIndicatorDescriptor`/`UIndicatorLayer`/`UIndicatorLibrary`/`ULyraIndicatorManagerComponent`/`SActorCanvas` |
| `PerformanceStats/` | 4 | `ULyraPerfStatContainerBase`/`ULyraPerfStatWidgetBase` |
| `Subsystem/` | 4 | `ULyraUIManagerSubsystem`/`ULyraUIMessaging` |
| `Weapons/` | 12 | `UCircumferenceMarkerWidget`/`UHitMarkerConfirmationWidget`/`ULyraReticleWidgetBase`/`ULyraWeaponUserInterface`/`SCircumferenceMarkerWidget`/`SHitMarkerConfirmationWidget` |

---

## 三、依赖分析（铁律 30/31）

| 依赖 | 状态 |
|------|------|
| CommonUI 插件（`UCommonActivatableWidget`/`UCommonUserWidget`/`UUIExtensionSubsystem`） | ✅ S1 已装 |
| CommonGame 插件（`UCommonUIManagerSubsystem`） | ✅ S1 已装 |
| P01 Messages / P03 Input / P05 GAS / P06 Character / P11 Weapons / P12 Player / P13 Experience | ✅ 大部分已教 |
| P14 AssetManager（加载 UI 资产） | ✅ 已教 |
| P15 GameFeatureAction_AddWidget（依赖 HUD） | ✅ 已教（挂起中） |

> ✅ **P16 主要依赖已就位**（CommonUI/CommonGame 插件 + P01~P15）。这意味 P16 教完后能解开 P15.7 的 3 处挂起（P15.7-a）。

> ⚠️ 具体每个类的断点，在教到那个类时精确分析。

---

## 四、教学顺序规划（子阶段拆分）

按依赖自下而上 + 重要性排序：

| 子阶段 | 内容 | 优先级 |
|--------|------|--------|
| **P16.1** | **HUD 核心**：`LyraActivatableWidget` → `LyraHUD` → `LyraHUDLayout` → `LyraGameViewportClient` | 🔴 最高（P15.7 等它） |
| **P16.2** | **UI 管理器**：`LyraUIManagerSubsystem` / `LyraUIMessaging` | 🔴 高 |
| **P16.3** | **Foundation**（5 类）：`LyraButtonBase`/`LyraActionWidget`/`LyraConfirmationScreen`/`LyraControllerDisconnectedScreen`/`LyraLoadingScreenSubsystem` | 🟡 中 |
| **P16.4** | **Common**（6 类）：`LyraBoundActionButton`/`LyraListView`/`LyraTabButtonBase`/`LyraTabListWidgetBase`/`LyraWidgetFactory`/`LyraWidgetFactory_Class` | 🟡 中 |
| **P16.5** | **Basic**（1 类）：`MaterialProgressBar` | 🟢 低 |
| **P16.6** | **Weapons UI**（6 类）：准星 + 命中标记（呼应 P11） | 🟡 中 |
| **P16.7** | **IndicatorSystem**（6 类）：世界空间指示器 | 🟡 中 |
| **P16.8** | **Frontend**（3 类）：主菜单状态机 | 🟡 中 |
| **P16.9** | **其余控件**：`LyraJoystickWidget`/`LyraSimulatedInputWidget`/`LyraTaggedWidget`/`LyraTouchRegion`/`LyraSettingScreen` | 🟢 低 |
| **P16.10** | **PerformanceStats**（2 类） | 🟢 低 |

> 建议从 **P16.1（HUD 核心）** 开始——它最基础，且能解开 P15.7 的挂起。

---

## 五、待确认

1. P16 是 40 个类的大块，按 **10 个子阶段**拆分（先 HUD 核心）认可吗？
2. 从 **P16.1 HUD 核心**（`LyraActivatableWidget` → `LyraHUD` → `LyraHUDLayout` → `LyraGameViewportClient`）开始，OK 吗？
3. 每个类仍走「先自己的总 md → 停 → 教 h → 停 → 教 cpp」（铁律 20/29/35）吗？
4. 确认后我建 **P16.1 子阶段总 md**，然后从 `LyraActivatableWidget` 开始教。

---

## 六、源码依据

- `E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\UI\`（79 文件）
- 你的工程：`Source/TenkaichiGame/UI/`（目录待建）