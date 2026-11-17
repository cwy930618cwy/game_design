# 01 — `Core.Build.cs` —— 完整代码

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs`
> **✅ 本节可独立编译**——C# 构建脚本，由 UBT 自己编译，不参与 C++ 编译。
> **教法**：每段代码下面先列「**本段新元素**」（第一次出现的，一行一个讲清），再讲「这段在干什么」。

---

## 一、这个文件解决什么问题（为什么）

上一课两张 `Target.cs` 回答「**这次编什么**」。这一份回答「**Core 这个模块怎么编**」——同一个 UBT 体系，基类从 `TargetRules` 换成 `ModuleRules`。

**说个场景**：`Target.cs` 是总包工头说「今天盖楼还是盖厂房」；`Core.Build.cs` 是 Core 这间机房的施工细则——用什么预编译头、接哪几路模块、每个平台额外挂什么库和宏。

> 🔑 **一句话**：330 行全是往 UBT 的清单里**加名字**（模块名 / 库名 / 宏名 / 路径），一行 C++ 逻辑都没有。

---

## 二、完整代码（注释已中文化，你照这个抄）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。

using UnrealBuildTool;
using System;
using System.IO;

// Core 模块声明：告诉 UBT 这个模块怎么编（依赖哪些模块、定义哪些宏、各平台额外挂什么）
public class Core : ModuleRules
{
	// 构造函数：把当前编译目标（平台 / 配置 / 目标类型）交给基类 ModuleRules
	public Core(ReadOnlyTargetRules Target) : base(Target)
	{
		NumIncludedBytesPerUnityCPPOverride = 491520; // 用 UBT 的 ProfileUnitySizes 模式测出来的最佳 Unity 合批大小

		PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";

		SharedPCHHeaderFile = "Public/CoreSharedPCH.h";

		PrivateDependencyModuleNames.Add("BuildSettings");
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("GoogleGameSDK");
			
			PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 需要

			if (Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 与 ARM Mali GPU 的性能计数器
				PrivateDependencyModuleNames.Add("heapprofd");      // 把自定义分配器暴露给 Google 的内存分析器
			}
			string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
		}

		PrivateDependencyModuleNames.Add("AtomicQueue");
		PrivateDependencyModuleNames.Add("AutoRTFM");
		PrivateDependencyModuleNames.Add("BLAKE3");
		PrivateDependencyModuleNames.Add("OodleDataCompression");
		PrivateDependencyModuleNames.Add("xxhash");

		PublicDependencyModuleNames.Add("GuidelinesSupportLibrary");
		PublicDependencyModuleNames.Add("TraceLog");
		PublicDependencyModuleNames.Add("AtomicQueue");

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"DerivedDataCache",
				"TargetPlatform",

				"Json",
				"RSA"

			}
			);

		PublicIncludePathModuleNames.AddRange(
			new string[] {
				"AutoRTFM",
				"ImageCore"
			}
			);

		if (Target.bBuildEditor == true)
		{
			PrivateIncludePathModuleNames.Add("DirectoryWatcher");
			DynamicallyLoadedModuleNames.Add("DirectoryWatcher");
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDefinitions.Add("PLATFORM_BUILDS_LIBPAS=1");
			PrivateDependencyModuleNames.Add("libpas");
		}
			
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);

			if (Target.WindowsPlatform.Architecture != UnrealArch.Arm64)
			{
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"IntelTBB",
					"IntelVTune"
					);
			}

			// 不希望静态分析器去跑第三方代码
			if (Target.StaticAnalyzer == StaticAnalyzer.None) 
			{
				PrivateDependencyModuleNames.Add("mimalloc");
				PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");
			}
			
			if (Target.WindowsPlatform.bUseBundledDbgHelp)
			{
				PublicDelayLoadDLLs.Add("DBGHELP.DLL");
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=1");
				RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/DbgHelp/dbghelp.dll");
			}
			else
			{
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=0");
			}
			PrivateDefinitions.Add("YIELD_BETWEEN_TASKS=1");

			if (Target.WindowsPlatform.bPixProfilingEnabled && Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.Add("WinPixEventRuntime");
			}

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"IntelTBB",
				"zlib",
				"PLCrashReporter"
				);
			PublicFrameworks.AddRange(new string[] { "Cocoa", "Carbon", "IOKit", "Security", "UniformTypeIdentifiers", "Network", "ScreenCaptureKit", "SystemConfiguration" });

			PrivateDependencyModuleNames.Add("mimalloc");
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			if (Target.bBuildEditor == true)
			{
				string XcodeRoot = Utils.RunLocalProcessAndReturnStdOut("/usr/bin/xcode-select", "--print-path");
				PublicAdditionalLibraries.Add(XcodeRoot + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/PrivateFrameworks/MultitouchSupport.framework/Versions/Current/MultitouchSupport.tbd");
			}
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.IOS))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);
			PublicFrameworks.AddRange(new string[] { "UIKit", "Foundation", "AudioToolbox", "AVFoundation", "GameKit", "CoreVideo", "CoreMedia", "CoreGraphics", "GameController", "SystemConfiguration", "DeviceCheck", "UserNotifications", "Network" });

			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PublicFrameworks.AddRange(new string[] { "CoreMotion", "AdSupport", "WebKit" });
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"PLCrashReporter"
					);

				PrivateIncludePathModuleNames.Add("MarketplaceKit");
			}
			if (Target.Platform == UnrealTargetPlatform.VisionOS)
			{
				PublicFrameworks.Add("CoreMotion");
			}

			if (Target.bCompileAgainstApplicationCore)
			{
				PrivateIncludePathModuleNames.Add("ApplicationCore");
			}

			bool bSupportAdvertising = Target.Platform == UnrealTargetPlatform.IOS;
			if (bSupportAdvertising)
			{
				PublicFrameworks.AddRange(new string[] { "iAD" });
			}

			// 为内嵌 Dll 导出 Core 的符号
			ModuleSymbolVisibility = ModuleRules.SymbolVisibility.VisibileForDll;
			
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
				{
					PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
					PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				}
			}
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"cxademangle",
				"zlib",
				"libunwind"
				);

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				PrivateDefinitions.Add("UE_CALLSTACK_TRACE_ANDROID_USE_STACK_FRAMES_WALKING=1");

				// 支持对 libc.so 的 malloc 做内存追踪
				PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Build", "Android", "Prebuilt", "ScudoMemoryTrace"));
				PrivateDefinitions.Add("UE_MEMORY_TRACE_ANDROID_ENABLE_SCUDO_TRACING_SUPPORT=1");
			}
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib",
				"jemalloc"
				);

			// Core 用到 dlopen()
			PublicSystemLibraries.Add("dl");

			PrivateDependencyModuleNames.Add("mimalloc");
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
			}
		}

		if (Target.bCompileICU == true)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "ICU");
		}
		PublicDefinitions.Add("UE_ENABLE_ICU=" + (Target.bCompileICU ? "1" : "0")); // 在代码库里启用/禁用（=1/=0）ICU。注意：这个开关是集成 ICU 期间用的，之后会移除。

		// 如果正在和引擎一起编译，就加上 Core 对引擎的依赖
		if (Target.bCompileAgainstEngine && !Target.bBuildRequiresCookedData)
		{
			DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
			DynamicallyLoadedModuleNames.AddRange(new string[] { "Virtualization" });
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop))
        {
			// 在桌面平台上启用更多崩溃信息。
			PublicDefinitions.Add("WITH_ADDITIONAL_CRASH_CONTEXTS=1");
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			// Visual Studio 的并发可视化扩展
			PrivateDependencyModuleNames.Add("ConcurrencyVisualizer");
			
			// Super Luminal。
			PrivateDependencyModuleNames.Add("SuperLuminal");
			
			// Visual Studio 性能分析器
			PrivateDependencyModuleNames.Add("VSPerfExternalProfiler");
		}

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Apple) ||
			Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("IS_RUNNING_GAMETHREAD_ON_EXTERNAL_THREAD=1");
		}

		// 设置一个宏，让 FApp::GetBuildTargetType() 能识别客户端目标
		if (Target.Type == TargetRules.TargetType.Client)
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=1");
		}
		else
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=0");
		}

		// 设置用于包含 / 排除 Iris 对 UObject 改动的定义。注意：只需要这个定义，因为我们完全不依赖 Iris。
		if (Target.bUseIris == true)
		{
			PublicDefinitions.Add("UE_WITH_IRIS=1");
		}
		else
		{
			PublicDefinitions.Add("UE_WITH_IRIS=0");
		}
		
		if (Target.Platform == UnrealTargetPlatform.Win64
			&& Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("PLATFORM_SUPPORTS_PLATFORM_EVENTS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_VIRTUAL_MEMORY_HOOKS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_MODULE_DIAGNOSTICS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_CALLSTACK=1");
			PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
		}

		// 临时性的东西。
		PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));

		PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));

		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_LTCG=" + (Target.bAllowLTCG ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG=" + (Target.bPGOOptimize ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG_PROFILING=" + (Target.bPGOProfile ? "1" : "0"));

		PrivateDefinitions.Add("UE_DEFINE_LEGACY_MATH_CONSTANT_MACRO_NAMES=0");

		PrivateDependencyModuleNames.Add("AutoRTFM");

		CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Error;

		IWYUSupport = IWYUSupport.KeepAsIs;

		if (Target.WindowsPlatform.bEnableInstrumentation)
		{
			PrivateDependencyModuleNames.Add("Detours");
		}

		PublicDefinitions.Add("UE_MERGED_MODULES=" + (Target.bMergeModules ? "1" : "0"));
	}

	// 决定当前平台能否使用二进制配置
	protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
	{
		return true;// Target.Platform != UnrealTargetPlatform.Android;
	}

	// 决定当前平台能否使用内存分配器校验（也就是 MallocStomp）。
	// 运行时校验必须通过 '-stompmalloc' 命令行参数开启。
	protected virtual bool bWithMallocStomp
	{
		get => 
			Target.Configuration != UnrealTargetConfiguration.Shipping &&
			(Target.Platform == UnrealTargetPlatform.Mac ||
			 Target.Platform == UnrealTargetPlatform.Linux ||
			 Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
			 Target.Platform == UnrealTargetPlatform.Win64);
	}
}
```

