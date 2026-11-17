# 09 — `Core.Build.cs` 逐行注释版（超详细）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
> **用途**：看不懂代码时看这份——**每一行上面都有详细中文注释**，讲清「这行在干什么 + 为什么这么写 + 涉及的类型从哪来」。
> **注意**：这份是**讲解版**，不是交付版。交付版（干净可照抄）在 `01_Core.Build.cs_完整代码.md`。

---

## 一、开头与类声明（第 1~10 行）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。
// 这是版权声明，所有引擎源码第一行都是它，交付时译成中文。

// 引入 UBT（UnrealBuildTool）程序集。
// 本文件用到的 ModuleRules（基类）、ReadOnlyTargetRules（目标规则）、
// UnrealTargetPlatform（平台类型）、UnrealPlatformGroup（平台组）、UnrealArch（架构）
// 全部来自这个程序集。
using UnrealBuildTool;

// 引入 .NET 基础类型命名空间。
// 本文件其实没直接用到 System 里的类型，但引擎原样保留，不许删。
using System;

// 引入 System.IO 命名空间。
// 本文件第 30 行要用 Path.Combine(...) 拼路径，Path 类就在 System.IO 里。
using System.IO;

// 声明 Core 模块类，继承 ModuleRules。
// 类名必须等于模块名（Core），UBT 靠类名找到这份构建单子。
// ModuleRules 是所有 *.Build.cs 的基类，定义在 ModuleRules.cs:103。
public class Core : ModuleRules
{
	// 构造函数：参数 Target 是「当前编译目标」的只读视图（ReadOnlyTargetRules）。
	// : base(Target) 把 Target 交给基类 ModuleRules 保存（ModuleRules.cs:1584）。
	// 不写 : base(Target) 的话，基类里的 Target 和 CppCompileWarningSettings 都是 null，
	// 后面所有 Target.xxx 都会空引用崩溃。
	public Core(ReadOnlyTargetRules Target) : base(Target)
	{
```

---

## 二、PCH 与 Unity 合批（第 11~15 行）

```csharp
		// 设置 Unity 合批时「每个 unity cpp 文件允许包含的源码字节上限」。
		// 491520 字节 = 480 KB（491520 / 1024）。
		// 引擎注释说这是用 UBT 的 ProfileUnitySizes 模式实测出来的最佳值。
		// 属性定义在 ModuleRules.cs:1162，默认值 0（0 = 不覆盖，用 Target 的全局值）。
		// Unity 合批 = 把多个 .cpp 拼进一个文件一起编，减少重复解析头文件的时间。
		NumIncludedBytesPerUnityCPPOverride = 491520;

		// 设置「私有预编译头」的相对路径（相对本模块目录）。
		// 私有 PCH 只给本模块的 .cpp 用，设了它就不再用共享 PCH。
		// 属性定义在 ModuleRules.cs:793，默认 null。
		// 对应真实文件：Core\Private\CorePrivatePCH.h（11 行）。
		PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";

		// 设置「共享预编译头」的相对路径。
		// 共享 PCH 是本模块对外提供的，别的模块也能吃。
		// 属性定义在 ModuleRules.cs:799，默认 null。
		// 对应真实文件：Core\Public\CoreSharedPCH.h（263 行 / 252 个 include）。
		SharedPCHHeaderFile = "Public/CoreSharedPCH.h";
```

---

## 三、Android 块（第 17~31 行）

```csharp
		// 往「私有依赖模块清单」里加 BuildSettings 模块。
		// 私有依赖 = 本模块 .cpp 能用它的头 + 链接它的库，但不对外传递。
		// 清单定义在 ModuleRules.cs:1284，类型 List<string>。
		PrivateDependencyModuleNames.Add("BuildSettings");

		// 判断「这次编的是不是 Android 平台」。
		// Target.Platform 是 ReadOnlyTargetRules.cs:56 的属性，返回 UnrealTargetPlatform。
		// UnrealTargetPlatform 是 partial struct（不是 enum），定义在 UEBuildTarget.cs:31。
		// UnrealTargetPlatform.Android 是它的静态属性，定义在 UEBuildTarget.cs:267。
		// == 运算符重载在 UEBuildTarget.cs:107，比较两个平台的内部 Id。
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// 链 Google 游戏 SDK（第三方模块）。
			// 真实位置：Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs
			// 里面有 libswappy_static.a 等 Android 游戏库。
			PrivateDependencyModuleNames.Add("GoogleGameSDK");
			
			// 往「私有头路径模块清单」里加 VulkanRHI。
			// 私有头路径模块 = 只要它的头文件路径（能 include），不链接它的库。
			// 清单定义在 ModuleRules.cs:1278。
			// 为什么只要头：Core 里的 AndroidPlatformMisc.cpp 要读 Vulkan 的常量/类型定义，
			// 但不调用 Vulkan 的函数，所以不需要链接 Vulkan 的库。
			// 真实位置：Engine\Source\Runtime\VulkanRHI\VulkanRHI.Build.cs
			PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 需要

			// 判断「这次编的不是发布包」。
			// Target.Configuration 是 ReadOnlyTargetRules.cs:58 的属性，返回 UnrealTargetConfiguration。
			// UnrealTargetConfiguration 是 enum，定义在 UEBuildTarget.cs:1066，6 个值：
			// Unknown / Debug / DebugGame / Development / Test / Shipping。
			// Shipping = 正式发布包，关掉所有调试功能。
			if (Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				// 链 GPU 性能计数器库（ARM CPU / ARM Mali GPU）。
				// 真实位置：Engine\Source\ThirdParty\libGPUCounters\libGPUCounters.Build.cs
				// 里面是 libGPUCounters.so。
				// 只在非 Shipping 加：性能计数器有开销，发布包不要。
				PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 与 ARM Mali GPU 的性能计数器
				// 链 Google 堆内存分析器。
				// 真实位置：Engine\Source\ThirdParty\heapprofd\heapprofd.Build.cs
				// heapprofd = heap profiler daemon，把 UE 的自定义分配器暴露给
				// Android Studio 的 Memory Profiler。
				// 只在非 Shipping 加：会暴露内部内存布局，发布包不要。
				PrivateDependencyModuleNames.Add("heapprofd");      // 把自定义分配器暴露给 Google 的内存分析器
			}
			// 算出「Core 模块相对引擎根」的路径。
			// Utils.MakePathRelativeTo 是静态方法，定义在 System\Utils.cs:740。
			// 参数 1：ModuleDirectory（ModuleRules.cs:1550）= 本模块目录绝对路径，
			//         例如 D:\...\UE_5.6\Engine\Source\Runtime\Core
			// 参数 2：Target.RelativeEnginePath（ReadOnlyTargetRules.cs:652）= 引擎根相对当前目录的路径，
			//         例如 D:\...\UE_5.6\Engine\
			// 返回：Source\Runtime\Core（相对引擎根的路径）
			// 为什么不用绝对路径：换台机器引擎装在别处，绝对路径就废了；回执里存相对路径更可移植。
			string ModulePath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			// 把 Core_APL.xml 注册进 Android 打包流程。
			// AdditionalPropertiesForReceipt 是 ReceiptPropertyList（ModuleRules.cs:1438），
			// 本质是一个键值对列表，Add(name, value) 往里塞一对。
			// key = "AndroidPlugin"（固定字符串，Android 打包器按这个 key 找）
			// value = Path.Combine(ModulePath, "Core_APL.xml") = Source\Runtime\Core\Core_APL.xml
			// 数据流：这里 Add → UEBuildTarget.cs:2407 写进 .target 回执 →
			//         UEDeployAndroid.cs:5953 按 key 读出来 → 打包时读 xml 往 Android 工程插内容。
			// Core_APL.xml 是 UPL（Unreal Plugin Language）脚本，73 行。
			// 没有这一行，xml 写了也没人读。
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "Core_APL.xml"));
		}
