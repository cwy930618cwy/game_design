// 鐗堟潈鎵€鏈?Epic Games, Inc. 淇濈暀鎵€鏈夋潈鍒┿€?
using UnrealBuildTool;
using System;
using System.IO;

// Core 妯″潡澹版槑锛氬憡璇?UBT 杩欎釜妯″潡鎬庝箞缂栵紙渚濊禆鍝簺妯″潡銆佸畾涔夊摢浜涘畯銆佸悇骞冲彴棰濆鎸備粈涔堬級
public class Core : ModuleRules
{
	// 鏋勯€犲嚱鏁帮細鎶婂綋鍓嶇紪璇戠洰鏍囷紙骞冲彴 / 閰嶇疆 / 鐩爣绫诲瀷锛変氦缁欏熀绫?ModuleRules
	public Core(ReadOnlyTargetRules Target) : base(Target)
	{
		NumIncludedBytesPerUnityCPPOverride = 491520; // 鐢?UBT 鐨?ProfileUnitySizes 妯″紡娴嬪嚭鏉ョ殑鏈€浣?Unity 鍚堟壒澶у皬

		PrivatePCHHeaderFile = "Private/CorePrivatePCH.h";

		SharedPCHHeaderFile = "Public/CoreSharedPCH.h";

		PrivateDependencyModuleNames.Add("BuildSettings");
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("GoogleGameSDK");
			
			PrivateIncludePathModuleNames.Add("VulkanRHI"); 				// AndroidPlatformMisc 闇€瑕?
			if (Target.Configuration != UnrealTargetConfiguration.Shipping)
			{
				PrivateDependencyModuleNames.Add("libGPUCounters");        // ARM CPU 涓?ARM Mali GPU 鐨勬€ц兘璁℃暟鍣?				PrivateDependencyModuleNames.Add("heapprofd");      // 鎶婅嚜瀹氫箟鍒嗛厤鍣ㄦ毚闇茬粰 Google 鐨勫唴瀛樺垎鏋愬櫒
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

			// 涓嶅笇鏈涢潤鎬佸垎鏋愬櫒鍘昏窇绗笁鏂逛唬鐮?			if (Target.StaticAnalyzer == StaticAnalyzer.None) 
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

			// 涓哄唴宓?Dll 瀵煎嚭 Core 鐨勭鍙?			ModuleSymbolVisibility = ModuleRules.SymbolVisibility.VisibileForDll;
			
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

				// 鏀寔瀵?libc.so 鐨?malloc 鍋氬唴瀛樿拷韪?				PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Build", "Android", "Prebuilt", "ScudoMemoryTrace"));
				PrivateDefinitions.Add("UE_MEMORY_TRACE_ANDROID_ENABLE_SCUDO_TRACING_SUPPORT=1");
			}
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target,
				"zlib",
				"jemalloc"
				);

			// Core 鐢ㄥ埌 dlopen()
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
		PublicDefinitions.Add("UE_ENABLE_ICU=" + (Target.bCompileICU ? "1" : "0")); // 鍦ㄤ唬鐮佸簱閲屽惎鐢?绂佺敤锛?1/=0锛塈CU銆傛敞鎰忥細杩欎釜寮€鍏虫槸闆嗘垚 ICU 鏈熼棿鐢ㄧ殑锛屼箣鍚庝細绉婚櫎銆?
		// 濡傛灉姝ｅ湪鍜屽紩鎿庝竴璧风紪璇戯紝灏卞姞涓?Core 瀵瑰紩鎿庣殑渚濊禆
		if (Target.bCompileAgainstEngine && !Target.bBuildRequiresCookedData)
		{
			DynamicallyLoadedModuleNames.AddRange(new string[] { "DerivedDataCache" });
			DynamicallyLoadedModuleNames.AddRange(new string[] { "Virtualization" });
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Desktop))
        {
			// 鍦ㄦ闈㈠钩鍙颁笂鍚敤鏇村宕╂簝淇℃伅銆?			PublicDefinitions.Add("WITH_ADDITIONAL_CRASH_CONTEXTS=1");
		}

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) && Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			// Visual Studio 鐨勫苟鍙戝彲瑙嗗寲鎵╁睍
			PrivateDependencyModuleNames.Add("ConcurrencyVisualizer");
			
			// Super Luminal銆?			PrivateDependencyModuleNames.Add("SuperLuminal");
			
			// Visual Studio 鎬ц兘鍒嗘瀽鍣?			PrivateDependencyModuleNames.Add("VSPerfExternalProfiler");
		}

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Apple) ||
			Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDefinitions.Add("IS_RUNNING_GAMETHREAD_ON_EXTERNAL_THREAD=1");
		}

		// 璁剧疆涓€涓畯锛岃 FApp::GetBuildTargetType() 鑳借瘑鍒鎴风鐩爣
		if (Target.Type == TargetRules.TargetType.Client)
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=1");
		}
		else
		{
			PrivateDefinitions.Add("IS_CLIENT_TARGET=0");
		}

		// 璁剧疆鐢ㄤ簬鍖呭惈 / 鎺掗櫎 Iris 瀵?UObject 鏀瑰姩鐨勫畾涔夈€傛敞鎰忥細鍙渶瑕佽繖涓畾涔夛紝鍥犱负鎴戜滑瀹屽叏涓嶄緷璧?Iris銆?		if (Target.bUseIris == true)
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

		// 涓存椂鎬х殑涓滆タ銆?		PrivateDefinitions.Add("PLATFORM_SUPPORTS_BINARYCONFIG=" + (SupportsBinaryConfig(Target) ? "1" : "0"));

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

	// 鍐冲畾褰撳墠骞冲彴鑳藉惁浣跨敤浜岃繘鍒堕厤缃?	protected virtual bool SupportsBinaryConfig(ReadOnlyTargetRules Target)
	{
		return true;// Target.Platform != UnrealTargetPlatform.Android;
	}

	// 鍐冲畾褰撳墠骞冲彴鑳藉惁浣跨敤鍐呭瓨鍒嗛厤鍣ㄦ牎楠岋紙涔熷氨鏄?MallocStomp锛夈€?	// 杩愯鏃舵牎楠屽繀椤婚€氳繃 '-stompmalloc' 鍛戒护琛屽弬鏁板紑鍚€?	protected virtual bool bWithMallocStomp
	{
		get => 
			Target.Configuration != UnrealTargetConfiguration.Shipping &&
			(Target.Platform == UnrealTargetPlatform.Mac ||
			 Target.Platform == UnrealTargetPlatform.Linux ||
			 Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
			 Target.Platform == UnrealTargetPlatform.Win64);
	}
}
