# P02 System 叶子（独立工具类）—— 总 md

> 依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\`（26 个文件，**本阶段只教 4 个叶子工具类**；其余 AssetManager/GameInstance/ReplicationGraph 等是收口类，归 P14 教）
> 对应 Lyra 文件夹：`System/`

---

## 一、这一章解决什么问题（为什么）

P01 Messages 已经教会了"怎么定义、复制、监听消息"。但很多玩法事件在触发前，经常要先问一些"**系统级问题**"：

1. **我现在是服务器还是客户端？** → 决定"这个事件该不该在这端处理"
2. **这个软引用 AssetId 对应的资产是什么？** → 按 ID 找资产
3. **这个 Actor 上所有 Mesh 组件的某个材质参数改一下** → 批量改材质
4. **全局游戏数据（伤害/治疗效果）在哪存？** → 一个全局 DataAsset

这些"**跟具体玩法无关、纯工具性质**"的函数，Lyra 都放在 `System/` 文件夹，做成**静态工具函数库（`UBlueprintFunctionLibrary`）**或**全局数据资产**。本阶段教其中 **4 个叶子**：

| 文件 | 干嘛 | 依赖 |
|------|------|------|
| `LyraActorUtilities` | 查询当前是哪种网络模式（服务器/客户端/单机） | 纯引擎 ✅ |
| `LyraSystemStatics` | 资产查询 + 批量改材质 + 切下一局 | 大部分引擎；**1 个函数依赖 P13 Experience** ⚠️ |
| `LyraGameData` | 全局游戏数据资产（伤害/治疗效果引用） | `.h` 可编译；**`.cpp` 依赖 P14 AssetManager** ⚠️ |
| `LyraDevelopmentStatics` | 编辑器/开发调试相关查询 | 大部分引擎；**3 个函数依赖 P18 Development** ⚠️ |

> **P02.1 `GameplayTagStack`**（带计数的标签栈）已在**重构版1 教过并落盘**（`P02.1 已完成`），本阶段**不重教**，直接从 P02.2 开始。

---

## 二、内部结构全景（4 个类）

```
System/ 文件夹（本阶段只教 4 个叶子）
├─ ULyraActorUtilities : UBlueprintFunctionLibrary      ← 网络模式查询
│   └─ SwitchOnNetMode(WorldContext) → EBlueprintExposedNetMode
├─ ULyraSystemStatics : UBlueprintFunctionLibrary        ← 资产/材质/切局工具
│   ├─ GetTypedSoftObjectReferenceFromPrimaryAssetId()
│   ├─ GetPrimaryAssetIdFromUserFacingExperienceName()   ⚠️ P13
│   ├─ PlayNextGame()
│   ├─ SetScalar/Vector/ColorParameterValueOnAllMeshComponents()
│   └─ FindComponentsByClass()
├─ ULyraGameData : UPrimaryDataAsset                     ← 全局游戏数据（.cpp ⚠️ P14）
│   ├─ DamageGameplayEffect_SetByCaller
│   ├─ HealGameplayEffect_SetByCaller
│   └─ DynamicTagGameplayEffect
└─ ULyraDevelopmentStatics : UBlueprintFunctionLibrary    ← 开发调试工具（3 函数 ⚠️ P18）
    ├─ ShouldSkipDirectlyToGameplay()
    ├─ ShouldLoadCosmeticBackgrounds()
    ├─ CanPlayerBotsAttack()
    ├─ FindPlayInEditorAuthorityWorld()
    ├─ FindClassByShortName()（含模板重载）
    └─ FindBlueprintClass() / GetAllBlueprints()（私有）