```

---

## 四、依赖清单（第 33~65 行）

```csharp
		// 私有依赖：AtomicQueue（无锁原子队列，多线程安全的数据结构）
		PrivateDependencyModuleNames.Add("AtomicQueue");
		// 私有依赖：AutoRTFM（Automatic Real-Time Transactional Memory，自动事务内存）
		PrivateDependencyModuleNames.Add("AutoRTFM");
		// 私有依赖：BLAKE3（现代哈希算法，比 SHA 快）
		PrivateDependencyModuleNames.Add("BLAKE3");
		// 私有依赖：OodleDataCompression（Oodle 压缩库，Epic 收购的压缩方案）
		PrivateDependencyModuleNames.Add("OodleDataCompression");
		// 私有依赖：xxhash（极快的非加密哈希算法）
		PrivateDependencyModuleNames.Add("xxhash");

		// 公开依赖：GuidelinesSupportLibrary（GSL，C++ 核心指南支持库，微软出品）
		// 公开依赖 = 本模块能用 + 包含本模块头的其他模块也能用（自动传递）。
		// 清单定义在 ModuleRules.cs:1273。
		PublicDependencyModuleNames.Add("GuidelinesSupportLibrary");
		// 公开依赖：TraceLog（性能追踪日志系统）
		PublicDependencyModuleNames.Add("TraceLog");
		// 公开依赖：AtomicQueue（与第 33 行重复！引擎原样，不许去重）
		// 为什么重复：私有和公开是两个独立清单，引擎作者两边都加了。
		PublicDependencyModuleNames.Add("AtomicQueue");

		// 往「私有头路径模块清单」批量加 4 个模块（只要头，不连库）。
		// AddRange 是 List<T> 的标准方法，一次加多个。
		// DerivedDataCache = DDC 派生数据缓存
		// TargetPlatform = 目标平台模块
		// Json = JSON 解析
		// RSA = RSA 加密
		// 注意：中间的空行是引擎原文，照抄。
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"DerivedDataCache",
				"TargetPlatform",

				"Json",
				"RSA"

			}
			);

		// 往「公开头路径模块清单」批量加 2 个模块（只要头，对外传递）。
		// AutoRTFM = 自动事务内存
		// ImageCore = 图像核心（像素格式等）
		PublicIncludePathModuleNames.AddRange(
			new string[] {
				"AutoRTFM",
				"ImageCore"
			}
			);

		// 判断「这次编的是不是编辑器」。
		// Target.bBuildEditor 是 ReadOnlyTargetRules.cs:190 的属性，bool。
		// 为真 = 编的是 UnrealEditor.exe 或编辑器 DLL。
		if (Target.bBuildEditor == true)
		{
			// 加 DirectoryWatcher 的头路径（编辑器要监控文件变化，热重载用）
			PrivateIncludePathModuleNames.Add("DirectoryWatcher");
			// 运行时动态加载 DirectoryWatcher（不静态链接，用到才载入）
			// 清单定义在 ModuleRules.cs:1428。
			DynamicallyLoadedModuleNames.Add("DirectoryWatcher");
		}