> ⚠️ **三处必须原样照抄的引擎原文细节**：
> 1. 第 316 行 `return true;// Target.Platform != UnrealTargetPlatform.Android;` —— `//` 前**没有空格**，后面是被注释掉的旧条件（属于代码，不翻译）；
> 2. 第 319 行注释里的 `plVatform` 是引擎原文错字（不是 `platform`），照抄；
> 3. 第 72 / 93 / 245 / 248 行是**带空白的空行**（3 个 Tab），第 278 行是 2 个 Tab 的空行，都照抄。

---

## 三、逐段拆解（每段先列新元素，再讲这段在干什么）

### 3.1 骨架：using + 类声明 + 构造函数

```csharp
using UnrealBuildTool;
using System;
using System.IO;

// Core 模块声明：告诉 UBT 这个模块怎么编（依赖哪些模块、定义哪些宏、各平台额外挂什么）
public class Core : ModuleRules
{
	// 构造函数：把当前编译目标（平台 / 配置 / 目标类型）交给基类 ModuleRules
	public Core(ReadOnlyTargetRules Target) : base(Target)
	{
```

**本段新元素**

- `using UnrealBuildTool` —— 引入 UBT 程序集，`ModuleRules` / `ReadOnlyTargetRules` / `UnrealTargetPlatform` 都在里面
- `using System` —— 引入 .NET 基础类型（本文件没直接用，但引擎原样保留，不许删）
- `using System.IO` —— 引入 `Path` 类（第 30 行 `Path.Combine` 要用）
- `ModuleRules` —— **模块规则基类**，所有 `*.Build.cs` 都继承它（`ModuleRules.cs:103`）
- `ReadOnlyTargetRules` —— **只读目标规则**，包着可写的 `TargetRules`，模块只能读不能改（`ReadOnlyTargetRules.cs:16`）
- `base(Target)` —— 调基类构造函数，把 `Target` 存进 `ModuleRules.Target`（`ModuleRules.cs:1584`）

