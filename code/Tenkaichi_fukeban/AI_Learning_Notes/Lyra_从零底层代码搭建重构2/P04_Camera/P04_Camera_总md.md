# P04 Camera —— 总 md（为什么 + 全景 + 依赖分析）

> 依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Camera\`（12 文件）
> 你新建：`Source/TenkaichiGame/Camera/` 下对应文件
> 前缀替换：`Lyra` → `Tenkaichi`（`ULyraCameraMode`→`UTenkaichiCameraMode`、`LyraCameraComponent.h`→`TenkaichiCameraComponent.h` 等，铁律 24/41）。
> **依赖结论（铁律 30，已查 `.h`+`.cpp` 真实 include）**：P04 全部依赖**引擎模块**（`GameplayTags`/`EnhancedInput` 第 0 步已启用）+ **文件夹内部互相引用**，**不依赖任何后期内容（GAS/Character/Health 等）→ 全文件夹可独立编译，零挂起！** 这是继 P00 Physics 之后第二个「纯叶子」文件夹。

---

## 一、这个阶段解决什么问题（为什么）

**相机是玩家看游戏的"眼睛"**。Lyra 的相机系统不是 UE 默认的单相机，而是一套**可配置、可叠加、防穿墙**的第三人称相机系统：

1. **`CameraAssistInterface`**（P04.1，已教）：相机穿墙策略接口——谁实现它，谁定制"允许穿透列表/禁止穿透目标/穿透回调"。
2. **`CameraMode`**（P04.2，本阶段）：相机模式基类——定义相机"放在哪、看哪、FOV 多少、怎么过渡"。是所有相机行为的地基。
3. **`CameraModeStack`**（P04.2，本阶段）：相机模式栈——同时叠加多个相机模式（如"正常 + 瞄准"），按权重混合它们。
4. **`CameraMode_ThirdPerson`**（P04.2，本阶段）：第三人称相机模式——跟随目标、按视角偏移、防穿墙。
5. **`CameraComponent`**（P04.2，本阶段）：相机组件——挂在角色上的入口，驱动 CameraModeStack 计算每帧视图。
6. **`PenetrationAvoidanceFeeler`**（P04.3，本阶段）：防穿透"探针"结构体——定义多条射线探测墙壁，避免相机穿模。
7. **`PlayerCameraManager`**（P04.3，本阶段）：玩家相机管理器——替换 UE 默认的，接入相机组件 + UI 相机。
8. **`UICameraManagerComponent`**（P04.3，本阶段）：UI 相机组件——当 UI 需要抢相机控制权（过场/菜单）时接管视图。

> 🔑 **一句话**：Lyra 用「相机模式栈 + 第三人称模式 + 防穿墙探针」搭了一套可配置、可混合、可防穿模的第三人称相机系统，替换 UE 默认相机。

---

## 二、依赖全景（铁律 30 查证，按被依赖在下）

```
P04 Camera
├─ P04.1 CameraAssistInterface（✅ 已教，重构版1 4.8.0）
├─ P04.2 CameraMode / CameraModeStack / CameraMode_ThirdPerson / CameraComponent
│    ├─ 引擎：Camera/CameraComponent.h、Engine/World.h、GameplayTagContainer.h（引擎 GameplayTags ✅）
│    ├─ 引擎：Components/CapsuleComponent.h、GameFramework/Character.h、GameFramework/Pawn.h
│    ├─ 引擎：Curves/CurveFloat.h、Curves/CurveVector.h、DrawDebugHelpers.h、GameFramework/CameraBlockingVolume.h
│    ├─ 引擎：Math/RotationMatrix.h、Components/PrimitiveComponent.h
│    └─ 内部：PenetrationAvoidanceFeeler（P04.3）+ CameraAssistInterface（P04.1）✅
└─ P04.3 PlayerCameraManager / UICameraManagerComponent / PenetrationAvoidanceFeeler
     ├─ 引擎：Camera/PlayerCameraManager.h、GameFramework/HUD.h、GameFramework/PlayerController.h、Engine/Canvas.h
     └─ 内部：CameraComponent（P04.2）+ PlayerCameraManager（P04.3）✅