```

---

## 五、平台分支（第 67~220 行）

### 5.1 Win64（第 67~71 行）

```csharp
		// 判断「这次编的是不是 Win64」。
		// UnrealTargetPlatform.Win64 定义在 UEBuildTarget.cs:252。
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// 定义私有宏 PLATFORM_BUILDS_LIBPAS=1。
			// 告诉 C++ 代码「这个平台用 libpas 分配器」。
			// 私有宏 = 只在本模块编译时定义，不对外传递。
			// 清单定义在 ModuleRules.cs:1397。
			PrivateDefinitions.Add("PLATFORM_BUILDS_LIBPAS=1");
			// 链 libpas 模块（Apple 系的高性能内存分配器，UE5 移植到全平台）
			PrivateDependencyModuleNames.Add("libpas");
		}
```

### 5.2 Windows 平台组（第 73~115 行）

```csharp
		// 判断「当前平台是否属于 Windows 平台组」。
		// Target.Platform.IsInGroup(...) 是 UnrealTargetPlatform 的实例方法，
		// 定义在 UEBuildTarget.cs:234，内部调 UEBuildPlatform.IsPlatformInGroup。
		// UnrealPlatformGroup.Windows 是平台组静态属性，定义在 UEBuildTarget.cs:520。
		// 用「平台组」比 == Win64 更宽：Win32 / Win64 都算 Windows 组。
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			// 加引擎第三方静态依赖 zlib（压缩库）。
			// AddEngineThirdPartyPrivateStaticDependencies 是 ModuleRules 的方法，
			// 定义在 ModuleRules.cs:1599，内部执行 PrivateDependencyModuleNames.AddRange(...)。
			// 一行顶三行：等价于 PrivateDependencyModuleNames.Add("zlib")。
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);

			// 判断「不是 ARM64 架构」（即 x64）。
			// Target.WindowsPlatform 是 ReadOnlyWindowsTargetRules（ReadOnlyTargetRules.cs:624），
			// 类定义在 UEBuildWindows.cs:902。
			// .Architecture 是 UEBuildWindows.cs:945 的属性，返回 UnrealArch。
			// UnrealArch.Arm64 是架构静态属性，定义在 UEBuildTarget.cs:836。
			if (Target.WindowsPlatform.Architecture != UnrealArch.Arm64)
			{
				// x64 才加 IntelTBB（并行库）+ IntelVTune（性能分析器）
				// ARM64 上这两个库没有对应版本，所以跳过。
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"IntelTBB",
					"IntelVTune"
					);
			}

			// 不希望静态分析器去跑第三方代码
			// 判断「没开静态分析」。
			// Target.StaticAnalyzer 是 ReadOnlyTargetRules.cs:447 的属性，返回 StaticAnalyzer 枚举。
			// StaticAnalyzer.None 定义在 TargetRules.cs:228。
			// 为什么：静态分析器跑第三方代码会报一堆噪音警告，所以只在没开分析时才用 mimalloc。
			if (Target.StaticAnalyzer == StaticAnalyzer.None) 
			{
				// 链 mimalloc 分配器（微软出品的高性能分配器）
				PrivateDependencyModuleNames.Add("mimalloc");
				// 定义宏：这个平台用 mimalloc
				PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");
			}
			
			// 判断「是否使用引擎自带的 dbghelp.dll」。
			// Target.WindowsPlatform.bUseBundledDbgHelp 是 UEBuildWindows.cs:1059 的属性，
			// 真值在 UEBuildWindows.cs:769，默认 true。
			// dbghelp.dll 是 Windows 的崩溃堆栈解析库。
			if (Target.WindowsPlatform.bUseBundledDbgHelp)
			{
				// 延迟加载 DBGHELP.DLL：程序启动不载入，第一次调用才载入。
				// 清单定义在 ModuleRules.cs:1392。
				PublicDelayLoadDLLs.Add("DBGHELP.DLL");
				// 定义宏：用自带 dbghelp
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=1");
				// 打包时把这个 dll 一起带上。
				// RuntimeDependencies 是 RuntimeDependencyList（ModuleRules.cs:1433），
				// 里面的文件会随目标一起 stage 到输出目录。
				// $(EngineDir) 是 UBT 的路径变量，运行时替换成引擎根。
				RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/DbgHelp/dbghelp.dll");
			}
			// 否则（用系统自带的 dbghelp.dll）
			else
			{
				// 定义宏：不用自带 dbghelp
				PrivateDefinitions.Add("USE_BUNDLED_DBGHELP=0");
			}
			// 定义宏：任务之间让出 CPU（避免长时间占用导致卡顿）
			PrivateDefinitions.Add("YIELD_BETWEEN_TASKS=1");

			// 判断「启用 PIX 且不是发布包」。
			// Target.WindowsPlatform.bPixProfilingEnabled 是 UEBuildWindows.cs:959 的属性，
			// 真值在 UEBuildWindows.cs:450，默认 true。
			// PIX 是微软的 GPU 调试器。
			if (Target.WindowsPlatform.bPixProfilingEnabled && Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				// 链 PIX 事件运行时
				PrivateDependencyModuleNames.Add("WinPixEventRuntime");
			}

			// 判断「不是发布包，且不是独立程序」。
			// Target.Type 是 ReadOnlyTargetRules.cs:70 的属性，返回 TargetType 枚举。
			// TargetType.Program 定义在 TargetRules.cs:45（如 ShaderCompileWorker.exe）。
			// 为什么排除 Program：独立程序不需要内存追踪这类游戏功能。
			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				// 打开内存标签追踪（给每个分配打标签，方便查内存泄漏）
				// 公开宏 = 本模块 + 所有依赖本模块的模块都会定义。
				// 清单定义在 ModuleRules.cs:1402。
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
			}
		}