**这段在干什么**：声明 `Core` 类继承 `ModuleRules`，构造函数接收 `Target` 并交给基类。**不写 `: base(Target)` 的话，`Target` 和 `CppCompileWarningSettings` 都是 null，后面全崩。**

> 类比：`Target.cs` 是「今天盖什么」，`Build.cs` 是「这间房怎么盖」——同一个包工头（UBT），两张不同的单子。

### 3.2 PCH 与 Unity 合批

```csharp
		NumIncludedBytesPerUnityCPPOverride = 491520; // 用 UBT 的 ProfileUnitySizes 模式测出来的最佳 Unity 合批大小

		PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";

		SharedPCHHeaderFile = "Public/CoreSharedPCH.h";
```

**本段新元素**

- `NumIncludedBytesPerUnityCPPOverride` —— Unity 合批时每个 unity cpp 的字节上限覆盖，0=不覆盖（`ModuleRules.cs:1162`）
- `PrivatePCHHeaderFile` —— 私有预编译头相对路径，设了就不再用共享 PCH（`ModuleRules.cs:793`）
- `SharedPCHHeaderFile` —— 共享预编译头相对路径，本模块对外提供（`ModuleRules.cs:799`）

**这段在干什么**：491520 = 480 KB，是引擎用 `ProfileUnitySizes` 模式实测的最佳合批大小；两个 PCH 分别指向 `Private/CorePrivatePCH.h`（11 行）和 `Public/CoreSharedPCH.h`（263 行）。

> 类比：PCH 是「提前和好的面团」，Unity 合批是「一次烤多少张饼」——两个都调好了，编译才快。

### 3.3 Android 块（第 18~31 行）

```csharp
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("GoogleGameSDK");
			
			PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 需要

			if (Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 与 ARM Mali GPU 的性能计数器
				PrivateDependencyModuleNames.Add("heapprofd");      // 把自定义分配器暴露给 Google 的内存分析器
			}
			string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
		}
```

**本段新元素**

- `Target.Platform` —— 当前编译平台，类型 `UnrealTargetPlatform`（`ReadOnlyTargetRules.cs:56`）
- `UnrealTargetPlatform.Android` —— Android 平台名，**struct 静态属性**（`UEBuildTarget.cs:267`）
- `PrivateDependencyModuleNames` —— 私有依赖模块清单，连库但不对外传递（`ModuleRules.cs:1284`）
- `PrivateIncludePathModuleNames` —— 私有头路径模块清单，只要头不连库（`ModuleRules.cs:1278`）
- `Target.Configuration` —— 当前编译配置，类型 `UnrealTargetConfiguration`（`ReadOnlyTargetRules.cs:58`）
- `UnrealTargetConfiguration.Shipping` —— 发布配置枚举值（`UEBuildTarget.cs:1096`）
- `Utils.MakePathRelativeTo` —— 算相对路径的静态方法（`System\Utils.cs`）
- `ModuleDirectory` —— 本模块目录绝对路径（`ModuleRules.cs:1550`）
- `Target.RelativeEnginePath` —— 引擎根相对当前目录的路径（`ReadOnlyTargetRules.cs:652`）
- `AdditionalPropertiesForReceipt` —— 构建回执附加属性列表（`ModuleRules.cs:1438`）
- `Path.Combine` —— .NET 拼路径方法（`System.IO`）

