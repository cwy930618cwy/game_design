# 04 — `LyraGame.Build.cs`（S00 精简版）复制文件

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs`（**91 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGame.Build.cs`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。
> ⚠️ **这是 S00 精简版**（6 公共 + 3 私有），S06 会整份替换成 Lyra 完整版（22 公共 + 25 私有）。

---

```csharp
// 版权声明：Epic Games 保留所有权利
// Copyright Epic Games, Inc. All Rights Reserved.

// 引入 UnrealBuildTool 命名空间；ModuleRules / PCHUsageMode 都在这里
using UnrealBuildTool;

// 定义 LyraGame 模块规则类；类名必须与模块名一致（.uproject 里写的是 "LyraGame"）
public class LyraGame : ModuleRules
{
	// 构造函数；UBT 编译本模块时调用，ReadOnlyTargetRules 携带目标配置（平台/配置/是否编辑器）
	public LyraGame(ReadOnlyTargetRules Target) : base(Target)
	{
		// PCH 使用模式：显式或共享 PCH
		// UseExplicitOrSharedPCHs = 优先用共享 PCH，没有共享 PCH 时用显式 PCH
		// 这是 UE5 推荐的默认值，能显著加快编译
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共头文件搜索路径：把模块根目录 "LyraGame" 加进去
		// 这样其他模块可以写 #include "Character/LyraCharacter.h"（省略 Source\LyraGame\ 前缀）
		PublicIncludePaths.AddRange(
			new string[] {
				"LyraGame"
			}
		);

		// 私有头文件搜索路径：本模块内部专用，不对外暴露
		// S00 阶段为空，与 Lyra 原版一致
		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		// 公共依赖模块：本模块的公共头文件里 include 了这些模块的头文件
		// 依赖会「传染」—— 依赖 LyraGame 的模块也会自动依赖这些
		PublicDependencyModuleNames.AddRange(
			new string[] {
				// 引擎最底层：FString / TArray / 容器 / 数学库
				"Core",
				// UObject 系统：UCLASS / UPROPERTY / 反射 / GC
				"CoreUObject",
				// 引擎主体：AActor / UActorComponent / UWorld / 渲染 / 物理
				"Engine",
				// GameplayTag 系统：FGameplayTag / FGameplayTagContainer
				"GameplayTags",
				// GameplayTask 系统：UAbilityTask 的基类
				"GameplayTasks",
				// GAS 主体：UAbilitySystemComponent / UGameplayAbility / UGameplayEffect
				"GameplayAbilities"
			}
		);

		// 私有依赖模块：只在本模块的 .cpp 里用到，公共头文件不暴露
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				// 输入核心：FKey / EKeys
				"InputCore",
				// 开发者设置：ULyraDeveloperSettings 的基类 UDeveloperSettings
				"DeveloperSettings",
				// 增强输入系统：UInputAction / UInputMappingContext
				"EnhancedInput"
			}
		);

		// 动态加载模块：运行时用 FModuleManager::LoadModulePtr 加载的模块
		// S00 阶段为空，与 Lyra 原版一致
		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);

		// 全局宏定义：Shipping 下把「绘制调试错误」变成编译期错误
		// 防止调试绘制代码漏进发行包
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		// 启用 GameplayDebugger 支持；UBT 会自动加 GameplayDebugger 依赖
		// 编辑器里按 ' 键可打开 GAS 调试面板
		SetupGameplayDebuggerSupport(Target);
		// 启用 Iris 网络复制支持；UBT 会自动加 Iris 相关依赖
		// Iris 是 UE5 的新一代网络复制系统
		SetupIrisSupport(Target);
	}
}
```

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraGame\LyraGame.Build.cs` |
| 类名 | `LyraGame`，与模块名一致 |
| PCH | `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;` |
| 公共头文件路径 | `PublicIncludePaths` 里有 `"LyraGame"` |
| 公共依赖 | **只有 6 个**：`Core` / `CoreUObject` / `Engine` / `GameplayTags` / `GameplayTasks` / `GameplayAbilities` |
| 私有依赖 | **只有 3 个**：`InputCore` / `DeveloperSettings` / `EnhancedInput` |
| 禁止出现 | `CommonGame` / `CommonUser` / `GameSettings` / `ModularGameplayActors` / `AsyncMixin` / `UIExtension` / `GameplayMessageRuntime` / `CommonLoadingScreen` / `GameSubtitles` |
| 宏定义 | `PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");` |
| 两个 Setup | `SetupGameplayDebuggerSupport(Target);` + `SetupIrisSupport(Target);` |
| 注释 | 所有 `//` 开头的行**全部删掉**，一行都不留 |
| 缩进 | Tab（不是空格） |

---

## 敲完还要做一件事

删掉旧的 `Source\Tenkaichi\` 整个目录（`Tenkaichi.Build.cs` / `Tenkaichi.cpp` / `Tenkaichi.h`）。

**原因**：`.uproject` 里 `Modules` 只有 `LyraGame`，没有 `Tenkaichi`。留着旧模块目录，UBT 会报「模块未在 .uproject 中声明」。

---

## 源码依据

- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:1-91`（全文）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:9`（`PCHUsage`）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:11-15`（`PublicIncludePaths`）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:22-47`（22 个公共依赖）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:49-77`（25 个私有依赖）
- `E:\ue5\LyraStarterGame\Source\LyraGame\LyraGame.Build.cs:84-89`（`PublicDefinitions` + 两个 Setup 调用）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:36,72`（`GameplayAbilities` / `EnhancedInput` 已启用，故 S00 可写）