```

### 5.3 Mac（第 116~133 行）

```csharp
		// 否则如果这次编的是 Mac。
		// UnrealTargetPlatform.Mac 定义在 UEBuildTarget.cs:257。
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			// 加三个第三方库：
			// IntelTBB = 并行库
			// zlib = 压缩库
			// PLCrashReporter = 崩溃上报库（第三方开源）
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"IntelTBB",
				"zlib",
				"PLCrashReporter"
				);
			// 加 8 个 macOS 系统框架：
			// Cocoa = macOS 应用框架
			// Carbon = 旧版 macOS API
			// IOKit = 硬件驱动接口
			// Security = 安全/加密
			// UniformTypeIdentifiers = 文件类型识别
			// Network = 网络
			// ScreenCaptureKit = 屏幕捕获
			// SystemConfiguration = 系统配置
			// PublicFrameworks 定义在 ModuleRules.cs:1367。
			PublicFrameworks.AddRange(new string[] { "Cocoa", "Carbon", "IOKit", "Security", "UniformTypeIdentifiers", "Network", "ScreenCaptureKit", "SystemConfiguration" });

			// 链 mimalloc 分配器（Mac 也用）
			PrivateDependencyModuleNames.Add("mimalloc");
			// 定义宏：这个平台用 mimalloc
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			// 如果这次编的是编辑器
			if (Target.bBuildEditor == true)
			{
				// 跑命令 /usr/bin/xcode-select --print-path，拿到 Xcode 安装路径。
				// Utils.RunLocalProcessAndReturnStdOut 定义在 System\Utils.cs:411，
				// 启动本地进程并返回它的 stdout。
				string XcodeRoot = Utils.RunLocalProcessAndReturnStdOut("/usr/bin/xcode-select", "--print-path");
				// 拼出触控板私有框架 MultitouchSupport.tbd 的完整路径并加进附加库。
				// PublicAdditionalLibraries 定义在 ModuleRules.cs:1330。
				// 为什么编辑器才要：编辑器要支持触控板手势，游戏包不需要。
				PublicAdditionalLibraries.Add(XcodeRoot + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/PrivateFrameworks/MultitouchSupport.framework/Versions/Current/MultitouchSupport.tbd");
			}
		}