**这段在干什么**

| 行 | 干什么 |
|---|---|
| `GoogleGameSDK` | Android 私有依赖 |
| `VulkanRHI`（仅头路径） | `AndroidPlatformMisc` 要用它的头，但不链接 |
| `libGPUCounters` / `heapprofd` | 只在非 Shipping 加：GPU 性能计数器 + Google 内存分析器 |
| `Utils.MakePathRelativeTo(...)` | 算出本模块相对引擎根的路径 |
| `AdditionalPropertiesForReceipt.Add("AndroidPlugin", ...)` | **把 `Core_APL.xml` 注册进 Android 打包流程**——下一课那个 73 行 xml 就是被这一行引用的 |

> 类比：最后一行是「把 xml 说明书塞进 Android 打包机的插槽」，没有它，xml 写了也没人读。

### 3.4 依赖清单（第 33~65 行）

```csharp
		PrivateDependencyModuleNames.Add("AtomicQueue");
		PrivateDependencyModuleNames.Add("AutoRTFM");
		PrivateDependencyModuleNames.Add("BLAKE3");
		PrivateDependencyModuleNames.Add("OodleDataCompression");
		PrivateDependencyModuleNames.Add("xxhash");

		PublicDependencyModuleNames.Add("GuidelinesSupportLibrary");
		PublicDependencyModuleNames.Add("TraceLog");
		PublicDependencyModuleNames.Add("AtomicQueue");
```

**本段新元素**

- `PublicDependencyModuleNames` —— 公开依赖模块清单，连库且对外传递（`ModuleRules.cs:1273`）

**这段在干什么**

| 清单 | 成员 | 含义 |
|---|---|---|
| `PrivateDependencyModuleNames` | `AtomicQueue` / `AutoRTFM` / `BLAKE3` / `OodleDataCompression` / `xxhash` | 只在本模块 `.cpp` 里用，不对外暴露 |
| `PublicDependencyModuleNames` | `GuidelinesSupportLibrary` / `TraceLog` / `AtomicQueue` | 对外暴露，包含 Core 头的模块也会拿到 |

> ⚠️ **`AtomicQueue` 在第 33 行和第 41 行各出现一次**（私有 + 公有），`AutoRTFM` 在第 34 行和第 300 行各出现一次——**引擎原样，不许去重**。

```csharp
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"DerivedDataCache",
				"TargetPlatform",

				"Json",
				"RSA"

			}
			);

		PublicIncludePathModuleNames.AddRange(
			new string[] {
				"AutoRTFM",
				"ImageCore"
			}
			);
```

**本段新元素**

- `PublicIncludePathModuleNames` —— 公开头路径模块清单，只要头且对外传递（`ModuleRules.cs:1268`）

**这段在干什么**

| 清单 | 成员 | 含义 |
|---|---|---|
| `PrivateIncludePathModuleNames` | `DerivedDataCache` / `TargetPlatform` / `Json` / `RSA` | **只要头路径，不链接** |
| `PublicIncludePathModuleNames` | `AutoRTFM` / `ImageCore` | 同上，但对外传递 |

> 类比：`DependencyModuleNames` 是「连人带设备一起借」，`IncludePathModuleNames` 是「只借说明书，不借设备」。

```csharp
		if (Target.bBuildEditor == true)
		{
			PrivateIncludePathModuleNames.Add("DirectoryWatcher");
			DynamicallyLoadedModuleNames.Add("DirectoryWatcher");
		}
```

**本段新元素**

- `Target.bBuildEditor` —— 是否在编编辑器（`ReadOnlyTargetRules.cs:190`）
- `DynamicallyLoadedModuleNames` —— 运行时动态加载的模块清单（`ModuleRules.cs:1428`）

**这段在干什么**：只有编编辑器时才挂 `DirectoryWatcher`（头路径 + 动态加载）。

> 类比：`DirectoryWatcher` 是「编辑器才用的监控摄像头」，游戏包里不需要。

### 3.5 平台分支（第 67~220 行）

**Win64（第 67~71 行）**

```csharp
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDefinitions.Add("PLATFORM_BUILDS_LIBPAS=1");
			PrivateDependencyModuleNames.Add("libpas");
		}
```

**本段新元素**

- `UnrealTargetPlatform.Win64` —— Win64 平台名（`UEBuildTarget.cs:252`）
- `PrivateDefinitions` —— 私有宏定义清单，不对外传递（`ModuleRules.cs:1397`）

**这段在干什么**：Win64 用 libpas 分配器，定义 `PLATFORM_BUILDS_LIBPAS=1` 并挂 `libpas` 模块。

**Windows 平台组（第 73~115 行）**

```csharp
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);

			if (Target.WindowsPlatform.Architecture != UnrealArch.Arm64)
			{
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"IntelTBB",
					"IntelVTune"
					);
			}
```

**本段新元素**