```

### 具体每个文件的真实依赖（`.cpp` include 为准）

| 文件 | `.h` 依赖 | `.cpp` 依赖 | 状态 |
|------|-----------|------------|------|
| `LyraCameraAssistInterface` | 引擎（CoreMinimal/Interface） | 无 `.cpp`（纯接口） | ✅ P04.1 已教（重构版1 4.8.0） |
| `LyraPenetrationAvoidanceFeeler` | 引擎（CoreMinimal） | 无 `.cpp`（纯结构体） | ✅ 纯叶子，可独立编译 |
| `LyraCameraMode`（含 CameraModeStack） | 引擎（Engine/World、GameplayTagContainer） | 引擎（Character/CapsuleComponent/Canvas）+ 内部 CameraComponent/PlayerCameraManager | ✅ 可独立编译 |
| `LyraCameraComponent` | 引擎（CameraComponent/Actor）+ 前置声明 CameraMode | 引擎（Pawn/PlayerController/Canvas）+ 内部 CameraMode | ✅ 可独立编译 |
| `LyraCameraMode_ThirdPerson` | 引擎（CurveFloat/DrawDebugHelpers）+ 内部 CameraMode/PenetrationAvoidanceFeeler | 引擎（Character/CameraBlockingVolume/CurveVector/RotationMatrix）+ 内部 CameraAssistInterface | ✅ 可独立编译 |
| `LyraPlayerCameraManager` | 引擎（PlayerCameraManager）+ 前置声明 UICameraManagerComponent | 引擎（Pawn/PlayerController/Canvas）+ 内部 CameraComponent/UICameraManagerComponent | ✅ 可独立编译 |
| `LyraUICameraManagerComponent` | 引擎（PlayerCameraManager）+ 前置声明 PlayerCameraManager | 引擎（HUD/PlayerController）+ 内部 PlayerCameraManager | ✅ 可独立编译 |

### 挂起总结

**P04 Camera 无任何挂起。** 全部 12 个文件依赖引擎 + 文件夹内部互相引用，不依赖 GAS/Character/Health/Experience 等后期内容。`GameplayTags`/`EnhancedInput` 依赖第 0 步已启用，Build.cs 已就位。

> **与 P03 Input 对比**：P03 的 `.cpp` 多处依赖 P18 Settings / P12 Player 挂起；P04 是"纯叶子"，所有 `.h`/`.cpp` 当场一比一完整教、当场落盘，**零挂起**。

---

## 三、全景：本阶段 8 个类/结构的职责

| 类/结构 | 属于 | 一句话职责 |
|--------|------|-----------|
| `ILyraCameraAssistInterface` | P04.1 | 相机穿墙策略接口（允许穿透/禁止穿透/穿透回调） |
| `FLyraPenetrationAvoidanceFeeler` | P04.3 | 防穿透"探针"结构体（一条探测射线：角度/权重/半径/间隔） |
| `ULyraCameraMode` | P04.2 | 相机模式基类（决定相机位置/旋转/FOV/过渡） |
| `ULyraCameraModeStack` | P04.2 | 相机模式栈（叠加多个模式，按权重混合） |
| `ULyraCameraMode_ThirdPerson` | P04.2 | 第三人称相机模式（跟随目标 + 视角偏移 + 防穿墙） |
| `ULyraCameraComponent` | P04.2 | 相机组件（角色入口，驱动模式栈算每帧视图） |
| `ALyraPlayerCameraManager` | P04.3 | 玩家相机管理器（替换默认，接入相机组件 + UI 相机） |
| `ULyraUICameraManagerComponent` | P04.3 | UI 相机组件（UI 需要时接管相机视图） |

> 注：`LyraCameraMode.h` 里定义了 **2 个类**（`ULyraCameraMode` 基类 + `ULyraCameraModeStack` 栈），都在 P04.2 教。

---

## 四、教学节奏（铁律 29/35）

- ✅ **本总 md**（P04.0）：概念 + 全景 + 依赖（零挂起）。
- ✅ **P04.1**：`LyraCameraAssistInterface` 已教完（重构版1 4.8.0，工程 `TenkaichiCameraAssistInterface.h` 已落盘，被 PlayerController 引用）——**直接沿用，不重教**。
- ✅ **P04.2**：`LyraCameraMode`（含 CameraModeStack）+ `LyraCameraMode_ThirdPerson` + `LyraCameraComponent` 三个类全部教完（各建总md→h→cpp，共 9 个 md），工程 `TenkaichiCameraMode.h/.cpp`、`TenkaichiCameraMode_ThirdPerson.h/.cpp`、`TenkaichiCameraComponent.h/.cpp` 已全部落盘。**P04.2 全清**。
- ✅ **P04.3**：`LyraPenetrationAvoidanceFeeler`（纯结构体，只有 `.h`，P04.3-1）、`LyraPlayerCameraManager`（P04.3-2）、`LyraUICameraManagerComponent`（P04.3-3）三个类全部教完（各建总md→h→cpp，共 9 个 md），工程 `TenkaichiPenetrationAvoidanceFeeler.h`、`TenkaichiPlayerCameraManager.h/.cpp`、`TenkaichiUICameraManagerComponent.h/.cpp` 已全部落盘。**P04.3 全清 → P04 Camera 文件夹 12 文件全部教完，整体编译闭环**。

> 每个独立类都按铁律 35 先建它自己的总 md → 停 → 教 `.h` → 停 → 教 `.cpp`。P04 无挂起，所有 `.h`/`.cpp` 当场完整落盘。
> **P04.3 死结**（PlayerCameraManager ↔ UICameraManagerComponent 互相 include）：两个 `.cpp` 都落盘后死结闭环，P04 相机文件夹整体编译通过（铁律 31 + 死结处理铁律）。

---

## 五、衔接说明

- P04 依赖 **引擎模块**（`GameplayTags`/`EnhancedInput` 第 0 步已启用，Build.cs 已有 `GameplayTags`/`EnhancedInput`）——**无新增插件**。
- P04.2 的 `CameraMode_ThirdPerson` 依赖 `CameraAssistInterface`（P04.1，已教）+ `PenetrationAvoidanceFeeler`（P04.3）——因都在本文件夹，先教叶子（Feeler），再教依赖它的类。
- P04 的 `CameraComponent` 被 **P06 Character / P12 Player** 使用（`PlayerController` 已引用 `CameraAssistInterface`）；P04 教完后，P06/P12 的相机相关引用即可编译。
- P04 完成后，进入 **P05 AbilitySystem（GAS 核心）**——那是最大的断点类集中地，也是解开阶段四 13 项挂起的时机。

---

## 待确认

1. P04 Camera 总 md 看懂了：Lyra 用「相机模式栈 + 第三人称模式 + 防穿墙探针」搭了一套可配置/可混合/可防穿模的第三人称相机系统；**全文件夹可独立编译、零挂起**（不依赖 GAS 等后期内容）。
2. **P04.1 `CameraAssistInterface` 已教完**（重构版1 4.8.0，工程已落盘）——直接沿用，不重教，OK 吗？
3. **P04 已全部教完**（P04.1~P04.3 共 8 个类/结构，12 个文件全落盘，整体编译闭环）。下一步进入 **P05 AbilitySystem（GAS 核心）**。

---

## 源码依据

- Lyra 文件夹：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Camera\`（12 文件）
- 依赖（`.cpp` include 为准）：全部引擎模块 + 文件夹内部互相引用（`CameraMode.cpp`→CameraComponent/PlayerCameraManager；`CameraMode_ThirdPerson.cpp`→CameraAssistInterface；`PlayerCameraManager.cpp`→CameraComponent/UICameraManagerComponent；`UICameraManagerComponent.cpp`→PlayerCameraManager）
- 引擎模块：`GameplayTags`、`EnhancedInput`（Build.cs 已加，见 `TenkaichiGame.Build.cs` 第 30/56 行）