```

### 5.4 IOS 平台组（第 134~179 行）

```csharp
		// 否则如果当前平台属于 IOS 平台组。
		// Target.IsInPlatformGroup(...) 是 ReadOnlyTargetRules.cs:696 的便捷方法，
		// 内部调 UEBuildPlatform.IsPlatformInGroup(Platform, group)。
		// UnrealPlatformGroup.IOS 定义在 UEBuildTarget.cs:535，
		// 包含 iOS + tvOS + VisionOS（注释原话：making IOS a group allows TVOS to compile IOS code）。
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.IOS))
		{
			// 加 zlib 压缩库
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib"
				);
			// 加 13 个 iOS 系统框架：
			// UIKit = iOS UI 框架
			// Foundation = 基础框架
			// AudioToolbox = 音频工具
			// AVFoundation = 音视频
			// GameKit = 游戏中心
			// CoreVideo = 视频核心
			// CoreMedia = 媒体核心
			// CoreGraphics = 图形核心
			// GameController = 手柄
			// SystemConfiguration = 系统配置
			// DeviceCheck = 设备校验
			// UserNotifications = 用户通知
			// Network = 网络
			PublicFrameworks.AddRange(new string[] { "UIKit", "Foundation", "AudioToolbox", "AVFoundation", "GameKit", "CoreVideo", "CoreMedia", "CoreGraphics", "GameController", "SystemConfiguration", "DeviceCheck", "UserNotifications", "Network" });

			// 如果这次编的是真 iOS（不是 tvOS / VisionOS）
			// UnrealTargetPlatform.IOS 定义在 UEBuildTarget.cs:262。
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				// 再加 3 个框架：
				// CoreMotion = 运动传感器（陀螺仪/加速度计）
				// AdSupport = 广告标识符
				// WebKit = 网页视图
				PublicFrameworks.AddRange(new string[] { "CoreMotion", "AdSupport", "WebKit" });
				// 加 PLCrashReporter 崩溃上报库
				AddEngineThirdPartyPrivateStaticDependencies(Target,
					"PLCrashReporter"
					);

				// 加 MarketplaceKit 的头路径（App Store 相关）
				PrivateIncludePathModuleNames.Add("MarketplaceKit");
			}
			// 如果这次编的是 VisionOS
			// UnrealTargetPlatform.VisionOS 定义在 UEBuildTarget.cs:287。
			if (Target.Platform == UnrealTargetPlatform.VisionOS)
			{
				// 加 CoreMotion 框架（VisionOS 也要运动传感器）
				PublicFrameworks.Add("CoreMotion");
			}

			// 如果依赖 ApplicationCore 模块。
			// Target.bCompileAgainstApplicationCore 是 ReadOnlyTargetRules.cs:214 的属性。
			if (Target.bCompileAgainstApplicationCore)
			{
				// 加 ApplicationCore 的头路径
				PrivateIncludePathModuleNames.Add("ApplicationCore");
			}

			// 判断是否支持广告（只有真 iOS 支持，tvOS / VisionOS 不支持）
			bool bSupportAdvertising = Target.Platform == UnrealTargetPlatform.IOS;
			// 如果支持广告
			if (bSupportAdvertising)
			{
				// 加 iAD 广告框架
				PublicFrameworks.AddRange(new string[] { "iAD" });
			}

			// 为内嵌 Dll 导出 Core 的符号
			// 设置符号可见性为 VisibileForDll（Dll 构建时导出符号）。
			// ModuleSymbolVisibility 定义在 ModuleRules.cs:1527。
			// ModuleRules.SymbolVisibility.VisibileForDll 定义在 ModuleRules.cs:265。
			// 注意：Visibile 是引擎原文拼写（不是 Visible），照抄。
			ModuleSymbolVisibility = ModuleRules.SymbolVisibility.VisibileForDll;
			
			// 如果这次编的是真 iOS
			if (Target.Platform == UnrealTargetPlatform.IOS)
			{
				// 如果不是发布包，且不是独立程序
				if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
				{
					// 打开内存追踪
					PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
					// 打开内存标签追踪
					PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
					// 打开调用栈追踪
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
					// 调用栈最多 64 帧（超过就截断，避免太慢）
					PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				}
			}
		}
```

### 5.5 Android 平台组（第 180~200 行）

```csharp
		// 否则如果当前平台属于 Android 平台组。
		// UnrealPlatformGroup.Android 定义在 UEBuildTarget.cs:550。
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Android))
		{
			// 加三个第三方库：
			// cxademangle = C++ 符号还原（把 mangled 名字还原成可读函数名）
			// zlib = 压缩库
			// libunwind = 堆栈回溯库
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"cxademangle",
				"zlib",
				"libunwind"
				);

			// 如果不是发布包，且不是独立程序
			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				// 打开内存追踪
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				// 打开内存标签追踪
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				// 打开调用栈追踪
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
				// 调用栈最多 64 帧
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_MAX_FRAMES=64");
				// Android 专用：用 stack frames walking 方式回溯（比默认方式快）
				PrivateDefinitions.Add("UE_CALLSTACK_TRACE_ANDROID_USE_STACK_FRAMES_WALKING=1");

				// 支持对 libc.so 的 malloc 做内存追踪
				// 加 Scudo 内存追踪的头路径。
				// PrivateIncludePaths 定义在 ModuleRules.cs:1310（直接加目录，不是模块名）。
				// EngineDirectory 是 ModuleRules.cs:1537 的属性（引擎根绝对路径）。
				PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Build", "Android", "Prebuilt", "ScudoMemoryTrace"));
				// 打开 Scudo 追踪支持（Scudo 是 Android 的内存分配器）
				PrivateDefinitions.Add("UE_MEMORY_TRACE_ANDROID_ENABLE_SCUDO_TRACING_SUPPORT=1");
			}
		}