- `Target.Platform.IsInGroup(...)` —— 平台对象自己的方法，判断是否属于某平台组（`UEBuildTarget.cs:234`）
- `UnrealPlatformGroup.Windows` —— Windows 平台组，struct 静态属性（`UEBuildTarget.cs:520`）
- `AddEngineThirdPartyPrivateStaticDependencies` —— 批量加引擎第三方静态依赖（`ModuleRules.cs:1599`）
- `Target.WindowsPlatform` —— Windows 专属设置入口（`ReadOnlyTargetRules.cs:624`）
- `Target.WindowsPlatform.Architecture` —— Windows 目标架构（`UEBuildWindows.cs:945`）
- `UnrealArch.Arm64` —— ARM64 架构，struct 静态属性（`UEBuildTarget.cs:836`）

**这段在干什么**：用「平台组」判断（比 `== Win64` 更宽，含 Win32）；所有 Windows 平台都要 `zlib`；x64 才加 `IntelTBB`（并行库）+ `IntelVTune`（性能分析）。

```csharp
			// 不希望静态分析器去跑第三方代码
			if (Target.StaticAnalyzer == StaticAnalyzer.None) 
			{
				PrivateDependencyModuleNames.Add("mimalloc");
				PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");
			}
```

**本段新元素**

- `Target.StaticAnalyzer` —— 静态分析器选择（`ReadOnlyTargetRules.cs:447`）
- `StaticAnalyzer.None` —— 不做静态分析枚举值（`TargetRules.cs:228`）

**这段在干什么**：没开静态分析时才用 mimalloc（分析器跑第三方代码会报一堆噪音）。

```csharp
			if (Target.WindowsPlatform.bUseBundledDbgHelp)
			{
				PublicDelayLoadDLLs.Add("DBGHELP.DLL");
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=1");
				RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/DbgHelp/dbghelp.dll");
			}
			else
			{
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=0");
			}
			PrivateDefinitions.Add("YIELD_BETWEEN_TASKS=1");
```

**本段新元素**

- `Target.WindowsPlatform.bUseBundledDbgHelp` —— 是否用引擎自带 dbghelp.dll（`UEBuildWindows.cs:1059`）
- `PublicDelayLoadDLLs` —— 延迟加载 DLL 清单（`ModuleRules.cs:1392`）
- `RuntimeDependencies` —— 运行时依赖文件清单，随目标暂存（`ModuleRules.cs:1433`）

**这段在干什么**：用自带 dbghelp 时延迟加载 + 打包带上；两种情况下都定义 `USE_BUNDLED_DBGHELP`，只是值不同。

```csharp
			if (Target.WindowsPlatform.bPixProfilingEnabled && Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.Add("WinPixEventRuntime");
			}

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
			}
		}
```

**本段新元素**

- `Target.WindowsPlatform.bPixProfilingEnabled` —— 是否启用 PIX GPU 调试（`UEBuildWindows.cs:959`）
- `Target.Type` —— 目标类型（`ReadOnlyTargetRules.cs:70`）
- `TargetType.Program` —— 独立程序枚举值（`TargetRules.cs:45`）
- `PublicDefinitions` —— 公开宏定义清单，对外传递（`ModuleRules.cs:1402`）

**这段在干什么**：PIX 只在非 Shipping 挂；内存标签追踪只在「非 Shipping 且非 Program」时打开。

**Mac（第 116~133 行）**

```csharp
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"IntelTBB",
				"zlib",
				"PLCrashReporter"
				);
			PublicFrameworks.AddRange(new string[] { "Cocoa", "Carbon", "IOKit", "Security", "UniformTypeIdentifiers", "Network", "ScreenCaptureKit", "SystemConfiguration" });

			PrivateDependencyModuleNames.Add("mimalloc");
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			if (Target.bBuildEditor == true)
			{
				string XcodeRoot = Utils.RunLocalProcessAndReturnStdOut("/usr/bin/xcode-select", "--print-path");
				PublicAdditionalLibraries.Add(XcodeRoot + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/PrivateFrameworks/MultitouchSupport.framework/Versions/Current/MultitouchSupport.tbd");
			}
		}
```

**本段新元素**

- `UnrealTargetPlatform.Mac` —— Mac 平台名（`UEBuildTarget.cs:257`）
- `PublicFrameworks` —— XCode framework 清单（`ModuleRules.cs:1367`）
- `Utils.RunLocalProcessAndReturnStdOut` —— 跑本地进程拿 stdout（`System\Utils.cs:411`）
- `PublicAdditionalLibraries` —— 附加库文件路径清单（`ModuleRules.cs:1330`）

**这段在干什么**：Mac 挂 3 个第三方库 + 8 个系统框架 + mimalloc；编辑器下跑 `xcode-select --print-path` 拿 Xcode 路径，拼出触控板私有框架 `.tbd` 路径。

> 类比：Mac 这段是「先问系统 Xcode 装在哪，再顺着路径去拿触控板驱动」。

**IOS 平台组（第 134~179 行）**

```csharp
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.IOS))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);
			PublicFrameworks.AddRange(new string[] { "UIKit", "Foundation", "AudioToolbox", "AVFoundation", "GameKit", "CoreVideo", "CoreMedia", "CoreGraphics", "GameController", "SystemConfiguration", "DeviceCheck", "UserNotifications", "Network" });

			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				PublicFrameworks.AddRange(new string[] { "CoreMotion", "AdSupport", "WebKit" });
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"PLCrashReporter"
					);

				PrivateIncludePathModuleNames.Add("MarketplaceKit");
			}
			if (Target.Platform == UnrealTargetPlatform.VisionOS)
			{
				PublicFrameworks.Add("CoreMotion");
			}
```