```

---

## 三、依赖分析（铁律 30/31 核查，追到底）

| 文件 | `.h` | `.cpp` 可编译部分 | `.cpp` 挂起部分 | 挂起依赖 |
|------|------|------------------|----------------|---------|
| `LyraActorUtilities` | 引擎 `UBlueprintFunctionLibrary` + 枚举 | 全部（引擎 `Actor::GetNetMode`） | **无** | — ✅ |
| `LyraSystemStatics` | 引擎 + `FPrimaryAssetId`/`SoftObjectPtr` | 资产查询/改材质/FindComponents | `GetPrimaryAssetIdFromUserFacingExperienceName` + include | **P13 Experience**（`ULyraUserFacingExperienceDefinition`） |
| `LyraGameData` | 引擎 `UPrimaryDataAsset` + `UGameplayEffect`**前置声明** | `.h` 全可编译 | `.cpp` 的 `Get()` + include | **P14 AssetManager**（`ULyraAssetManager::Get().GetGameData()`） |
| `LyraDevelopmentStatics` | 引擎 + `Templates/SubclassOf.h` | 资产搜索/类查找 | 三个 Settings 函数 + include | **P18 Development**（`ULyraDeveloperSettings`） |

**关键点（铁律 30）**：
- `LyraGameData.h` 里 `UGameplayEffect` 只是**前置声明**（`TSoftClassPtr<UGameplayEffect>` 是软引用成员），**不阻塞编译**——所以 `.h` 可独立编译；只有 `.cpp` 的 `Get()` 硬调用 `ULyraAssetManager` 才需挂起。
- `LyraActorUtilities` 完全独立，**无任何挂起**，可先教。

**依赖链追到底**：
- `LyraActorUtilities` → 引擎 `Actor::GetNetMode`（纯引擎）
- `LyraSystemStatics` → 引擎 AssetManager/World/MeshComponent + `LyraLogChannels`（第 0 步已教）+ `ULyraUserFacingExperienceDefinition`（**P13**）
- `LyraGameData` → `UPrimaryDataAsset`（引擎）+ `ULyraAssetManager`（**P14**）
- `LyraDevelopmentStatics` → 引擎 AssetRegistry/Blueprint + `ULyraDeveloperSettings`（**P18**）

**结论**：按依赖从简到繁教：**`LyraActorUtilities`（全可编译）→ `LyraSystemStatics`（P13 段挂起）→ `LyraGameData`（`.cpp` 挂起）→ `LyraDevelopmentStatics`（P18 段挂起）**。挂起内容登记注释备忘录（铁律 34）。

---

## 四、新知识点预告（教到时会拆开讲）

1. **`UBlueprintFunctionLibrary`**（复用 P01.5）：静态函数暴露给蓝图。
2. **`ENetMode` / `GetNetMode()`**：引擎网络模式枚举——服务器/客户端/单机怎么判断。
3. **`FPrimaryAssetId` / `TSoftObjectPtr` / `UAssetManager`**：资产系统——按 ID 找资产、软引用。
4. **`UPrimaryDataAsset`**：全局数据资产基类——放不常变的全局配置。
5. **`TSoftClassPtr<UGameplayEffect>`**：GAS 效果的软类引用（GAS 类型，先认识，P05 深入）。
6. **`WITH_EDITOR` / `GetDefault<>()`**：编辑器专用代码 + 读默认配置对象。

---

## 五、怎么教（推进节奏）

按铁律 35/20/29：**先建本总 md（已建）→ 停，等确认 → 逐个文件教 `.h` → 教 `.cpp`**。每个文件按铁律 35 再建**它自己的总 md**（先总md → 停 → `.h` → 停 → `.cpp`）。

教学顺序（依赖从简到繁）：
1. `LyraActorUtilities`（**无挂起**，纯引擎）✅ 已完成（总md→h→cpp 三步全部教完；h 已落盘，cpp 待用户落盘）
2. `LyraSystemStatics`（**1 个函数挂起 P13**）✅ 已完成（总md→h→cpp 三步教完；h 已落盘，cpp 待用户落盘，`GetPrimaryAssetIdFromUserFacingExperienceName` + include 挂起）
3. `LyraGameData`（**`Get()` 挂起 P14**）✅ 已完成（总md→h→cpp 三步教完；h 已落盘，cpp 待用户落盘，`Get()` + include 挂起）
4. `LyraDevelopmentStatics`（**组 A 3 函数挂起 P18**）✅ 已完成（总md→h→cpp 三步教完；h 已落盘，cpp 已落盘 224 行，组 A 3 函数 + include 挂起）→ **P02 System 叶子全清**

> ⚠️ **挂起说明（铁律 33/34）**：三个文件的 `.cpp` 里硬依赖未就位阶段的代码（P13 Experience / P14 AssetManager / P18 Development），**代码一比一完整写上、用 `#if 0`/`//` 注释挂起**，登记注释备忘录，到对应阶段就位后解开。

---

**下一步**：**P02 System 叶子全部完成**（GameplayTagStack / ActorUtilities / SystemStatics / GameData / DevelopmentStatics）。下一阶段进入 **P03 Input**（总 md → 按子项教学），见总纲 P03 章节。