```

### 5.6 Unix 平台组（第 201~220 行）

```csharp
		// 否则如果当前平台属于 Unix 平台组。
		// UnrealPlatformGroup.Unix 定义在 UEBuildTarget.cs:540。
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			// 加两个第三方库：
			// zlib = 压缩库
			// jemalloc = 高性能分配器（FreeBSD 出品）
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib",
				"jemalloc"
				);

			// Core 用到 dlopen()
			// 加系统库 dl（dlopen / dlsym / dlclose 要用）。
			// PublicSystemLibraries 定义在 ModuleRules.cs:1362，
			// 按名字让链接器去系统路径找（"dl" → -ldl）。
			PublicSystemLibraries.Add("dl");

			// 链 mimalloc 分配器（Unix 也用）
			PrivateDependencyModuleNames.Add("mimalloc");
			// 定义宏：这个平台用 mimalloc
			PrivateDefinitions.Add("PLATFORM_BUILDS_MIMALLOC=1");

			// 如果不是发布包，且不是独立程序
			if (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Type != TargetType.Program)
			{
				// 打开内存追踪
				PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
				// 打开内存标签追踪
				PublicDefinitions.Add("UE_MEMORY_TAGS_TRACE_ENABLED=1");
				// 打开调用栈追踪
				PublicDefinitions.Add("UE_CALLSTACK_TRACE_ENABLED=1");
				// 注意：Unix 比 Android 少一个 UE_CALLSTACK_TRACE_MAX_FRAMES，引擎原样。
			}
		}