**本段新元素**

- `Target.IsInPlatformGroup(...)` —— Target 的便捷方法，判断当前平台是否属于某平台组（`ReadOnlyTargetRules.cs:696`）
- `UnrealPlatformGroup.IOS` —— iOS 平台组（含 iOS + tvOS + VisionOS）（`UEBuildTarget.cs:535`）
- `UnrealTargetPlatform.IOS` —— iOS 平台名（`UEBuildTarget.cs:262`）
- `UnrealTargetPlatform.VisionOS` —— VisionOS 平台名（`UEBuildTarget.cs:287`）

**这段在干什么**：`IsInPlatformGroup(IOS)` 一次匹配 iOS / tvOS / VisionOS，13 个框架全挂；只有真 iOS 再加 `CoreMotion` / `AdSupport` / `WebKit` + `PLCrashReporter` + `MarketplaceKit`；VisionOS 只加 `CoreMotion`。

```csharp
			if (Target.bCompileAgainstApplicationCore)
			{
				PrivateIncludePathModuleNames.Add("ApplicationCore");
			}

			bool bSupportAdvertising = Target.Platform == UnrealTargetPlatform.IOS;
			if (bSupportAdvertising)
			{
				PublicFrameworks.AddRange(new string[] { "iAD" });
			}

			// 为内嵌 Dll 导出 Core 的符号
			ModuleSymbolVisibility = ModuleRules.SymbolVisibility.VisibileForDll;
			
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
				{
					PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
					PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				}
			}
		}
```

**本段新元素**

- `Target.bCompileAgainstApplicationCore` —— 是否依赖 ApplicationCore（`ReadOnlyTargetRules.cs:214`）
- `ModuleSymbolVisibility` —— 符号可见性（`ModuleRules.cs:1527`）
- `ModuleRules.SymbolVisibility.VisibileForDll` —— Dll 构建时导出符号枚举值（`ModuleRules.cs:265`，引擎原文拼写 `Visibile`）

**这段在干什么**：编 ApplicationCore 时加它的头路径；只有 iOS 支持广告（`iAD`）；导出 Core 符号给内嵌 Dll；iOS 非 Shipping 打开 4 个内存追踪宏。

**Android 平台组（第 180~200 行）**

```csharp
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"cxademangle",
				"zlib",
				"libunwind"
				);

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				PrivateDefinitions.Add("UE_CALLSTACK_TRACE_ANDROID_USE_STACK_FRAMES_WALKING=1");

				// 支持对 libc.so 的 malloc 做内存追踪
				PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Build", "Android", "Prebuilt", "ScudoMemoryTrace"));
				PrivateDefinitions.Add("UE_MEMORY_TRACE_ANDROID_ENABLE_SCUDO_TRACING_SUPPORT=1");
			}
		}
```

**本段新元素**

- `UnrealPlatformGroup.Android` —— Android 平台组（`UEBuildTarget.cs:550`）
- `PrivateIncludePaths` —— 私有 include 目录清单（`ModuleRules.cs:1310`）
- `EngineDirectory` —— 引擎根目录绝对路径（`ModuleRules.cs:1537`）

**这段在干什么**：Android 挂 `cxademangle`（C++ 符号还原）/ `zlib` / `libunwind`（堆栈回溯）；非 Shipping 打开 5 个内存追踪宏 + Scudo 追踪头路径。

**Unix 平台组（第 201~220 行）**

```csharp
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib",
				"jemalloc"
				);

			// Core 用到 dlopen()
			PublicSystemLibraries.Add("dl");

			PrivateDependencyModuleNames.Add("mimalloc");
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
			}
		}
```

**本段新元素**

- `UnrealPlatformGroup.Unix` —— Unix 平台组（`UEBuildTarget.cs:540`）
- `PublicSystemLibraries` —— 系统库名清单，按名找（`ModuleRules.cs:1362`）

**这段在干什么**：Unix 挂 `zlib` / `jemalloc`；`PublicSystemLibraries.Add("dl")` 是因为 Core 用 `dlopen()`；非 Shipping 打开 3 个内存追踪宏（比 Android 少一个 `MAX_FRAMES`）。

> 类比：`PublicSystemLibraries` 是「跟操作系统借的库」，`AddEngineThirdPartyPrivateStaticDependencies` 是「跟引擎自带的第三方借的库」——两个不同的仓库。

### 3.6 通用开关（第 222~311 行）

```csharp
		if (Target.bCompileICU == true)
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "ICU");
		}
		PublicDefinitions.Add("UE_ENABLE_ICU=" + (Target.bCompileICU ? "1" : "0")); // 在代码库里启用/禁用（=1/=0）ICU。注意：这个开关是集成 ICU 期间用的，之后会移除。
```

**本段新元素**

- `Target.bCompileICU` —— 是否编 ICU 国际化组件（`ReadOnlyTargetRules.cs:169`）

**这段在干什么**：编 ICU 时才挂库；`UE_ENABLE_ICU` **无论编不编都定义**，值随开关变。