```

---

## 六、通用开关（第 222~311 行）

```csharp
		// 如果编 ICU（International Components for Unicode，国际化组件）。
		// Target.bCompileICU 是 ReadOnlyTargetRules.cs:169 的属性。
		if (Target.bCompileICU == true)
		{
			// 加 ICU 第三方库
			AddEngineThirdPartyPrivateStaticDependencies(Target, "ICU");
		}
		// 定义宏 UE_ENABLE_ICU：编了就是 1，没编就是 0。
		// 注意：无论编不编都定义，只是值不同。
		// 三元运算符 (条件 ? 真值 : 假值) 是 C# 语法。
		PublicDefinitions.Add("UE_ENABLE_ICU=" + (Target.bCompileICU ? "1" : "0")); // 在代码库里启用/禁用（=1/=0）ICU。注意：这个开关是集成 ICU 期间用的，之后会移除。

		// 如果正在和引擎一起编译，就加上 Core 对引擎的依赖
		// 条件 1：Target.bCompileAgainstEngine（ReadOnlyTargetRules.cs:210）= 和引擎一起编
		// 条件 2：!Target.bBuildRequiresCookedData（ReadOnlyTargetRules.cs:192）= 不要求已 cook 的数据
		// 两个都满足 = 编辑器 / 开发版（不是打包版）
		if (Target.bCompileAgainstEngine && !Target.bBuildRequiresCookedData)
		{
			// 动态加载 DerivedDataCache（DDC 派生数据缓存）
			DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
			// 动态加载 Virtualization（虚拟化）
			DynamicallyLoadedModuleNames.AddRange(new string[] { "Virtualization" });
		}

		// 如果当前平台属于桌面组（Win / Mac / Linux）。
		// UnrealPlatformGroup.Desktop 定义在 UEBuildTarget.cs:555。
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop))
        {
			// 在桌面平台上启用更多崩溃信息。
			// 打开更多崩溃上下文（崩溃时收集更多信息）
			PublicDefinitions.Add("WITH_ADDITIONAL_CRASH_CONTEXTS=1");
		}

		// 如果当前平台属于 Windows 组，且不是发布包
		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			// Visual Studio 的并发可视化扩展
			// 链 VS 并发可视化扩展（看多线程执行时间线）
			PrivateDependencyModuleNames.Add("ConcurrencyVisualizer");
			
			// Super Luminal。
			// 链 Superluminal 性能分析器（第三方 CPU profiler）
			PrivateDependencyModuleNames.Add("SuperLuminal");
			
			// Visual Studio 性能分析器
			// 链 VS 性能分析器
			PrivateDependencyModuleNames.Add("VSPerfExternalProfiler");
		}

		// 如果当前平台属于 Apple 组，或者是 Android。
		// UnrealPlatformGroup.Apple 定义在 UEBuildTarget.cs:530。
		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Apple) ||
			Target.Platform == UnrealTargetPlatform.Android)
		{
			// 定义宏：游戏线程跑在外部线程上。
			// 为什么：这些平台的主线程不是游戏线程，游戏线程跑在单独的外部线程。
			PublicDefinitions.Add("IS_RUNNING_GAMETHREAD_ON_EXTERNAL_THREAD=1");
		}

		// 设置一个宏，让 FApp::GetBuildTargetType() 能识别客户端目标
		// 如果目标类型是 Client（客户端，无服务器代码的游戏包）。
		// TargetRules.TargetType.Client 定义在 TargetRules.cs:35。
		if (Target.Type == TargetRules.TargetType.Client)
		{
			// 定义宏：是客户端目标
			PrivateDefinitions.Add("IS_CLIENT_TARGET=1");
		}
		// 否则
		else
		{
			// 定义宏：不是客户端目标
			PrivateDefinitions.Add("IS_CLIENT_TARGET=0");
		}

		// 设置用于包含 / 排除 Iris 对 UObject 改动的定义。注意：只需要这个定义，因为我们完全不依赖 Iris。
		// 如果启用 Iris 网络复制系统。
		// Target.bUseIris 是 ReadOnlyTargetRules.cs:181 的属性。
		if (Target.bUseIris == true)
		{
			// 定义宏：启用 Iris
			PublicDefinitions.Add("UE_WITH_IRIS=1");
		}
		// 否则
		else
		{
			// 定义宏：不启用 Iris
			PublicDefinitions.Add("UE_WITH_IRIS=0");
		}
		
		// 如果这次编的是 Win64，且不是发布包
		if (Target.Platform == UnrealTargetPlatform.Win64
			&& Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			// 支持平台事件
			PublicDefinitions.Add("PLATFORM_SUPPORTS_PLATFORM_EVENTS=1");
			// 支持 Win32 虚拟内存 Hook 追踪
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_VIRTUAL_MEMORY_HOOKS=1");
			// 支持 Win32 模块诊断追踪
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_MODULE_DIAGNOSTICS=1");
			// 支持 Win32 调用栈追踪
			PublicDefinitions.Add("PLATFORM_SUPPORTS_TRACE_WIN32_CALLSTACK=1");
			// 内存追踪可用
			PublicDefinitions.Add("UE_MEMORY_TRACE_AVAILABLE=1");
		}

		// 临时性的东西。
		// 定义宏 PLATFORM_SUPPORTS_BINARYCONFIG：调本文件自己的 SupportsBinaryConfig 方法（第 316 行）。
		// 三元运算符：返回 true 就是 "1"，false 就是 "0"。
		PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));

		// 定义宏 WITH_MALLOC_STOMP：调本文件自己的 bWithMallocStomp 属性（第 319 行）。
		PublicDefinitions.Add("WITH_MALLOC_STOMP=" + (bWithMallocStomp ? "1" : "0"));

		// 定义宏：是否允许 LTCG（Link Time Code Generation，链接时代码生成）。
		// Target.bAllowLTCG 是 ReadOnlyTargetRules.cs:465 的属性。
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_LTCG=" + (Target.bAllowLTCG ? "1" : "0"));
		// 定义宏：是否 PGO 优化（Profile Guided Optimization，按采样数据优化）。
		// Target.bPGOOptimize 是 ReadOnlyTargetRules.cs:473 的属性。
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG=" + (Target.bPGOOptimize ? "1" : "0"));
		// 定义宏：是否 PGO 采样（生成 profile 数据）。
		// Target.bPGOProfile 是 ReadOnlyTargetRules.cs:471 的属性。
		PrivateDefinitions.Add("PLATFORM_COMPILER_OPTIMIZATION_PG_PROFILING=" + (Target.bPGOProfile ? "1" : "0"));

		// 关掉旧数学常量宏名（如 PI / SMALL_NUMBER 的旧写法）。
		// 设为 0 = 不定义旧宏名，强制用新写法。
		PrivateDefinitions.Add("UE_DEFINE_LEGACY_MATH_CONSTANT_MACRO_NAMES=0");

		// 私有依赖：AutoRTFM（与第 34 行重复！引擎原样，不许去重）
		PrivateDependencyModuleNames.Add("AutoRTFM");

		// 不安全类型转换（如 double→float、int64→int32）当错误处理。
		// CppCompileWarningSettings 是 ModuleRules.cs:982 的属性（警告设置总对象）。
		// WarningLevel.Error 定义在 ModuleRules.cs:38。
		CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Error;

		// IWYU（Include What You Use，用到什么包含什么）保持原样，不强制。
		// IWYUSupport 是 ModuleRules.cs:1222 的属性。
		// IWYUSupport.KeepAsIs 定义在 ModuleRules.cs:80。
		IWYUSupport = IWYUSupport.KeepAsIs;

		// 如果启用插桩（API Hook）。
		// Target.WindowsPlatform.bEnableInstrumentation 是 UEBuildWindows.cs:985 的属性，
		// 真值在 UEBuildWindows.cs:508，默认 false。
		if (Target.WindowsPlatform.bEnableInstrumentation)
		{
			// 链 Detours（微软的 API Hook 库）
			PrivateDependencyModuleNames.Add("Detours");
		}

		// 定义宏 UE_MERGED_MODULES：是否合并模块。
		// Target.bMergeModules 是 ReadOnlyTargetRules.cs:572 的属性。
		PublicDefinitions.Add("UE_MERGED_MODULES=" + (Target.bMergeModules ? "1" : "0"));
	}
```

---

## 七、两个自定义成员（第 314~329 行）

```csharp
	// 决定当前平台能否使用二进制配置
	// protected = 子类可访问；virtual = 子类可重写。
	// 参数 Target 是 ReadOnlyTargetRules（和构造函数参数同名，但这里是方法参数）。
	protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
	{
		// 恒为真。
		// 后面 // 是被注释掉的旧条件（属于代码，不翻译）。
		// 注意：// 前没有空格，引擎原文如此，照抄。
		return true;// Target.Platform != UnrealTargetPlatform.Android;
	}

	// 决定当前平台能否使用内存分配器校验（也就是 MallocStomp）。
	// 运行时校验必须通过 '-stompmalloc' 命令行参数开启。
	// protected virtual bool = 子类可重写的只读属性。
	protected virtual bool bWithMallocStomp
	{
		// 表达式体写法，等价于 get { return ...; }
		get => 
			// 条件 1：不是发布包
			Target.Configuration != UnrealTargetConfiguration.Shipping &&
			// 条件 2：平台是 Mac / Linux / LinuxArm64 / Win64 之一
			// UnrealTargetPlatform.Linux 定义在 UEBuildTarget.cs:272
			// UnrealTargetPlatform.LinuxArm64 定义在 UEBuildTarget.cs:277
			(Target.Platform == UnrealTargetPlatform.Mac ||
			 Target.Platform == UnrealTargetPlatform.Linux ||
			 Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
			 Target.Platform == UnrealTargetPlatform.Win64);
	}
}
```

---

## 八、速查：这段代码在干什么（一句话版）

| 段 | 行 | 一句话 |
|---|---|---|
| 骨架 | 1~15 | 声明 `Core` 类继承 `ModuleRules`，设 PCH 和 Unity 合批 |
| Android 块 | 17~31 | 编 Android 时挂 Google SDK / Vulkan 头 / 两个诊断工具 / 注册 xml |
| 依赖清单 | 33~65 | 加 5 个私有依赖 + 3 个公开依赖 + 两组头路径模块 + 编辑器专属 |
| Win64 | 67~71 | 用 libpas 分配器 |
| Windows 组 | 73~115 | zlib / IntelTBB / mimalloc / dbghelp / PIX / 内存追踪 |
| Mac | 116~133 | 3 个第三方库 + 8 个框架 + mimalloc + 触控板驱动 |
| IOS 组 | 134~179 | 13 个框架 + PLCrashReporter + iAD + 符号导出 + 内存追踪 |
| Android 组 | 180~200 | 3 个第三方库 + 5 个内存追踪宏 + Scudo |
| Unix 组 | 201~220 | zlib / jemalloc / dl / mimalloc + 3 个内存追踪宏 |
| 通用开关 | 222~311 | ICU / DDC / 崩溃上下文 / 3 个 profiler / 客户端宏 / Iris / LTCG / PGO / 警告 / IWYU / 合并 |
| 自定义成员 | 314~329 | `SupportsBinaryConfig()` 恒真 + `bWithMallocStomp` 只读属性 |

---

## 九、C# 语法小抄（本文件用到的）

| 语法 | 含义 | 例子 |
|---|---|---|
| `using X;` | 引入命名空间 | `using System.IO;` |
| `class A : B` | A 继承 B | `public class Core : ModuleRules` |
| `: base(Target)` | 调基类构造函数 | `public Core(...) : base(Target)` |
| `List<T>.Add(x)` | 往列表加一项 | `PrivateDependencyModuleNames.Add("zlib")` |
| `List<T>.AddRange(arr)` | 往列表批量加 | `PublicFrameworks.AddRange(new string[] {...})` |
| `a == b` / `a != b` | 等于 / 不等于 | `Target.Platform == UnrealTargetPlatform.Android` |
| `a && b` | 逻辑与 | `Target.bCompileAgainstEngine && !Target.bBuildRequiresCookedData` |
| `!a` | 逻辑非 | `!Target.bBuildRequiresCookedData` |
| `a \|\| b` | 逻辑或 | `Target.Platform == X \|\| Target.Platform == Y` |
| `cond ? a : b` | 三元运算符 | `Target.bCompileICU ? "1" : "0"` |
| `if (...) { } else { }` | 条件分支 | — |
| `string x = ...;` | 声明字符串变量 | `string ModulePath = ...` |
| `bool x = ...;` | 声明布尔变量 | `bool bSupportAdvertising = ...` |
| `protected virtual bool X` | 子类可重写的只读属性 | `protected virtual bool bWithMallocStomp` |
| `get => expr;` | 表达式体 getter | `get => Target.Configuration != ...` |

---

## 待确认

1. 这份超详细注释版看懂了吗？看不懂哪一行告诉我，我再拆。
2. 交付版（干净可照抄）在 `01_Core.Build.cs_完整代码.md`。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`（73 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs`（748 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\UEBuildTarget.cs`（6352 行）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\TargetRules.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\UEBuildWindows.cs`
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\System\Utils.cs`