```csharp
		// 如果正在和引擎一起编译，就加上 Core 对引擎的依赖
		if (Target.bCompileAgainstEngine && !Target.bBuildRequiresCookedData)
		{
			DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
			DynamicallyLoadedModuleNames.AddRange(new string[] { "Virtualization" });
		}
```

**本段新元素**

- `Target.bCompileAgainstEngine` —— 是否和引擎一起编（`ReadOnlyTargetRules.cs:210`）
- `Target.bBuildRequiresCookedData` —— 是否要求已 cook 的数据（`ReadOnlyTargetRules.cs:192`）

**这段在干什么**：和引擎一起编、且不要求已 cook 数据（编辑器 / 开发版）时，动态加载 `DerivedDataCache`（DDC）+ `Virtualization`。

```csharp
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop))
        {
			// 在桌面平台上启用更多崩溃信息。
			PublicDefinitions.Add("WITH_ADDITIONAL_CRASH_CONTEXTS=1");
		}
```

**本段新元素**

- `UnrealPlatformGroup.Desktop` —— 桌面平台组（Win/Mac/Linux）（`UEBuildTarget.cs:555`）

**这段在干什么**：桌面平台打开更多崩溃上下文。

> ⚠️ 第 236 行 `        {` 用的是**空格缩进**（8 个空格），不是 Tab——引擎原文如此，照抄。

```csharp
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			// Visual Studio 的并发可视化扩展
			PrivateDependencyModuleNames.Add("ConcurrencyVisualizer");
			
			// Super Luminal。
			PrivateDependencyModuleNames.Add("SuperLuminal");
			
			// Visual Studio 性能分析器
			PrivateDependencyModuleNames.Add("VSPerfExternalProfiler");
		}
```

**这段在干什么**：Windows 非 Shipping 挂 3 个性能分析器（VS 并发可视化 / Superluminal / VS 性能分析器）。

```csharp
		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Apple) ||
			Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("IS_RUNNING_GAMETHREAD_ON_EXTERNAL_THREAD=1");
		}
```

**本段新元素**

- `UnrealPlatformGroup.Apple` —— Apple 平台组（`UEBuildTarget.cs:530`）

**这段在干什么**：Apple 或 Android 上游戏线程跑在外部线程（这些平台主线程不是游戏线程）。

```csharp
		// 设置一个宏，让 FApp::GetBuildTargetType() 能识别客户端目标
		if (Target.Type == TargetRules.TargetType.Client)
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=1");
		}
		else
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=0");
		}
```

**本段新元素**

- `TargetRules.TargetType.Client` —— 客户端目标类型枚举值（`TargetRules.cs:35`）

**这段在干什么**：**呼应上一课两张 `Target.cs` 的 `Type` 字段**——是 Client 就定义 `IS_CLIENT_TARGET=1`，否则 `=0`。

```csharp
		// 设置用于包含 / 排除 Iris 对 UObject 改动的定义。注意：只需要这个定义，因为我们完全不依赖 Iris。
		if (Target.bUseIris == true)
		{
			PublicDefinitions.Add("UE_WITH_IRIS=1");
		}
		else
		{
			PublicDefinitions.Add("UE_WITH_IRIS=0");
		}
```

**本段新元素**

- `Target.bUseIris` —— 是否用 Iris 网络复制（`ReadOnlyTargetRules.cs:181`）

**这段在干什么**：只定义 `UE_WITH_IRIS` 宏，**不挂 Iris 模块**（注释明说「完全不依赖 Iris」）。

```csharp
		if (Target.Platform == UnrealTargetPlatform.Win64
			&& Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("PLATFORM_SUPPORTS_PLATFORM_EVENTS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_VIRTUAL_MEMORY_HOOKS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_MODULE_DIAGNOSTICS=1");
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_CALLSTACK=1");
			PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
		}
```

**这段在干什么**：Win64 非 Shipping 打开 4 个 Win32 追踪能力 + 内存追踪可用。

```csharp
		// 临时性的东西。
		PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));

		PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));

		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_LTCG=" + (Target.bAllowLTCG ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG=" + (Target.bPGOOptimize ? "1" : "0"));
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG_PROFILING=" + (Target.bPGOProfile ? "1" : "0"));

		PrivateDefinitions.Add("UE_DEFINE_LEGACY_MATH_CONSTANT_MACRO_NAMES=0");
```

**本段新元素**

- `Target.bAllowLTCG` —— 是否允许 LTCG 链接时代码生成（`ReadOnlyTargetRules.cs:465`）
- `Target.bPGOOptimize` —— 是否 PGO 优化（`ReadOnlyTargetRules.cs:473`）
- `Target.bPGOProfile` —— 是否 PGO 采样（`ReadOnlyTargetRules.cs:471`）

**这段在干什么**：`PLATFORM_SUPPORTS_BINARYCONFIG` 调本文件自己的 `SupportsBinaryConfig(Target)`；`WITH_MALLOC_STOMP` 调本文件自己的 `bWithMallocStomp` 属性；LTCG / PGO 三个宏跟着 Target 开关走；关掉旧数学常量宏名。

```csharp
		PrivateDependencyModuleNames.Add("AutoRTFM");

		CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Error;

		IWYUSupport = IWYUSupport.KeepAsIs;

		if (Target.WindowsPlatform.bEnableInstrumentation)
		{
			PrivateDependencyModuleNames.Add("Detours");
		}

		PublicDefinitions.Add("UE_MERGED_MODULES=" + (Target.bMergeModules ? "1" : "0"));
	}
```

**本段新元素**

- `CppCompileWarningSettings` —— 警告设置总对象（`ModuleRules.cs:982`）
- `WarningLevel.Error` —— 警告当错误枚举值（`ModuleRules.cs:38`）
- `IWYUSupport` —— IWYU 支持程度（`ModuleRules.cs:1222`）
- `IWYUSupport.KeepAsIs` —— 保持原样不强制 IWYU 枚举值（`ModuleRules.cs:80`）
- `Target.WindowsPlatform.bEnableInstrumentation` —— 是否启用插桩 Detours（`UEBuildWindows.cs:985`）
- `Target.bMergeModules` —— 是否合并模块（`ReadOnlyTargetRules.cs:572`）

**这段在干什么**：`AutoRTFM`（第 300 行）**与第 34 行重复，引擎原样**；不安全类型转换当**错误**处理；IWYU 保持原样；开插桩时挂 `Detours`；`UE_MERGED_MODULES` 跟合并开关走。

### 3.7 两个自定义成员（第 314~329 行）

```csharp
	// 决定当前平台能否使用二进制配置
	protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
	{
		return true;// Target.Platform != UnrealTargetPlatform.Android;
	}
```

**这段在干什么**：`protected virtual` 表示子类可重写；恒返回 `true`；`//` 后面是被注释掉的旧条件（属于代码，不翻译，`//` 前无空格）。

```csharp
	// 决定当前平台能否使用内存分配器校验（也就是 MallocStomp）。
	// 运行时校验必须通过 '-stompmalloc' 命令行参数开启。
	protected virtual bool bWithMallocStomp
	{
		get => 
			Target.Configuration != UnrealTargetConfiguration.Shipping &&
			(Target.Platform == UnrealTargetPlatform.Mac ||
			 Target.Platform == UnrealTargetPlatform.Linux ||
			 Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
			 Target.Platform == UnrealTargetPlatform.Win64);
	}
```

**本段新元素**

- `UnrealTargetPlatform.Linux` —— Linux 平台名（`UEBuildTarget.cs:272`）
- `UnrealTargetPlatform.LinuxArm64` —— Linux ARM64 平台名（`UEBuildTarget.cs:277`）

**这段在干什么**：**只读属性**（只有 `get`，没有 `set`）；`get =>` 是表达式体写法；条件 = 非 Shipping **且** 平台是 Mac / Linux / LinuxArm64 / Win64。

> 类比：`bWithMallocStomp` 是「这个平台能不能开内存踩踏检测」的开关——只在开发机上开，Shipping 包永远关。

---

## 四、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `Core.Build.cs` | `Core.Build.cs` | **完全一致** |
| 路径 | `Engine\Source\Runtime\Core\` | `ue_5.6\Engine\Source\Runtime\Core\` | 只换根 |
| 类名 | `Core` | `Core` | 完全一致 |
| 代码行 | 330 行 | 332 行 | **只多 2 行中文注释**（类上方 + 构造函数上方） |
| 英文注释 | 19 处 | 0 处 | 全部译成中文 |
| 被注释掉的代码 | `// Target.Platform != ...` | 原样保留 | 属于代码，不翻译 |
| 引擎错字 | `plVatform` / `VisibileForDll` | 原样保留 | 一比一照抄 |
| 空白行 | 第 72/93/245/248 行带 3 Tab，第 278 行带 2 Tab | 原样保留 | 不许「整理干净」 |

> 除注释中文化外，**代码一字不差**。

---

## 五、收尾

到这里 `Core.Build.cs` 就完了（330 行 → 332 行）。

**结构**：一个 `ModuleRules` 子类，构造函数里 5 段——骨架（PCH + 合批）→ 依赖清单 → 平台分支（Win64 / Windows / Mac / IOS / Android / Unix）→ 通用开关 → 两个自定义成员。

**关键概念**：
- `Private*` vs `Public*`：私有只给自己用，公有对外传递
- `DependencyModuleNames` vs `IncludePathModuleNames`：前者连库，后者只要头
- `AddEngineThirdPartyPrivateStaticDependencies` vs `PublicSystemLibraries`：引擎自带第三方 vs 操作系统库
- `Target.Platform == X` vs `Target.Platform.IsInGroup(Group)` vs `Target.IsInPlatformGroup(Group)`：单平台 / 平台对象方法 / Target 便捷方法
- `PrivateDefinitions` vs `PublicDefinitions`：宏是否对外传递

---

## 待确认

1. 330 行代码看懂了吗？照着「二、完整代码」抄一遍到 `ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs`。
2. 抄的时候注意三处：`return true;//` 的 `//` 前无空格、`plVatform` 错字、第 72/93/245/248/278 行的空白行。
3. 抄完说「下一步」，我先验收这份文件，再教 `Core_APL.xml`（73 行）。

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
- 基类：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行）
- 目标规则：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（748 行）
- 平台定义：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildTarget.cs`（6352 行）
- Windows 规则：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\UEBuildWindows.cs`
- 工具方法：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\Utils.cs`
- 你的还原：`ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs`