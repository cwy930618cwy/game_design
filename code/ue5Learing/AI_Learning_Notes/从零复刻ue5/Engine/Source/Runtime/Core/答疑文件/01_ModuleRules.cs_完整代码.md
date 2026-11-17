# 01 — `ModuleRules.cs` —— 完整代码（注释已中文化）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（**2119 行 / 76049 字节**）
> 你新建文件：`ue_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（答疑用，非当前主线）
> **✅ 本节可独立编译**——UBT 自己的 C# 源码。

---

## 一、这个文件解决什么问题（为什么）

`Core.Build.cs` 里 `public class Core : ModuleRules` 的那个 `ModuleRules`，就是本文件。它是**所有 `*.Build.cs` 的基类**：你写的每个模块单子，本质都是继承它、在构造函数里往它的清单里加名字。

> 🔑 **一句话**：`ModuleRules` = **模块施工单的模板类**，2119 行里 90% 是「栏目定义」（属性）+ 一小撮「填表助手」（`AddEngineThirdPartyPrivateStaticDependencies` 这类方法）。

---

## 二、完整代码（注释已中文化，你照这个抄）

```csharp
// 版权所有 Epic Games, Inc. 保留所有权利。

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Reflection;
using EpicGames.Core;
using Microsoft.Extensions.Logging;
using UnrealBuildBase;

namespace UnrealBuildTool
{
	/// <summary>
	/// 控制某条特定警告如何被对待
	/// </summary>
	public enum WarningLevel
	{
		/// <summary>
		/// 使用默认行为
		/// </summary>
		Default,

		/// <summary>
		/// 不显示诊断信息
		/// </summary>
		Off,

		/// <summary>
		/// 正常输出警告
		/// </summary>
		Warning,

		/// <summary>
		/// 把警告当错误输出
		/// </summary>
		Error,
	}

	/// <summary>
	/// 描述 Verse 代码的来源与可见性
	/// </summary>
	public enum VerseScope
	{
		/// <summary>
		/// 由 Epic 创建，只有公开定义对公开用户可见
		/// </summary>
		PublicAPI,

		/// <summary>
		/// 由 Epic 创建，对公开用户完全隐藏
		/// </summary>
		InternalAPI,

		/// <summary>
		/// 由公开用户创建
		/// </summary>
		PublicUser,

		/// <summary>
		/// 由 Epic 内部用户创建
		/// </summary>
		InternalUser
	}

	/// <summary>
	/// 一个模块对 include-what-you-use（用到什么就包含什么）的支持程度
	/// </summary>
	public enum IWYUSupport
	{
		/// <summary>
		/// None 表示代码甚至无法编译。IWYU 需要完全跳过这个模块
		/// </summary>
		None,

		/// <summary>
		/// 模块可以被 IWYU 修改，但我们希望它保持原样、手动处理变更
		/// </summary>
		KeepAsIs,

		/// <summary>
		/// 模块会被解析和处理。这意味着从外部看它的 include 会被剥离，
		/// 尽管文件本身不会被修改。可用于推迟某个模块的 IWYU 工作。
		/// 在传递性 include 方面，这个模块从外部看被视为已修改。
		/// </summary>
		KeepAsIsForNow,

		/// <summary>
		/// 与 KeepAsIsForNow 相同，但允许 IWYU 更新私有头文件和 cpp 文件。
		/// </summary>
		KeepPublicAsIsForNow,

		/// <summary>
		/// 完整的 IWYU 支持。以 -Mode=IWYU 运行时，这个模块会在需要时被修改
		/// </summary>
		Full,
	}

	/// <summary>
	/// ModuleRules 是一个数据结构，包含定义一个模块所需的规则
	/// </summary>
	public class ModuleRules
	{
		/// <summary>
		/// 模块类型
		/// </summary>
		public enum ModuleType
		{
			/// <summary>
			/// C++
			/// </summary>
			CPlusPlus,

			/// <summary>
			/// 外部（第三方）
			/// </summary>
			External,
		}

		/// <summary>
		/// 覆盖 UHTModuleType 的设置，以使用另一组 PKG_ 标志。
		/// 不能在插件上设置，因为该值已在 '.uplugin' 文件中设置
		/// </summary>
		public enum PackageOverrideType
		{
			/// <summary>
			/// 不覆盖此模块的包类型
			/// </summary>
			None,

			/// <summary>
			/// 在此模块上设置 PKG_EditorOnly 标志
			/// </summary>
			EditorOnly,

			/// <summary>
			/// 在此模块上设置 PKG_Developer
			/// </summary>
			EngineDeveloper,

			/// <summary>
			/// 在此模块上设置 PKG_Developer
			/// </summary>
			GameDeveloper,

			/// <summary>
			/// 在此模块上设置 PKG_UncookedOnly 标志
			/// </summary>
			EngineUncookedOnly,

			/// <summary>
			/// 作为游戏在此模块上设置 PKG_UncookedOnly 标志
			/// </summary>
			GameUncookedOnly
		}

		/// <summary>
		/// 代码优化设置
		/// </summary>
		public enum CodeOptimization
		{
			/// <summary>
			/// 尽可能永不优化代码。
			/// </summary>
			Never,

			/// <summary>
			/// 只在非调试构建中优化代码（Debug 中不优化）。
			/// </summary>
			InNonDebugBuilds,

			/// <summary>
			/// 只在 Shipping 构建中优化代码（Debug、DebugGame、Development 中不优化）
			/// </summary>
			InShippingBuildsOnly,

			/// <summary>
			/// 尽可能始终优化代码。
			/// </summary>
			Always,

			/// <summary>
			/// 默认：游戏模块用 'InNonDebugBuilds'，其他用 'Always'。
			/// </summary>
			Default,
		}

		/// <summary>
		/// 此模块使用哪种类型的 PCH。
		/// </summary>
		public enum PCHUsageMode
		{
			/// <summary>
			/// 默认：引擎模块使用共享 PCH，游戏模块不使用
			/// </summary>
			Default,

			/// <summary>
			/// 从不使用任何 PCH。
			/// </summary>
			NoPCHs,

			/// <summary>
			/// 从不使用共享 PCH。合适时总是为此模块生成独立的 PCH
			/// </summary>
			NoSharedPCHs,

			/// <summary>
			/// 可以使用共享 PCH！
			/// </summary>
			UseSharedPCHs,

			/// <summary>
			/// 如果没有通过 PrivatePCHHeaderFile 设置显式私有 PCH，则可以使用共享 PCH。
			/// 无论哪种情况，源文件都不手动包含模块 PCH，而应包含匹配的头文件。
			/// </summary>
			UseExplicitOrSharedPCHs,
		}

		/// <summary>
		/// 此模块应为哪些类型的目标预编译
		/// </summary>
		public enum PrecompileTargetsType
		{
			/// <summary>
			/// 从不预编译此模块。
			/// </summary>
			None,

			/// <summary>
			/// 从模块所在目录推断。Engine/Source/Runtime 下的引擎模块为游戏编译，
			/// Engine/Source/Editor 下的为编辑器编译，以此类推……
			/// </summary>
			Default,

			/// <summary>
			/// 任意游戏目标。
			/// </summary>
			Game,

			/// <summary>
			/// 任意编辑器目标。
			/// </summary>
			Editor,

			/// <summary>
			/// 任意目标。
			/// </summary>
			Any,
		}

		/// <summary>
		/// 特殊情况下控制此模块中符号的可见性
		/// </summary>
		public enum SymbolVisibility
		{
			/// <summary>
			/// 标准可见性规则
			/// </summary>
			Default,

			/// <summary>
			/// 确保此模块中的符号在 Dll 构建中可见
			/// </summary>
			VisibileForDll,
		}

		/// <summary>
		/// 启用时，此模块中可以使用 unity 文件的文件类型。
		/// </summary>
		[Flags]
		public enum UnityFileType
		{
			/// <summary>
			/// 无文件
			/// </summary>
			None,

			/// <summary>
			/// C++
			/// </summary>
			Cpp,

			/// <summary>
			/// C
			/// </summary>
			C,

			/// <summary>
			/// C++（备用扩展名）
			/// </summary>
			CC,

			/// <summary>
			/// C++（备用扩展名）
			/// </summary>
			CXX,

			/// <summary>
			/// Objective-C
			/// </summary>
			M,

			/// <summary>
			/// Objective-C++
			/// </summary>
			MM,
		}

		/// <summary>
		/// 目标在运行时需要的文件信息，必须随目标一起移动。
		/// </summary>
		[Serializable]
		public class RuntimeDependency
		{
			/// <summary>
			/// 应该被暂存（stage）的文件。应使用 $(EngineDir) 和 $(ProjectDir) 变量作为根，
			/// 以便目标可以迁移到不同机器。
			/// </summary>
			public string Path { get; init; }

			/// <summary>
			/// 此文件的初始位置。构建时会被复制到 Path，准备暂存。
			/// </summary>
			public string? SourcePath { get; init; }

			/// <summary>
			/// 如何暂存此文件。
			/// </summary>
			public StagedFileType Type { get; init; }

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="inPath">运行时依赖的路径</param>
			/// <param name="inType">如何暂存给定路径</param>
			public RuntimeDependency(string inPath, StagedFileType inType = StagedFileType.NonUFS)
			{
				Path = inPath;
				Type = inType;
			}

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="inPath">运行时依赖的路径</param>
			/// <param name="inSourcePath">工作树中该文件的源路径</param>
			/// <param name="inType">如何暂存给定路径</param>
			public RuntimeDependency(string inPath, string inSourcePath, StagedFileType inType = StagedFileType.NonUFS)
			{
				Path = inPath;
				SourcePath = inSourcePath;
				Type = inType;
			}
		}

		/// <summary>
		/// 运行时依赖列表，附带添加新项的便捷方法
		/// </summary>
		[Serializable]
		public class RuntimeDependencyList
		{
			/// <summary>
			/// 内部的运行时依赖列表
			/// </summary>
			readonly List<RuntimeDependency> _inner = new();

			/// <summary>
			/// 内部运行时依赖列表的只读访问
			/// </summary>
			internal IReadOnlyList<RuntimeDependency> Inner => _inner.AsReadOnly();

			/// <summary>
			/// 默认构造函数
			/// </summary>
			public RuntimeDependencyList()
			{
			}

			/// <summary>
			/// 向列表添加一个运行时依赖
			/// </summary>
			/// <param name="inPath">运行时依赖的路径。可包含通配符。</param>
			public void Add(string inPath)
			{
				_inner.Add(new RuntimeDependency(inPath));
			}

			/// <summary>
			/// 向列表添加一个运行时依赖
			/// </summary>
			/// <param name="inPath">运行时依赖的路径。可包含通配符。</param>
			/// <param name="inType">如何暂存此文件</param>
			public void Add(string inPath, StagedFileType inType)
			{
				_inner.Add(new RuntimeDependency(inPath, inType));
			}

			/// <summary>
			/// 向列表添加一个运行时依赖
			/// </summary>
			/// <param name="inPath">运行时依赖的路径。可包含通配符。</param>
			/// <param name="inSourcePath">要添加为依赖的文件的源路径。可包含通配符。</param>
			/// <param name="inType">如何暂存此文件</param>
			public void Add(string inPath, string inSourcePath, StagedFileType inType = StagedFileType.NonUFS)
			{
				_inner.Add(new RuntimeDependency(inPath, inSourcePath, inType));
			}
		}

		/// <summary>
		/// 构建回执（receipt）属性列表，附带添加新项的便捷方法
		/// </summary>
		[Serializable]
		public class ReceiptPropertyList
		{
			/// <summary>
			/// 内部的回执属性列表
			/// </summary>
			readonly List<ReceiptProperty> _inner = new();

			/// <summary>
			/// 内部回执属性列表的只读访问
			/// </summary>
			internal IReadOnlyList<ReceiptProperty> Inner => _inner.AsReadOnly();

			/// <summary>
			/// 默认构造函数
			/// </summary>
			public ReceiptPropertyList()
			{
			}

			/// <summary>
			/// 向列表添加一个回执属性
			/// </summary>
			/// <param name="name">属性名</param>
			/// <param name="value">属性值</param>
			public void Add(string name, string value)
			{
				_inner.Add(new ReceiptProperty(name, value));
			}

			/// <summary>
			/// 从列表中移除回执属性
			/// </summary>
			/// <param name="match">匹配谓词</param>
			/// <returns>被移除的项数</returns>
			public int RemoveAll(Predicate<ReceiptProperty> match)
			{
				return _inner.RemoveAll(match);
			}
		}

		/// <summary>
		/// 存储 IOS 或 MacOS 上某个 framework 的信息
		/// </summary>
		public class Framework
		{
			/// <summary>
			/// framework 的名称
			/// </summary>
			internal string Name { get; init; }

			/// <summary>
			/// 指定包含它的 zip 文件路径，或 framework 在磁盘上的位置
			/// </summary>
			internal string Path { get; init; }

			/// <summary>
			/// 需要一起复制的捆绑资源
			/// </summary>
			internal string? CopyBundledAssets { get; init; } = null;

			/// <summary>
			/// 如何处理 framework 的链接与复制
			/// </summary>
			public enum FrameworkMode
			{
				/// <summary>
				/// 把这个 framework 传给链接器
				/// </summary>
				Link,

				/// <summary>
				/// 把这个 framework 复制进最终的 .app
				/// </summary>
				Copy,

				/// <summary>
				/// 既链接进可执行文件，也复制进 .app
				/// </summary>
				LinkAndCopy,
			}

			/// <summary>
			/// 链接和生成 .app 期间如何处理这个 framework
			/// </summary>
			internal FrameworkMode Mode { get; init; }

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="name">framework 的名称</param>
			/// <param name="path">包含 framework 的 zip 文件路径，或磁盘上的 framework</param>
			/// <param name="copyBundledAssets">需要一起复制的捆绑资源</param>
			/// <param name="bCopyFramework">把 framework 复制到目标的 Framework 目录</param>
			public Framework(string name, string path, string? copyBundledAssets = null, bool bCopyFramework = false)
				: this(name, path, bCopyFramework ? FrameworkMode.LinkAndCopy : FrameworkMode.Link, copyBundledAssets)
			{
			}

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="name">framework 的名称</param>
			/// <param name="path">包含 framework 的 zip 文件路径，或磁盘上的 framework</param>
			/// <param name="mode">链接和生成 .app 期间如何处理这个 framework</param>
			/// <param name="copyBundledAssets">需要一起复制的捆绑资源</param>
			public Framework(string name, string path, FrameworkMode mode, string? copyBundledAssets = null)
			{
				Name = name;
				Path = path;
				Mode = mode;
				CopyBundledAssets = copyBundledAssets;
			}

			/// <summary>
			/// 指定该文件是否为 zip 文件
			/// </summary>
			public bool IsZipFile() => Path.EndsWith(".zip", StringComparison.OrdinalIgnoreCase);
		}

		/// <summary>
		/// 需要复制进 Mac 或 iOS 应用包的资源
		/// </summary>
		public class BundleResource
		{
			/// <summary>
			/// 资源路径
			/// </summary>
			public string? ResourcePath { get; init; } = null;

			/// <summary>
			/// 在包内容中的子目录
			/// </summary>
			public string? BundleContentsSubdir { get; init; } = null;

			/// <summary>
			/// 是否输出日志
			/// </summary>
			public bool bShouldLog { get; init; } = true;

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="resourcePath">资源路径</param>
			/// <param name="bundleContentsSubdir">在包内容中的子目录</param>
			/// <param name="bShouldLog">是否输出日志</param>
			public BundleResource(string resourcePath, string bundleContentsSubdir = "Resources", bool bShouldLog = true)
			{
				ResourcePath = resourcePath;
				BundleContentsSubdir = bundleContentsSubdir;
				this.bShouldLog = bShouldLog;
			}
		}

		/// <summary>
		/// Windows 类型库（TLB/OLB 文件）的信息，需要生成对应头文件。
		/// </summary>
		public class TypeLibrary
		{
			/// <summary>
			/// 类型库的名称
			/// </summary>
			public string FileName { get; init; }

			/// <summary>
			/// #import 指令的附加属性
			/// </summary>
			public string Attributes { get; init; }

			/// <summary>
			/// 输出头文件的名称
			/// </summary>
			public string Header { get; init; }

			/// <summary>
			/// 输出 include 文件的名称
			/// </summary>
			public string? Include { get; init; }

			/// <summary>
			/// 构造函数
			/// </summary>
			/// <param name="fileName">类型库的名称。遵循 MSVC #import 指令中 filename 参数的约定。</param>
			/// <param name="attributes">import 指令的附加属性</param>
			/// <param name="header">输出头文件的名称</param>
			/// <param name="include">输出 include 文件的名称</param>
			public TypeLibrary(string fileName, string attributes, string header, string? include = null)
			{
				FileName = fileName;
				Attributes = attributes;
				Header = header;
				Include = include;
			}
		}

		/// <summary>
		/// 此模块的名称
		/// </summary>
		public string Name { get; internal set; }

		/// <summary>
		/// 包含此模块的文件
		/// </summary>
		internal FileReference File { get; set; }

		/// <summary>
		/// 包含此模块的目录
		/// </summary>
		internal DirectoryReference Directory { get; set; }

		/// <summary>
		/// 包含 Build.cs 文件的目录，或平台扩展子类目录
		/// </summary>
		protected DirectoryReference PlatformDirectory => DirectoriesForModuleSubClasses[GetType()]; // 只要能被查询到，这里就必然存在

		/// <summary>
		/// 如果规则是平台扩展子类则返回 true
		/// </summary>
		protected bool bIsPlatformExtension => (Directory != PlatformDirectory);

		/// <summary>
		/// 对此模块有贡献的附加目录（通常位于 UnrealBuildTool.EnginePlatformExtensionsDirectory）。
		/// 该字典用于跟踪模块子类
		/// </summary>
		internal Dictionary<Type, DirectoryReference> DirectoriesForModuleSubClasses;

		/// <summary>
		/// 对此模块有贡献、但不基于子类的附加目录（NotForLicensees 等）
		/// </summary>
		private List<DirectoryReference> AdditionalModuleDirectories = new();

		/// <summary>
		/// 搜索模块时使用的规则程序集
		/// </summary>
		internal RulesAssembly RulesAssembly;

		/// <summary>
		/// 包含此模块的插件
		/// </summary>
		internal PluginInfo? Plugin;

		/// <summary>
		/// 如果某个插件包含此模块则为 true
		/// </summary>
		public bool IsPlugin => Plugin != null;

		/// <summary>
		/// 此实例的规则上下文
		/// </summary>
		internal ModuleRulesContext Context { get; set; }

		/// <summary>
		/// 此模块所属目标的规则
		/// </summary>
		public ReadOnlyTargetRules Target { get; init; }

		/// <summary>
		/// 模块类型
		/// </summary>
		public ModuleType Type { get; set; } = ModuleType.CPlusPlus;

		/// <summary>
		/// 目标日志记录器的访问器
		/// </summary>
		public ILogger Logger => Target.Logger;

		/// <summary>
		/// 覆盖后的模块类型，会设置不同的包标志。
		/// 不能用于属于插件的模块，因为那已在 `.uplugin` 文件中设置。
		/// </summary>
		public PackageOverrideType OverridePackageType
		{
			get => overridePackageType ?? PackageOverrideType.None;
			set => overridePackageType = !IsPlugin
					? value
					: throw new CompilationResultException(CompilationResult.RulesError, KnownLogEvents.RulesAssembly, "Module '{ModuleName}' cannot override package type because it is part of a plugin!", Name);
		}

		private PackageOverrideType? overridePackageType;

		/// <summary>
		/// 如果此模块指定了覆盖类型则返回 true
		/// </summary>
		public bool HasPackageOverride => OverridePackageType != PackageOverrideType.None;

		/// <summary>
		/// 构建 DLL 时把此模块放入 Binaries/PLATFORM 的哪个子目录。
		/// 只应由通过搜索找到的模块使用，例如 TargetPlatform 或 ShaderFormat 模块。
		/// 如果不用 FindModules 追踪它们，这些模块将找不到。
		/// </summary>
		public string BinariesSubFolder { get; set; } = String.Empty;

		private CodeOptimization? OptimizeCodeOverride;

		/// <summary>
		/// 此模块的代码应在何时优化。
		/// </summary>
		public CodeOptimization OptimizeCode
		{
			get
			{
				if (bCodeCoverage)
				{
					return CodeOptimization.Never;
				}

				if (OptimizeCodeOverride.HasValue)
				{
					return OptimizeCodeOverride.Value;
				}

				bool? shouldOptimizeCode = null;
				if (Target.EnableOptimizeCodeForModules?.Contains(Name) ?? false)
				{
					shouldOptimizeCode = true;
				}

				if (Target.DisableOptimizeCodeForModules?.Contains(Name) ?? false)
				{
					shouldOptimizeCode = false;
				}

				if (!shouldOptimizeCode.HasValue)
				{
					return CodeOptimization.Default;
				}

				return shouldOptimizeCode.Value ? CodeOptimization.Always : CodeOptimization.Never;
			}
			set => OptimizeCodeOverride = value;
		}

		private OptimizationMode? OptimizationLevelOverride = null;

		/// <summary>
		/// 允许针对速度或代码体积微调优化等级。这需要私有 PCH（或 NoPCHs，不推荐）
		/// </summary>
		public OptimizationMode OptimizationLevel
		{
			get
			{
				if (Target.OptimizeForSizeModules?.Contains(Name) ?? false)
				{
					return OptimizationMode.Size;
				}
				if (Target.OptimizeForSizeAndSpeedModules?.Contains(Name) ?? false)
				{
					return OptimizationMode.SizeAndSpeed;
				}
				if (OptimizationLevelOverride.HasValue)
				{
					return OptimizationLevelOverride.Value;
				}
				else
				{
					return Target.OptimizationLevel;
				}
			}
			set => OptimizationLevelOverride = value;
		}

		/// <summary>
		/// 允许覆盖此模块的浮点语义。这需要私有 PCH（或 NoPCHs，不推荐）
		/// </summary>
		public FPSemanticsMode FPSemantics
		{
			get => FPSemanticsPrivate ?? Target.FPSemantics;
			set => FPSemanticsPrivate = value;
		}

		private FPSemanticsMode? FPSemanticsPrivate = null;

		/// <summary>
		/// 此模块中每个源文件都应强制包含的头文件。
		/// </summary>
		public List<string> ForceIncludeFiles { get; } = new List<string>();

		/// <summary>
		/// 此模块的显式私有 PCH。意味着此模块不会使用共享 PCH。
		/// </summary>
		public string? PrivatePCHHeaderFile { get; set; }

		/// <summary>
		/// 此模块提供的共享 PCH 的头文件名。必须是公开 C++ 头文件的有效相对路径。
		/// 只应为被大量其他 C++ 模块包含的头文件设置此项。
		/// </summary>
		public string? SharedPCHHeaderFile { get; set; }

		/// <summary>
		/// 为此模块的中间产物指定中间目录和文件的备用名称。遇到路径长度限制时有用。
		/// </summary>
		public string? ShortName { get; set; } = null;

		/// <summary>
		/// 此模块的预编译头使用方式
		/// </summary>
		public PCHUsageMode PCHUsage
		{
			get
			{
				if (PCHUsagePrivate.HasValue)
				{
					// 使用覆盖值
					return PCHUsagePrivate.Value;
				}
				else if (Target.bEnableCppModules)
				{
					return PCHUsageMode.NoPCHs;
				}
				else if (Target.bIWYU || DefaultBuildSettings >= BuildSettingsVersion.V2)
				{
					// 使用共享或显式 PCH，并启用 IWYU
					return PCHUsageMode.UseExplicitOrSharedPCHs;
				}
				else if (Plugin != null)
				{
					// 较老的插件默认使用共享 PCH，因为它们通常不够大，不值得拥有自己的 PCH。
					return PCHUsageMode.UseSharedPCHs;
				}
				else
				{
					// 较老的游戏模块默认不启用共享 PCH，因为游戏通常有自己的大型预编译头。
					return PCHUsageMode.NoSharedPCHs;
				}
			}
			set => PCHUsagePrivate = value;
		}
		private PCHUsageMode? PCHUsagePrivate;

		/// <summary>
		/// 此模块是否应被视为引擎模块（例如使用引擎定义、PCH，在 DebugGame 配置下以优化方式编译等）。
		/// 初始值根据创建它的规则程序集决定。
		/// </summary>
		public bool bTreatAsEngineModule { get; set; }

		/// <summary>
		/// 如果这里包含某个平台，且项目已覆盖该平台的默认 SDK 版本，则此模块将作为项目模块编译，而不是共享引擎模块
		/// </summary>
		public List<UnrealTargetPlatform> SDKVersionRelevantPlatforms { get; } = new();

		/// <summary>
		/// 对不正确的 UE_LOG 格式字符串发出编译错误。
		/// </summary>
		public bool bValidateFormatStrings
		{
			get => bValidateFormatStringsPrivate ?? (bTreatAsEngineModule || Target.bValidateFormatStrings);
			set => bValidateFormatStringsPrivate = value;
		}
		private bool? bValidateFormatStringsPrivate;

		/// <summary>
		/// 对非引擎模块使用内部 API 时发出废弃警告或错误
		/// </summary>
		public bool bValidateInternalApi
		{
			get => bValidateInternalApiPrivate ?? !bTreatAsEngineModule;
			set => bValidateInternalApiPrivate = value;
		}
		private bool? bValidateInternalApiPrivate;

		/// <summary>
		/// 对非引擎模块使用实验性 API 时发出废弃警告或错误
		/// </summary>
		public bool bValidateExperimentalApi
		{
			get => bValidateExperimentalApiPrivate ?? !bTreatAsEngineModule;
			set => bValidateExperimentalApiPrivate = value;
		}
		private bool? bValidateExperimentalApiPrivate;

		/// <summary>
		/// 默认使用哪个引擎版本的构建设置。
		/// </summary>
		public BuildSettingsVersion DefaultBuildSettings
		{
			get => DefaultBuildSettingsPrivate ?? Target.DefaultBuildSettings;
			set => DefaultBuildSettingsPrivate = value;
		}
		private BuildSettingsVersion? DefaultBuildSettingsPrivate;

		/// <summary>
		/// 编译此模块时使用哪个版本的 include 顺序。可通过命令行 -ForceIncludeOrder 或模块规则覆盖。
		/// </summary>
		public EngineIncludeOrderVersion IncludeOrderVersion
		{
			get
			{
				if (Target.ForcedIncludeOrder != null)
				{
					return Target.ForcedIncludeOrder.Value;
				}
				if (bTreatAsEngineModule)
				{
					return EngineIncludeOrderVersion.Latest;
				}
				return IncludeOrderVersionPrivate ?? Target.IncludeOrderVersion;
			}
			set => IncludeOrderVersionPrivate = value;
		}
		private EngineIncludeOrderVersion? IncludeOrderVersionPrivate;

		/// <summary>
		/// 使用运行时类型信息（RTTI）
		/// </summary>
		public bool bUseRTTI { get; set; }

		/// <summary>
		/// 是否指示 MSVC 移除未被引用的 COMDAT 函数和数据。
		/// </summary>
		/// <seealso href="https://learn.microsoft.com/en-us/cpp/build/reference/zc-inline-remove-unreferenced-comdat">zc-inline-remove-unreferenced-comdat</seealso>
		public bool bVcRemoveUnreferencedComdat { get; set; } = true;

		/// <summary>
		/// 启用代码覆盖率编译/链接支持。
		/// </summary>
		public bool bCodeCoverage { get; set; }

		/// <summary>
		/// 已废弃：在支持它的平台上，指示编译器在使用 SSE 或 AVX 内建函数的地方生成 AVX 指令。
		/// 注意：启用此项会改变 PC 平台的最低配置要求，生成的程序在不支持 AVX 的机器上会崩溃。
		/// </summary>
		[Obsolete("bUseAVX is obsolete and will be removed in UE5.4, please replace with MinCpuArchX64")]
		public bool bUseAVX
		{
			get => MinCpuArchX64 >= MinimumCpuArchitectureX64.AVX;
			set => MinCpuArchX64 = value ? MinimumCpuArchitectureX64.AVX : MinimumCpuArchitectureX64.None;
		}

		/// <summary>
		/// 在支持它的 x64 平台上，指示编译器在使用 SSE 或 AVX 内建函数的地方生成 AVX 指令。
		/// 注意：启用此项会改变 PC 平台的最低配置要求，生成的程序在不支持 AVX 的机器上会崩溃。
		/// </summary>
		public MinimumCpuArchitectureX64? MinCpuArchX64 { get; set; } = null;

		/// <summary>
		/// 启用缓冲区安全检查。通常应启用，因为它能防止严重的安全风险。
		/// </summary>
		public bool bEnableBufferSecurityChecks { get; set; } = true;

		/// <summary>
		/// 启用异常处理
		/// </summary>
		public bool bEnableExceptions { get; set; }

		/// <summary>
		/// 启用 Objective-C 异常处理
		/// </summary>
		public bool bEnableObjCExceptions { get; set; }

		/// <summary>
		/// 启用 Objective-C 自动引用计数（ARC）
		/// 如果设为 true，就不应为此模块使用共享 PCH。引擎短期内不会大量使用 ARC
		/// 不这样做会导致编译错误，因为共享 PCH 编译时使用的标志与使用方不同
		/// </summary>
		public bool bEnableObjCAutomaticReferenceCounting { get; set; }

		/// <summary>
		/// 如何处理确定性警告（实验性）。
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.DeterministicWarningLevel")]
		public WarningLevel DeterministicWarningLevel
		{
			get => CppCompileWarningSettings.DeterministicWarningLevel;
			set => CppCompileWarningSettings.DeterministicWarningLevel = value;
		}

		/// <summary>
		/// 模块使用的设置对象。
		/// </summary>
		public CppCompileWarnings CppCompileWarningSettings { get; init; }

		/// <summary>
		/// 如何处理影子变量警告
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.ShadowVariableWarningLevel")]
		public WarningLevel ShadowVariableWarningLevel
		{
			get => CppCompileWarningSettings.ShadowVariableWarningLevel;
			set => CppCompileWarningSettings.ShadowVariableWarningLevel = value;
		}

		/// <summary>
		/// 是否把所有警告都当错误。UE 已把大部分警告当错误，但禁用了一些（例如废弃警告）。
		/// </summary>
		public bool bWarningsAsErrors { get; set; }

		/// <summary>
		/// 如何处理不安全的隐式类型转换警告（例如 double->float 或 int64->int32）
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.UnsafeTypeCastWarningLevel")]
		public WarningLevel UnsafeTypeCastWarningLevel
		{
			get => CppCompileWarningSettings.UnsafeTypeCastWarningLevel;
			set => CppCompileWarningSettings.UnsafeTypeCastWarningLevel = value;
		}


		/// <summary>
		/// 指示把条件表达式中的未定义标识符当作什么警告/错误级别。
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.UndefinedIdentifierWarningLevel")]
		public WarningLevel UndefinedIdentifierWarningLevel
		{
			get => CppCompileWarningSettings.UndefinedIdentifierWarningLevel;
			set => CppCompileWarningSettings.UndefinedIdentifierWarningLevel = value;
		}

		/// <summary>
		/// 对在 #if 表达式中使用未定义标识符的情况启用警告
		/// </summary>
		[Obsolete("Deprecated in UE5.5 - Replace with ModuleRules.UndefinedIdentifierWarningLevel")]
		public bool bEnableUndefinedIdentifierWarnings { get; set; } = true;

		/// <summary>
		/// 如何处理一般的模块 include 路径校验消息
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.ModuleIncludePathWarningLevel")]
		public WarningLevel ModuleIncludePathWarningLevel
		{
			get => CppCompileWarningSettings.ModuleIncludePathWarningLevel;
			set => CppCompileWarningSettings.ModuleIncludePathWarningLevel = value;
		}

		/// <summary>
		/// 如何处理私有模块 include 路径校验消息（模块添加了暴露私有头文件的 include 路径）
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.ModuleIncludePrivateWarningLevel")]
		public WarningLevel ModuleIncludePrivateWarningLevel
		{
			get => CppCompileWarningSettings.ModuleIncludePrivateWarningLevel;
			set => CppCompileWarningSettings.ModuleIncludePrivateWarningLevel = value;
		}

		/// <summary>
		/// 如何处理不必要的模块子目录 include 路径校验消息
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.ModuleIncludeSubdirectoryWarningLevel")]
		public WarningLevel ModuleIncludeSubdirectoryWarningLevel
		{
			get => CppCompileWarningSettings.ModuleIncludeSubdirectoryWarningLevel;
			set => CppCompileWarningSettings.ModuleIncludeSubdirectoryWarningLevel = value;
		}

		/// <summary>
		/// 禁用所有静态分析——clang、msvc、pvs-studio。
		/// </summary>
		public bool bDisableStaticAnalysis { get; set; }

		/// <summary>
		/// 使用 EspXEngine 插件启用额外的分析器扩展警告。仅支持 MSVC。
		/// 参见 https://learn.microsoft.com/en-us/cpp/code-quality/using-the-cpp-core-guidelines-checkers
		/// 默认会新增大量警告。如果启用此项，建议同时使用 StaticAnalyzerRulesets。
		/// </summary>
		public bool bStaticAnalyzerExtensions { get; set; }

		/// <summary>
		/// 用于过滤警告的静态分析规则集。仅支持 MSVC。
		/// 参见 https://learn.microsoft.com/en-us/cpp/code-quality/using-rule-sets-to-specify-the-cpp-rules-to-run
		/// </summary>
		public HashSet<FileReference> StaticAnalyzerRulesets = new();

		/// <summary>
		/// 应启用的静态分析检查器（替代默认值）。仅支持 Clang。
		/// 完整列表参见 https://clang.llvm.org/docs/analyzer/checkers.html，或运行：
		///    'clang -Xclang -analyzer-checker-help'
		/// 或：
		///    'clang -Xclang -analyzer-checker-help-alpha'
		/// 查看实验性检查器列表。
		/// </summary>
		public HashSet<string> StaticAnalyzerCheckers { get; } = new();

		/// <summary>
		/// 应禁用的静态分析默认检查器。若 StaticAnalyzerCheckers 已填充则此项无效。仅支持 Clang。
		/// 完整列表参见 https://clang.llvm.org/docs/analyzer/checkers.html，或运行：
		///    'clang -Xclang -analyzer-checker-help'
		/// 或：
		///    'clang -Xclang -analyzer-checker-help-alpha'
		/// 查看实验性检查器列表。
		/// </summary>
		public HashSet<string> StaticAnalyzerDisabledCheckers { get; } = new() {
			// 进行中，等所有警告修完后移除被禁用的检查器
			"core.BitwiseShift",
			"core.CallAndMessage",
			"core.DivideZero",
			"core.NonNullParamChecker",
			"core.NullDereference",
			"core.UndefinedBinaryOperatorResult",
			"core.uninitialized.Assign",
			"core.uninitialized.Branch",
			"core.uninitialized.UndefReturn",
			"cplusplus.Move",
			"cplusplus.NewDelete",
			"cplusplus.NewDeleteLeaks",
			"unix.cstring.NullArg",

			// 需要评估
			"deadcode.DeadStores",
		};

		/// <summary>
		/// 应启用的静态分析非默认检查器。若 StaticAnalyzerCheckers 已填充则此项无效。仅支持 Clang。
		/// 完整列表参见 https://clang.llvm.org/docs/analyzer/checkers.html，或运行：
		///    'clang -Xclang -analyzer-checker-help'
		/// 或：
		///    'clang -Xclang -analyzer-checker-help-alpha'
		/// 查看实验性检查器列表。
		/// </summary>
		public HashSet<string> StaticAnalyzerAdditionalCheckers { get; } = new();

		/// <summary>
		/// 应禁用的 PVS Studio 分析错误。
		/// </summary>
		public HashSet<string> StaticAnalyzerPVSDisabledErrors { get; } = new();

		private bool? bUseUnityOverride;
		/// <summary>
		/// 如果启用了 unity 构建，可用此项覆盖此特定模块是否使用 Unity 构建。
		/// 通过 BuildConfiguration 中的按模块配置设置。
		/// </summary>
		public bool bUseUnity
		{
			set => bUseUnityOverride = value;
			get => bUseUnityOverride ?? Target.DisableUnityBuildForModules?.Contains(Name) != true;
		}

		/// <summary>
		/// 是否合并模块的 unity 文件与生成的 unity 文件以加快编译。
		/// </summary>
		public bool bMergeUnityFiles { get; set; } = true;

		/// <summary>
		/// 启用时会被放入 unity 文件的源文件类型。
		/// </summary>
		public UnityFileType UnityFileTypes { get; set;  } = UnityFileType.Cpp;

		/// <summary>
		/// 此模块中源文件数量达到多少时才启用 unity 构建。如果设为 -1 以外的值，
		/// 将覆盖由 MinGameModuleSourceFilesForUnityBuild 控制的默认设置
		/// </summary>
		public int MinSourceFilesForUnityBuildOverride { get; set; } = 0;

		/// <summary>
		/// 非零时覆盖 BuildConfiguration.MinFilesUsingPrecompiledHeader。
		/// </summary>
		public int MinFilesUsingPrecompiledHeaderOverride { get; set; } = 0;

		/// <summary>
		/// 非零时覆盖 Target.NumIncludedBytesPerUnityCPP。
		/// </summary>
		public int NumIncludedBytesPerUnityCPPOverride { get; set; } = 0;

		/// <summary>
		/// 辅助函数：获取每个 unity cpp 文件包含的字节数
		/// </summary>
		public int GetNumIncludedBytesPerUnityCPP() => (NumIncludedBytesPerUnityCPPOverride != 0 && !Target.bDisableModuleNumIncludedBytesPerUnityCPPOverride) ? NumIncludedBytesPerUnityCPPOverride : Target.NumIncludedBytesPerUnityCPP;

		/// <summary>
		/// 模块使用了 #import，因此用 SN-DBS 编译时必须在本地构建
		/// </summary>
		public bool bBuildLocallyWithSNDBS { get; set; }

		/// <summary>
		/// 当存在可以内联到匹配的手写 cpp 文件中的 .gen.cpp 文件时启用警告
		/// </summary>
		[Obsolete("Deprecated in UE5.6 - Replace with ModuleRules.CppCompileWarningSettings.NonInlinedGenCppWarningLevel")]
		public bool bEnableNonInlinedGenCppWarnings { get; set; }

		/// <summary>
		/// 此模块的再分发覆盖标志。
		/// </summary>
		public bool? IsRedistributableOverride { get; set; } = null;

		/// <summary>
		/// 此模块的输出是否可以公开分发，即使它依赖的代码/模块不可以（即 CarefullyRedist、NotForLicensees、NoRedist）。
		/// 当你计划发布二进制但不发布源码时应使用此项。
		/// </summary>
		public bool bLegalToDistributeObjectCode { get; set; }

		/// <summary>
		/// 编译此二进制时允许引用的文件夹列表，且不传播受限文件夹名称
		/// </summary>
		public List<string> AllowedRestrictedFolders = new();

		/// <summary>
		/// 受限文件夹引用的别名集合
		/// </summary>
		public Dictionary<string, string> AliasRestrictedFolders = new();

		/// <summary>
		/// 当 PCHUsage 设为 ExplicitOrSharedPCH 时强制执行「用到什么就包含什么」规则；
		/// 使用单体头文件（Engine.h、UnrealEd.h 等）时发出警告，并检查源文件是否先包含自己的匹配头文件。
		/// </summary>
		[Obsolete("Deprecated in UE5.2 - Use IWYUSupport instead.")]
		public bool bEnforceIWYU
		{
			set
			{
				if (!value)
				{
					IWYUSupport = IWYUSupport.None;
				}
			}
		}

		/// <summary>
		/// 允许「用到什么就包含什么」在运行时修改源代码。bEnforceIWYU 必须为 true 此项才有意义。
		/// 
		/// </summary>
		public IWYUSupport IWYUSupport { get; set; } = IWYUSupport.Full;

		/// <summary>
		/// 是否向模块添加所有默认 include 路径（例如 Source/Classes 文件夹、Source/Public 下的子文件夹）。
		/// </summary>
		public bool bAddDefaultIncludePaths { get; set; } = true;

		/// <summary>
		/// 是否忽略模块中悬空的（即未解析的外部）符号
		/// </summary>
		public bool bIgnoreUnresolvedSymbols { get; set; }

		/// <summary>
		/// 此模块是否应被预编译。默认为目标的 bPrecompile 标志。清除此标志可阻止模块被预编译。
		/// </summary>
		public bool bPrecompile { get; set; }

		/// <summary>
		/// 此模块是否应使用预编译数据。从已安装程序集创建的模块始终为 true。
		/// </summary>
		public bool bUsePrecompiled { get; set; }

		/// <summary>
		/// 此模块是否可以使用 PLATFORM_XXXX 风格的宏，其中 XXXX 是机密平台名称。
		/// 这用于确保引擎或其他共享代码不会在 #if PLATFORM_XXXX 块内泄露机密信息。
		/// 但被授权方的游戏代码可能希望允许它们。
		/// </summary>
		public bool bAllowConfidentialPlatformDefines { get; set; }

		/// <summary>
		/// 仅在启用 AutoRTFMCompiler 时禁用此模块的 AutoRTFM 插桩
		/// </summary>
		public bool bDisableAutoRTFMInstrumentation { get; set; }
		
		/// <summary>
		/// 如果为 true，USTRUCTS 的自动生成函数始终导出
		/// </summary>
		public bool bAlwaysExportStructs = true;

		/// <summary>
		/// 如果为 true，UENUMS 的自动生成函数始终导出
		/// </summary>
		public bool bAlwaysExportEnums = true;
		
		/// <summary>
		/// 模块名称列表（无需路径），其头文件是本模块公开头文件需要访问的，但我们不需要「导入」或链接它们。
		/// </summary>
		public List<string> PublicIncludePathModuleNames = new();

		/// <summary>
		/// 公开依赖模块名称列表（无需路径）（自动处理公有/私有 include）。这些是我们的公开源文件所需的模块。
		/// </summary>
		public List<string> PublicDependencyModuleNames = new();

		/// <summary>
		/// 模块名称列表（无需路径），其头文件是本模块私有代码文件需要访问的，但我们不需要「导入」或链接它们。
		/// </summary>
		public List<string> PrivateIncludePathModuleNames = new();

		/// <summary>
		/// 私有依赖模块名称列表。这些是我们的私有代码依赖、但公开 include 文件不依赖的模块。
		/// </summary>
		public List<string> PrivateDependencyModuleNames = new();

		/// <summary>
		/// 仅为兼容旧代码保留，新代码不应使用。应被视为循环引用的模块依赖列表。
		/// 这些模块必须已添加到公有或私有依赖模块列表中。
		/// </summary>
		public List<string> CircularlyReferencedDependentModules = new();

		/// <summary>
		/// 系统/库 include 路径列表——通常用于外部（第三方）模块。
		/// 这些是公开稳定的头文件目录，解析头文件依赖时不会被检查。
		/// </summary>
		public List<string> PublicSystemIncludePaths = new();

		/// <summary>
		/// （当前不需要此设置，因为我们会从 'Public' 文件夹发现所有文件）暴露给其他模块的 include 文件路径列表
		/// </summary>
		public List<string> PublicIncludePaths = new();

		/// <summary>
		/// （当前不需要此设置，因为我们会从 'Internal' 文件夹发现所有文件）暴露给其他内部模块的 include 文件路径列表
		/// </summary>
		public List<string> InternalIncludePaths = new();

		/// <summary>
		/// 此模块内部 include 文件的所有路径列表，不暴露给其他模块
		/// （至少包含一个指向 'Private' 路径的项，若想避免相对路径可添加更多）
		/// </summary>
		public List<string> PrivateIncludePaths = new();

		/// <summary>
		/// 系统库路径列表（.lib 文件所在目录）——外部（第三方）模块请改用 PublicAdditionalLibaries
		/// </summary>
		public List<string> PublicSystemLibraryPaths = new();

		/// <summary>
		/// 运行时库搜索路径列表（例如 .so 文件）
		/// </summary>
		public List<string> PrivateRuntimeLibraryPaths = new();

		/// <summary>
		/// 运行时库搜索路径列表（例如 .so 文件）
		/// </summary>
		public List<string> PublicRuntimeLibraryPaths = new();

		/// <summary>
		/// 附加库列表（.lib 文件名，含扩展名）——通常用于外部（第三方）模块
		/// </summary>
		public List<string> PublicAdditionalLibraries = new();

		/// <summary>
		/// 暴露给其他模块的附加调试可视化器列表（.natvis、.natstepfilter、.natjmc）——通常用于外部（第三方）模块
		/// </summary>
		public List<string> PublicDebugVisualizerPaths = new();

		/// <summary>
		/// 按架构划分的、链接时应忽略的依赖列表（在为多架构构建、而某个库只需用于一种架构时有用），由工具链决定是否使用
		/// </summary>
		public Dictionary<string, List<UnrealArch>> DependenciesToSkipPerArchitecture = new();

		/// <summary>
		/// 返回传入模块名所在的目录。
		/// </summary>
		/// <param name="moduleName">模块名称</param>
		/// <returns>模块所在目录</returns>
		public string GetModuleDirectory(string moduleName)
		{
			FileReference? moduleFileReference = RulesAssembly.GetModuleFileName(moduleName)
				?? throw new CompilationResultException(CompilationResult.RulesError, KnownLogEvents.RulesAssembly, "Could not find a module named '{ModuleName}'.", moduleName);
			return moduleFileReference.Directory.FullName;
		}

		/// <summary>
		/// 附加的预构建库列表（.lib 文件名，含扩展名）——通常用于仍会被构建、但通过 TargetRules.PreBuildSteps 或 TargetRules.PreBuildTargets 构建的附加目标
		/// </summary>
		public List<string> PublicPreBuildLibraries = new();

		/// <summary>
		/// 要使用的系统库列表——通常按名称引用，然后通过系统路径查找。如果需要引用 .lib 文件，请改用 PublicAdditionalLibraries
		/// </summary>
		public List<string> PublicSystemLibraries = new();

		/// <summary>
		/// XCode framework 列表（iOS 和 MacOS）
		/// </summary>
		public List<string> PublicFrameworks = new();

		/// <summary>
		/// 弱链接 framework 列表（用于操作系统版本过渡）
		/// </summary>
		public List<string> PublicWeakFrameworks = new();

		/// <summary>
		/// 附加 framework 列表——通常用于 Mac 和 iOS 上的外部（第三方）模块
		/// </summary>
		public List<Framework> PublicAdditionalFrameworks = new();

		/// <summary>
		/// 应复制到 Mac 或 iOS 应用包的附加资源列表
		/// </summary>
		public List<BundleResource> AdditionalBundleResources = new();

		/// <summary>
		/// 需要为其生成头文件的类型库列表（仅 Windows）
		/// </summary>
		public List<TypeLibrary> TypeLibraries = new();

		/// <summary>
		/// 延迟加载 DLL 列表——通常用于外部（第三方）模块
		/// </summary>
		public List<string> PublicDelayLoadDLLs = new();

		/// <summary>
		/// 此模块的私有编译器定义
		/// </summary>
		public List<string> PrivateDefinitions = new();

		/// <summary>
		/// 此模块的公开编译器定义
		/// </summary>
		public List<string> PublicDefinitions = new();

		/// <summary>
		/// 追加（或创建）
		/// </summary>
		/// <param name="definition">定义名</param>
		/// <param name="text">要追加的文本</param>
		public void AppendStringToPublicDefinition(string definition, string text)
		{
			string withEquals = definition + "=";
			for (int index = 0; index < PublicDefinitions.Count; index++)
			{
				if (PublicDefinitions[index].StartsWith(withEquals, StringComparison.Ordinal))
				{
					PublicDefinitions[index] = PublicDefinitions[index] + text;
					return;
				}
			}

			// 走到这里说明需要新建一项
			PublicDefinitions.Add(definition + "=" + text);
		}

		/// <summary>
		/// 此模块在运行时可能需要的附加模块
		/// </summary>
		public List<string> DynamicallyLoadedModuleNames = new();

		/// <summary>
		/// 此模块在运行时依赖的文件列表。这些文件会随目标一起暂存。
		/// </summary>
		public RuntimeDependencyList RuntimeDependencies = new();

		/// <summary>
		/// 要添加到构建回执的附加属性列表
		/// </summary>
		public ReceiptPropertyList AdditionalPropertiesForReceipt = new();

		/// <summary>
		/// 此模块应为哪些目标预编译
		/// </summary>
		public PrecompileTargetsType PrecompileForTargets { get; set; } = PrecompileTargetsType.Default;

		/// <summary>
		/// 修改后会使 makefile 失效的外部文件。相对路径相对于 .build.cs 文件解析。
		/// </summary>
		public List<string> ExternalDependencies = new();

		/// <summary>
		/// 修改后会使 makefile 失效的子类规则文件。
		/// </summary>
		public List<string>? SubclassRules;

		/// <summary>
		/// 此模块是否要求实现 IMPLEMENT_MODULE 宏。大多数 UE 模块都需要，
		/// 因为我们用 IMPLEMENT_MODULE 宏做其他全局重载（例如把 operator new/delete 转发给 GMalloc）。
		/// </summary>
		public bool? bRequiresImplementModule { get; set; }

		/// <summary>
		/// 模块可以提供 ExtraRootPath。它会成为 rootpaths 表的一部分，
		/// 当模块使用正常环境之外的路径时需要。
		/// 相同路径的所有额外根应使用相同 Id，否则本可命中的缓存会变成未命中。
		/// 确保路径尽可能长。即不要用 "Program Files" 作为根路径，而应用 "Program Files/MyProgram"
		/// </summary>
		public (string id, string path)? ExtraRootPath { get; set; }

		/// <summary>
		/// 如果此模块有关联的 Verse 代码，这里是它的 Verse 根路径
		/// </summary>
		public string? VersePath { get; private set; }

		/// <summary>
		/// 此模块 Source/Verse 文件夹中 Verse 代码的可见性
		/// </summary>
		public VerseScope VerseScope { get; private set; } = VerseScope.PublicUser;

		/// <summary>
		/// 此模块是否相对于其他模块 'Public' 文件夹的根来限定被包含的头文件。
		/// 这会减少必须传给编译器的搜索路径数量，提升性能并缩短编译器命令行长度。
		/// </summary>
		public bool bLegacyPublicIncludePaths
		{
			set => bLegacyPublicIncludePathsPrivate = value;
			get => bLegacyPublicIncludePathsPrivate ?? ((DefaultBuildSettings < BuildSettingsVersion.V2) && Target.bLegacyPublicIncludePaths);
		}
		private bool? bLegacyPublicIncludePathsPrivate;

		/// <summary>
		/// 此模块是否相对于父目录来限定被包含的头文件。
		/// 这会减少必须传给编译器的搜索路径数量，提升性能并缩短编译器命令行长度。
		/// </summary>
		public bool bLegacyParentIncludePaths
		{
			set => bLegacyParentIncludePathsPrivate = value;
			get => bLegacyParentIncludePathsPrivate ?? ((DefaultBuildSettings < BuildSettingsVersion.V3) && Target.bLegacyParentIncludePaths);
		}
		private bool? bLegacyParentIncludePathsPrivate;

		/// <summary>
		/// 是否按允许列表校验循环依赖
		/// 循环模块依赖会导致构建变慢。强烈不建议禁用此项。
		/// 引擎模块会忽略此项，它们始终按允许列表校验。
		/// </summary>
		public bool bValidateCircularDependencies { get; set; } = true;

		/// <summary>
		/// 编译此模块使用哪个 C++ 标准
		/// </summary>
		public CppStandardVersion? CppStandard { get; set; }

		/// <summary>
		/// 编译此模块使用哪个 C 标准
		/// </summary>
		public CStandardVersion? CStandard { get; set; }

		/// <summary>
		/// 子目录名称与被调用以生成头文件的函数列表。
		/// 子目录名会追加到生成代码目录后，形成新的目录，头文件在其中生成。
		/// </summary>
		public List<(string, Action<ILogger, DirectoryReference>)> GenerateHeaderFuncs = new();

		/// <summary>
		/// 控制符号可见性
		/// </summary>
		public SymbolVisibility ModuleSymbolVisibility { get; set; } = SymbolVisibility.Default;

		/// <summary>
		/// 当前宿主平台的 AutoSDK 目录
		/// </summary>
		public string? AutoSdkDirectory => UEBuildPlatformSDK.TryGetHostPlatformAutoSDKDir(out DirectoryReference? autoSdkDir) ? autoSdkDir.FullName : null;

		/// <summary>
		/// 当前引擎目录
		/// </summary>
		public string EngineDirectory => Unreal.EngineDirectory.FullName;

		/// <summary>
		/// 包含此插件的目录属性。用于添加第三方依赖的路径。
		/// </summary>
		public string PluginDirectory => Plugin == null
					? throw new CompilationResultException(CompilationResult.RulesError, KnownLogEvents.RulesAssembly, "Module '{ModuleName}' does not belong to a plugin; PluginDirectory property is invalid.", Name)
					: Plugin.Directory.FullName;

		/// <summary>
		/// 包含此模块的目录属性。用于添加第三方依赖的路径。
		/// </summary>
		[SuppressMessage("Naming", "CA1721:Property names should not match get methods", Justification = "GetModuleDirectory() is to get the path to other modules")]
		public string ModuleDirectory => Directory.FullName;

		/// <summary>
		/// 包含此模块或平台扩展子类的目录属性。用于添加第三方依赖的路径。
		/// </summary>
		protected string PlatformModuleDirectory => PlatformDirectory.FullName;

		/// <summary>
		/// PlatformModuleDirectory 下的平台名称——对于 PlatformExtension，
		/// 我们不用平台子目录（因为它已在平台目录中），所以这里返回 '.'
		/// </summary>
		protected string PlatformSubdirectoryName => bIsPlatformExtension ? "." : Target.Platform.ToString();

		/// <summary>
		/// 返回模块的低级测试目录 "Tests"。
		/// </summary>
		public string TestsDirectory => IsTestModule ? Directory.FullName : Path.Combine(Directory.FullName, "Tests");

		/// <summary>
		/// 如果设置，此模块会用这个输入头文件在 C++ 与 Swift 之间生成桥接头文件
		/// </summary>
		public string? SwiftInteropHeader = null;

		/// <summary>
		/// 用作当前模块所有 clang 编译单元的 LLVM FPass 插件的文件列表。
		///    'clang -fpass-plugin=[FPassPlugin]'
		/// </summary>
		public List<string> FPassPlugins = new();

#nullable disable
		/// <summary>
		/// 构造函数。为了向后兼容（无参构造函数正在被淘汰），
		/// 原本应在此处完成的初始化改由 RulesAssembly.CreateModulRules 完成。
		/// </summary>
		/// <param name="target">构建此目标的规则</param>
		public ModuleRules(ReadOnlyTargetRules target)
		{
			Target = target;
			CppCompileWarningSettings = new CppCompileWarnings(this, Logger);
		}
#nullable restore

		/// <summary>
		/// 把给定的引擎第三方模块添加为静态私有依赖
		///	静态链接到此模块，意味着它们使用其他模块的导出符号
		///	私有，意味着给出本模块 include 路径时，不会暴露被包含模块的 include 路径
		///	注意：没有 AddThirdPartyPublicStaticDependencies 函数。
		/// </summary>
		/// <param name="target">此模块所属的目标</param>
		/// <param name="moduleNames">要添加的模块名称</param>
		public void AddEngineThirdPartyPrivateStaticDependencies(ReadOnlyTargetRules target, params string[] moduleNames)
		{
			if (!bUsePrecompiled || target.LinkType == TargetLinkType.Monolithic)
			{
				PrivateDependencyModuleNames.AddRange(moduleNames);
			}
		}

		/// <summary>
		/// 把给定的引擎第三方模块添加为动态私有依赖
		///	动态链接到此模块，意味着它们不使用其他模块的导出符号
		///	私有，意味着给出本模块 include 路径时，不会暴露被包含模块的 include 路径
		///	注意：没有 AddThirdPartyPublicDynamicDependencies 函数。
		/// </summary>
		/// <param name="target">正在构建的目标的规则</param>
		/// <param name="moduleNames">要添加的模块名称</param>
		public void AddEngineThirdPartyPrivateDynamicDependencies(ReadOnlyTargetRules target, params string[] moduleNames)
		{
			if (!bUsePrecompiled || target.LinkType == TargetLinkType.Monolithic)
			{
				PrivateIncludePathModuleNames.AddRange(moduleNames);
				DynamicallyLoadedModuleNames.AddRange(moduleNames);
			}
		}

		/// <summary>
		/// 确保为使用 VerseVM 编译代码设置好必要的链接器依赖
		/// </summary>
		public void SetupVerse()
		{
			if (Target.bUseVerseVM)
			{
				PublicDependencyModuleNames.AddRange(
					new string[]
					{
						"libpas",
						"CorePreciseFP",
					}
				);
			}
		}

		/// <summary>
		/// 为此模块设置 Verse
		/// </summary>
		public void SetupVerse(string versePath, VerseScope verseScope)
		{
			VersePath = versePath;
			VerseScope = verseScope;

			SetupVerse();
		}

		/// <summary>
		/// 为此模块设置 Mesh Editor 支持（基于 UEBuildConfiguration 中的设置）
		/// </summary>
		public void EnableMeshEditorSupport(ReadOnlyTargetRules target) => PublicDefinitions.Add($"ENABLE_MESH_EDITOR={(target.bEnableMeshEditor ? 1 : 0)}");

		/// <summary>
		/// 为此模块设置 GameplayDebugger 支持
		/// </summary>
		public void SetupGameplayDebuggerSupport(ReadOnlyTargetRules target, bool bAddAsPublicDependency = false)
		{
			if (target.bUseGameplayDebugger || target.bUseGameplayDebuggerCore)
			{
				PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER_CORE=1");
				PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=" + (target.bUseGameplayDebugger ? 1 : 0));
				if (target.bUseGameplayDebugger || (target.bUseGameplayDebuggerCore && target.Configuration != UnrealTargetConfiguration.Shipping))
				{
					PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER_MENU=1");
				}
				else
				{
					PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER_MENU=0");
				}

				if (bAddAsPublicDependency)
				{
					PublicDependencyModuleNames.Add("GameplayDebugger");
					if (target.Type == TargetType.Editor)
					{
						PublicDependencyModuleNames.Add("GameplayDebuggerEditor");
					}
				}
				else
				{
					PrivateDependencyModuleNames.Add("GameplayDebugger");
					if (target.Type == TargetType.Editor)
					{
						PrivateDependencyModuleNames.Add("GameplayDebuggerEditor");
					}
				}
			}
			else
			{
				PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER_CORE=0");
				PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
				PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER_MENU=0");
			}
		}

		/// <summary>
		/// 为此模块设置 Iris 支持（基于 UEBuildConfiguration 中的设置）
		/// </summary>
		public void SetupIrisSupport(ReadOnlyTargetRules target, bool bAddAsPublicDependency = false)
		{
			if (target.bUseIris == true)
			{
				PublicDefinitions.Add("UE_WITH_IRIS=1");
				PrivateDefinitions.Add("UE_NET_HAS_IRIS_FASTARRAY_BINDING=1");

				if (bAddAsPublicDependency)
				{
					PublicDependencyModuleNames.Add("IrisCore");
				}
				else
				{
					PrivateDependencyModuleNames.Add("IrisCore");
				}
			}
			else
			{
				PublicDefinitions.Add("UE_WITH_IRIS=0");

				// 不编译 Iris 时，我们有一个供 UHT 依赖的 Iris 桩模块
				if (bAddAsPublicDependency)
				{
					PublicDependencyModuleNames.Add("IrisStub");
				}
				else
				{
					PrivateDependencyModuleNames.Add("IrisStub");
				}
			}
		}

		/// <summary>
		/// 如果给定目标支持 Chaos 可视化调试器则返回 true
		/// </summary>
		protected bool IsChaosVisualDebuggerSupported(ReadOnlyTargetRules target)
		{
			return target.bCompileChaosVisualDebuggerSupport && target.Configuration != UnrealTargetConfiguration.Shipping && target.bEnableTrace;
		}

		/// <summary>
		/// 为此模块设置 Chaos 可视化调试器支持（记录将在 Chaos Visual Debugger 工具中可视化的调试数据所必需）
		/// </summary>
		public void SetupModuleChaosVisualDebuggerSupport(ReadOnlyTargetRules target)
		{
			//TODO: 如果显式启用了 CVD 支持但禁用了 UE Trace，发出某种警告
			if (IsChaosVisualDebuggerSupported(target))
			{
				PublicDependencyModuleNames.Add("ChaosVDRuntime");

				PublicDefinitions.Add("WITH_CHAOS_VISUAL_DEBUGGER=1");
			}
			else
			{
				PublicDefinitions.Add("WITH_CHAOS_VISUAL_DEBUGGER=0");
			}
		}

		/// <summary>
		/// 为此模块设置物理支持（基于 UEBuildConfiguration 中的设置）
		/// </summary>
		public void SetupModulePhysicsSupport(ReadOnlyTargetRules target)
		{
			PublicIncludePathModuleNames.AddRange(
					new string[] {
					"Chaos",
					}
				);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"PhysicsCore",
					"ChaosCore",
					"Chaos",
					"NNE",
					}
				);

			PublicDefinitions.Add("WITH_CLOTH_COLLISION_DETECTION=1");

			SetupModuleChaosVisualDebuggerSupport(target);
		}

		/// <summary>
		/// 根据自定义特性判断某个模块类型对目标是否有效
		/// </summary>
		/// <param name="moduleType">要检查的模块类型</param>
		/// <param name="targetRules">要对照检查的目标</param>
		/// <param name="invalidReason">输出参数，此模块无效的原因</param>
		/// <returns>模块有效返回 true，否则返回 false</returns>
		internal static bool IsValidForTarget(Type moduleType, ReadOnlyTargetRules targetRules, [NotNullWhen(false)] out string? invalidReason)
		{
			IEnumerable<TargetType> supportedTargetTypes = moduleType.GetCustomAttributes<SupportedTargetTypesAttribute>().SelectMany(x => x.TargetTypes).Distinct();
			if (supportedTargetTypes.Any() && !supportedTargetTypes.Contains(targetRules.Type))
			{
				invalidReason = $"TargetType '{targetRules.Type}'";
				return false;
			}

			IEnumerable<UnrealTargetConfiguration> supportedConfigurations = moduleType.GetCustomAttributes<SupportedConfigurationsAttribute>().SelectMany(x => x.Configurations).Distinct();
			if (supportedConfigurations.Any() && !supportedConfigurations.Contains(targetRules.Configuration))
			{
				invalidReason = $"Configuration '{targetRules.Configuration}'";
				return false;
			}

			// 跳过平台扩展模块。我们只关心基础模块，不关心平台覆盖版本。
			// 平台覆盖版本会在后续真正构建模块时应用。
			if (!UEBuildPlatform.GetPlatformFolderNames().Any(name => moduleType.Name.EndsWith("_" + name, StringComparison.OrdinalIgnoreCase)))
			{
				IEnumerable<SupportedPlatformsAttribute> platformAttributes = moduleType.GetCustomAttributes<SupportedPlatformsAttribute>();
				IEnumerable<UnrealTargetPlatform> supportedPlatforms = platformAttributes.SelectMany(x => x.Platforms).Distinct();
				if (platformAttributes.Any() && !supportedPlatforms.Contains(targetRules.Platform))
				{
					invalidReason = $"Platform '{targetRules.Platform}'";
					return false;
				}
			}

			IEnumerable<ObsoleteAttribute> obsoleteAttributes = moduleType.GetCustomAttributes<ObsoleteAttribute>();
			if (obsoleteAttributes.Any())
			{
				invalidReason = $"Obsolete '{(obsoleteAttributes.First().Message ?? "unspecified")}'";
				return false;
			}

			invalidReason = null;
			return true;
		}

		/// <summary>
		/// 判断此模块能否为当前目标预编译。
		/// </summary>
		/// <param name="rulesFile">模块规则文件的路径</param>
		/// <returns>模块可预编译返回 true，否则返回 false</returns>
		internal bool IsValidForTarget(FileReference rulesFile)
		{
			if (Type == ModuleType.CPlusPlus)
			{
				switch (PrecompileForTargets)
				{
					case PrecompileTargetsType.None:
						return false;
					case PrecompileTargetsType.Default:
						return (Target.Type == TargetType.Editor || !Unreal.GetExtensionDirs(Unreal.EngineDirectory, "Source/Developer").Any(dir => rulesFile.IsUnderDirectory(dir)) || Plugin != null);
					case PrecompileTargetsType.Game:
						return (Target.Type == TargetType.Client || Target.Type == TargetType.Server || Target.Type == TargetType.Game);
					case PrecompileTargetsType.Editor:
						return (Target.Type == TargetType.Editor);
					case PrecompileTargetsType.Any:
						return true;
				}
			}
			return false;
		}

		/// <summary>
		/// 判断给定平台在当前 Target 上下文中是否可用
		/// </summary>
		/// <param name="inPlatform">要检查的平台</param>
		/// <param name="bIgnoreSDKCheck">检查平台可用性时忽略 SDK 是否存在（许多平台可以作为目标平台 cook 而不需要 SDK）</param>
		/// <returns>可用返回 true，否则返回 false</returns>
		protected bool IsPlatformAvailable(UnrealTargetPlatform inPlatform, bool bIgnoreSDKCheck = false)
		{
			return UEBuildPlatform.IsPlatformAvailableForTarget(inPlatform, Target, bIgnoreSDKCheck);
		}

		/// <summary>
		/// 返回给定模块组中的所有模块
		/// </summary>
		/// <param name="moduleGroup">模块组名称，由 [ModuleGroup("Name")] 特性定义</param>
		/// <param name="bOnlyValid">只包含对当前 Target 有效的模块</param>
		/// <returns>模块名集合</returns>
		public IEnumerable<string> GetModulesInGroup(string moduleGroup, bool bOnlyValid = true)
		{
			// 找出这个选定列表中不允许的平台/组
			List<string>? disallowedPlatformsAndGroups = (bOnlyValid && Target.OptedInModulePlatforms != null) ? Utils.MakeListOfUnsupportedPlatforms(Target.OptedInModulePlatforms.ToList(), false, Logger) : null;

			foreach (Type moduleType in RulesAssembly.GetTypes()
				.Where(t => t.IsSubclassOf(typeof(ModuleRules)) && t.IsDefined(typeof(ModuleGroupsAttribute), true))
				.Where(t => !bOnlyValid || IsValidForTarget(t, Target, out string? _))
				)
			{
				// 检查模块文件是否被禁止
				if (disallowedPlatformsAndGroups != null)
				{
					FileReference? moduleFileName = RulesAssembly.GetModuleFileName(moduleType.Name);
					if (moduleFileName != null)
					{
						if (moduleFileName.ContainsAnyNames(disallowedPlatformsAndGroups, Unreal.EngineDirectory) ||
							(Target.ProjectFile != null && moduleFileName.ContainsAnyNames(disallowedPlatformsAndGroups, Target.ProjectFile.Directory)))
						{
							continue;
						}
					}
				}

				if (moduleType.GetCustomAttributes<ModuleGroupsAttribute>().Any(x => x.ModuleGroups.Contains(moduleGroup)))
				{
					yield return moduleType.Name;
				}
			}
		}

		/// <summary>
		/// 为构建低级测试可执行文件准备模块。
		/// 如果作为测试模块链的一部分构建模块，它们需要 LowLevelTestsRunner 依赖。
		/// </summary>
		internal void PrepareModuleForTests()
		{
			TestTargetRules? testTargetRules = Target.InnerTestTargetRules;
			if (testTargetRules == null)
			{
				return;
			}

			lock (testTargetRules)
			{
				if (Name != "LowLevelTestsRunner")
				{
					if (!PrivateIncludePathModuleNames.Contains("LowLevelTestsRunner"))
					{
						PrivateIncludePathModuleNames.Add("LowLevelTestsRunner");
					}
				}
			}
		}

		internal bool IsTestModule => bIsTestModuleOverride ?? false;

		/// <summary>
		/// 这是否是低级测试模块。
		/// </summary>
		protected bool? bIsTestModuleOverride { get; set; }

		/// <summary>
		/// 返回给定模块子类的模块目录（平台扩展会添加 ModuleRules 子类以加入平台特定设置）
		/// </summary>
		/// <param name="type">子类的 typeof</param>
		/// <returns>子类 .Build.cs 所在目录，找不到则返回 null</returns>
		public DirectoryReference? GetModuleDirectoryForSubClass(Type type) => DirectoriesForModuleSubClasses.TryGetValue(type, out DirectoryReference? directory) ? directory : null;

		/// <summary>
		/// 返回此模块所有子类的目录，以及规则指定的任何附加目录
		/// </summary>
		/// <returns>目录列表，若未添加则返回 null</returns>
		public DirectoryReference[] GetAllModuleDirectories()
		{
			List<DirectoryReference> allDirectories = new List<DirectoryReference>(DirectoriesForModuleSubClasses.Values);
			allDirectories.AddRange(AdditionalModuleDirectories);
			return allDirectories.ToArray();
		}

		/// <summary>
		/// 如果存在则添加一个附加模块目录（用于 NotForLicensees/NoRedist）
		/// </summary>
		/// <param name="directory">目录</param>
		/// <returns>目录存在返回 true</returns>
		protected bool ConditionalAddModuleDirectory(DirectoryReference directory)
		{
			if (DirectoryReference.Exists(directory))
			{
				AdditionalModuleDirectories.Add(directory);
				return true;
			}

			return false;
		}

		/// <summary>
		/// 返回构建配置是否支持 VcPkg。
		/// </summary>
		/// <returns>支持返回 true</returns>
		public bool IsVcPackageSupported => Target.Platform == UnrealTargetPlatform.Win64 ||
					Target.Platform == UnrealTargetPlatform.Linux ||
					Target.Platform == UnrealTargetPlatform.LinuxArm64 ||
					Target.Platform == UnrealTargetPlatform.Mac;

		/// <summary>
		/// 返回构建配置的 VcPkg 根目录
		/// </summary>
		/// <param name="packageName">第三方包名称</param>
		/// <returns>VcPkg 根目录</returns>
		[SuppressMessage("Globalization", "CA1308:Normalize strings to uppercase", Justification = "Architecture path is lowercase")]
		public string GetVcPackageRoot(string packageName)
		{
			// TODO: MacOS 支持、其他平台支持
			string targetPlatform = Target.Platform.ToString();
			string? platform = null;
			string? architecture = null;
			string linkage = String.Empty;
			string toolset = String.Empty;
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				platform = "windows";
				architecture = Target.WindowsPlatform.Architecture.ToString().ToLowerInvariant();
				if (Target.bUseStaticCRT)
				{
					linkage = "-static";
				}
				else
				{
					linkage = "-static-md";
				}
				toolset = "-v142";
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				architecture = "x86_64";
				platform = "unknown-linux-gnu";
			}
			else if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
			{
				architecture = "aarch64";
				platform = "unknown-linux-gnueabi";
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				architecture = "x86_64";
				platform = "osx";
			}

			if (String.IsNullOrEmpty(targetPlatform) || String.IsNullOrEmpty(platform) || String.IsNullOrEmpty(architecture))
			{
				throw new System.NotSupportedException($"Platform {Target.Platform} not currently supported by vcpkg");
			}

			string triplet = $"{architecture}-{platform}{linkage}{toolset}";

			return Path.Combine("ThirdParty", "vcpkg", targetPlatform, triplet, $"{packageName}_{triplet}");
		}

		/// <summary>
		/// 把用 vcpkg 编译的库添加到当前模块
		/// </summary>
		/// <param name="packageName">第三方包名称</param>
		/// <param name="addInclude">是否把 include 目录添加到 PublicIncludePaths</param>
		/// <param name="libraries">要添加到 PublicAdditionalLibraries 的库名称</param>
		public void AddVcPackage(string packageName, bool addInclude, params string[] libraries)
		{
			string vcPackageRoot = GetVcPackageRoot(packageName);

			if (!System.IO.Directory.Exists(vcPackageRoot))
			{
				throw new DirectoryNotFoundException(vcPackageRoot);
			}

			string libraryExtension = String.Empty;
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				libraryExtension = ".lib";
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64 || Target.Platform == UnrealTargetPlatform.Mac)
			{
				libraryExtension = ".a";
			}

			foreach (string library in libraries)
			{
				string libraryPath = Path.Combine(vcPackageRoot, "lib", $"{library}{libraryExtension}");
				if ((Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64 || Target.Platform == UnrealTargetPlatform.Mac) && !library.StartsWith("lib", StringComparison.OrdinalIgnoreCase))
				{
					libraryPath = Path.Combine(vcPackageRoot, "lib", $"lib{library}{libraryExtension}");
				}
				if (!System.IO.File.Exists(libraryPath))
				{
					throw new FileNotFoundException(libraryPath);
				}
				PublicAdditionalLibraries.Add(libraryPath);
			}

			if (addInclude)
			{
				string includePath = Path.Combine(vcPackageRoot, "include");
				if (!System.IO.Directory.Exists(includePath))
				{
					throw new DirectoryNotFoundException(includePath);
				}

				PublicSystemIncludePaths.Add(Path.Combine(vcPackageRoot, "include"));
			}
		}

		/// <summary>
		/// 把定义列表中的某个期望值替换为新值
		/// </summary>
		/// <param name="definitions">定义列表，例如 PublicDefinitions</param>
		/// <param name="name">要修改的宏名称</param>
		/// <param name="previousValue">期望的旧值</param>
		/// <param name="newValue">新值</param>
		/// <exception cref="Exception">旧值不存在时抛出</exception>
		protected static void ChangeDefinition(List<string> definitions, string name, string previousValue, string newValue)
		{
			if (!definitions.Remove($"{name}={previousValue}"))
			{
				throw new Exception("Failed to removed expected definition");
			}
			definitions.Add($"{name}={newValue}");
		}

		/// <summary>
		/// 把模块名列表中的某个期望值替换为新值
		/// </summary>
		/// <param name="definitions">模块名列表，例如 PublicDependencyModuleNames</param>
		/// <param name="previousModule">期望的旧模块名</param>
		/// <param name="newModule">新模块名</param>
		/// <exception cref="Exception">旧模块名不存在时抛出</exception>
		protected static void ReplaceModule(List<string> definitions, string previousModule, string newModule)
		{
			if (!definitions.Remove(previousModule))
			{
				throw new Exception("Failed to removed expected module name");
			}
			definitions.Add(newModule);
		}
	}
}
```

> ⚠️ **必须原样照抄的引擎原文细节**：
> 1. `VisibileForDll` —— 引擎原文拼写（不是 `Visible`），照抄；
> 2. `plVatform` 类错字在本文件里没有，但 `bEnableNonInlinedGenCppWarnings` 等废弃属性的 `[Obsolete]` 字符串**保持英文原文**（那是 API 契约，不是注释）；
> 3. `ChangeDefinition` / `ReplaceModule` 里 `"Failed to removed expected definition"` —— 引擎原文语法错误（`removed` 应为 `remove`），照抄；
> 4. `CreateModulRules` —— 引擎原文拼写（少一个 `e`），照抄。

---

## 三、逐段拆解（先讲清为什么，再给代码）

### 3.1 三个顶层枚举（第 18~98 行）

```csharp
	public enum WarningLevel
	{
		Default,
		Off,
		Warning,
		Error,
	}
```

| 值 | 含义 |
|---|---|
| `Default` | 用默认行为 |
| `Off` | 不显示诊断 |
| `Warning` | 正常输出警告 |
| `Error` | 当错误输出 |

> 类比：`WarningLevel` 是「这条警告的喇叭音量旋钮」——从静音到直接报错，四档。

```csharp
	public enum IWYUSupport
	{
		None,
		KeepAsIs,
		KeepAsIsForNow,
		KeepPublicAsIsForNow,
		Full,
	}
```

| 值 | 含义 |
|---|---|
| `None` | 代码甚至编不过，IWYU 完全跳过 |
| `KeepAsIs` | 能改但保持原样，手动处理 |
| `KeepAsIsForNow` | 解析处理但文件不改，推迟 IWYU |
| `KeepPublicAsIsForNow` | 同上，但允许改私有头和 cpp |
| `Full` | 完整 IWYU 支持 |

> `Core.Build.cs` 第 304 行 `IWYUSupport = IWYUSupport.KeepAsIs;` 用的就是这个枚举。

### 3.2 七个嵌套枚举（第 108~308 行）

```csharp
		public enum PCHUsageMode
		{
			Default,
			NoPCHs,
			NoSharedPCHs,
			UseSharedPCHs,
			UseExplicitOrSharedPCHs,
		}
```

| 值 | 含义 |
|---|---|
| `Default` | 引擎模块用共享 PCH，游戏模块不用 |
| `NoPCHs` | 从不用 PCH |
| `NoSharedPCHs` | 从不用共享 PCH，自己生成独立 PCH |
| `UseSharedPCHs` | 可以用共享 PCH |
| `UseExplicitOrSharedPCHs` | 没设私有 PCH 时可用共享 PCH |

> 类比：PCH 是「提前和好的面团」，这五档是「用不用现成面团、还是自己单独和」。

```csharp
		[Flags]
		public enum UnityFileType
		{
			None, Cpp, C, CC, CXX, M, MM,
		}
```

| 值 | 含义 |
|---|---|
| `Cpp` / `C` / `CC` / `CXX` | C++ / C / C++（备用扩展名）×2 |
| `M` / `MM` | Objective-C / Objective-C++ |

> `[Flags]` 表示可以按位组合，例如 `Cpp | C`。

### 3.3 六个嵌套辅助类（第 314~606 行）

```csharp
		public class RuntimeDependency
		{
			public string Path { get; init; }
			public string? SourcePath { get; init; }
			public StagedFileType Type { get; init; }
			// 2 个构造函数
		}
```

| 成员 | 含义 |
|---|---|
| `Path` | 暂存后的目标路径（用 `$(EngineDir)` / `$(ProjectDir)` 作根） |
| `SourcePath` | 构建时的源位置 |
| `Type` | 暂存方式（`StagedFileType`） |

> `Core.Build.cs` 第 96 行 `RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/DbgHelp/dbghelp.dll")` 用的就是它。

```csharp
		public class Framework
		{
			internal string Name { get; init; }
			internal string Path { get; init; }
			internal string? CopyBundledAssets { get; init; } = null;
			internal FrameworkMode Mode { get; init; }
			public enum FrameworkMode { Link, Copy, LinkAndCopy }
		}
```

| 成员 | 含义 |
|---|---|
| `Name` / `Path` | framework 名与路径（zip 或磁盘） |
| `Mode` | `Link` 只链接 / `Copy` 只复制进 .app / `LinkAndCopy` 两者都做 |

> 类比：`Framework` 是「Mac/iOS 的框架包裹单」——写清名字、位置、是链接还是塞进 app 包。

### 3.4 身份与路径（第 611~667 行）

```csharp
		public string Name { get; internal set; }
		internal FileReference File { get; set; }
		internal DirectoryReference Directory { get; set; }
		public ReadOnlyTargetRules Target { get; init; }
```

| 成员 | 含义 |
|---|---|
| `Name` | 模块名（= `Core.Build.cs` 里的类名 `Core`） |
| `File` / `Directory` | 模块的 `.Build.cs` 文件与目录 |
| `Target` | **只读目标规则**——所有 `Target.Platform` / `Target.Configuration` 判断的源头 |

> 🔑 `Target` 是 `init` 属性：只能在构造函数里赋值一次，之后只读。这就是为什么 `Core.Build.cs` 构造函数必须 `: base(Target)`。

### 3.5 编译配置（第 672~1066 行）

```csharp
		public string? PrivatePCHHeaderFile { get; set; }
		public string? SharedPCHHeaderFile { get; set; }
```

| 属性 | `Core.Build.cs` 用在哪 |
|---|---|
| `PrivatePCHHeaderFile` | 第 13 行 `= "Private/CorePrivatePCH.h"` |
| `SharedPCHHeaderFile` | 第 15 行 `= "Public/CoreSharedPCH.h"` |

```csharp
		public CppCompileWarnings CppCompileWarningSettings { get; init; }
```

| 属性 | `Core.Build.cs` 用在哪 |
|---|---|
| `CppCompileWarningSettings` | 第 302 行 `CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Error;` |

> ⚠️ 本文件里 8 个 `[Obsolete]` 的警告级别属性（`DeterministicWarningLevel` / `ShadowVariableWarningLevel` / `UnsafeTypeCastWarningLevel` / `UndefinedIdentifierWarningLevel` / `ModuleIncludePathWarningLevel` / `ModuleIncludePrivateWarningLevel` / `ModuleIncludeSubdirectoryWarningLevel`）**全部转发到 `CppCompileWarningSettings`**——这是 UE5.6 的重构：警告设置集中到一个对象里。

### 3.6 Unity 合批（第 1127~1167 行）

```csharp
		public int NumIncludedBytesPerUnityCPPOverride { get; set; } = 0;

		public int GetNumIncludedBytesPerUnityCPP() => (NumIncludedBytesPerUnityCPPOverride != 0 && !Target.bDisableModuleNumIncludedBytesPerUnityCPPOverride) ? NumIncludedBytesPerUnityCPPOverride : Target.NumIncludedBytesPerUnityCPP;
```

| 成员 | 含义 |
|---|---|
| `NumIncludedBytesPerUnityCPPOverride` | 覆盖每个 unity cpp 的字节上限，0 = 不覆盖 |
| `GetNumIncludedBytesPerUnityCPP()` | 取值助手：有覆盖用覆盖，否则用 `Target` 的默认值 |

> `Core.Build.cs` 第 11 行 `NumIncludedBytesPerUnityCPPOverride = 491520;` 就是设这个。

### 3.7 依赖与链接清单（第 1268~1438 行）★ 最常用

```csharp
		public List<string> PublicIncludePathModuleNames = new();
		public List<string> PublicDependencyModuleNames = new();
		public List<string> PrivateIncludePathModuleNames = new();
		public List<string> PrivateDependencyModuleNames = new();
```

| 清单 | 含义 | `Core.Build.cs` 用在哪 |
|---|---|---|
| `PublicIncludePathModuleNames` | 公开头需要它的头，但不链接 | 第 54 行 |
| `PublicDependencyModuleNames` | 公开源文件依赖，自动带 include | 第 39~41 行 |
| `PrivateIncludePathModuleNames` | 私有代码需要它的头，但不链接 | 第 43 行、第 63 行 |
| `PrivateDependencyModuleNames` | 私有代码依赖 | 第 17 行、第 33~37 行、第 300 行 |

> 类比：`DependencyModuleNames` 是「连人带设备一起借」，`IncludePathModuleNames` 是「只借说明书，不借设备」；`Public` 是「借来的东西可以转借给别人」，`Private` 是「只自己用」。

```csharp
		public List<string> PublicSystemLibraries = new();
		public List<string> PublicFrameworks = new();
		public List<string> PublicAdditionalLibraries = new();
		public List<string> PublicDelayLoadDLLs = new();
```

| 清单 | 含义 | `Core.Build.cs` 用在哪 |
|---|---|---|
| `PublicSystemLibraries` | 系统库（按名找） | 第 206 行 `Add("dl")` |
| `PublicFrameworks` | XCode framework | 第 121、137、142、147、163 行 |
| `PublicAdditionalLibraries` | 附加 .lib / .tbd | 第 131 行 |
| `PublicDelayLoadDLLs` | 延迟加载 DLL | 第 92 行 `Add("DBGHELP.DLL")` |

```csharp
		public List<string> PrivateDefinitions = new();
		public List<string> PublicDefinitions = new();
```

| 清单 | 含义 | `Core.Build.cs` 用在哪 |
|---|---|---|
| `PrivateDefinitions` | 私有宏 | 第 69、80、88、90、94、97、264、268、281、283、285、287、289、291、293、295 行 |
| `PublicDefinitions` | 公开宏 | 第 105、224、238、272、276、284、290、298、311 行 |

### 3.8 构造函数与两个第三方依赖助手（第 1584~1622 行）

```csharp
		public ModuleRules(ReadOnlyTargetRules target)
		{
			Target = target;
			CppCompileWarningSettings = new CppCompileWarnings(this, Logger);
		}
```

| 行 | 干什么 |
|---|---|
| `Target = target` | 保存目标规则（`init` 属性，只能在这里赋值） |
| `CppCompileWarningSettings = new CppCompileWarnings(this, Logger)` | 创建警告设置对象，把 `this` 和 `Logger` 传进去 |

> 🔑 这就是为什么 `Core.Build.cs` 必须写 `: base(Target)`——不调基类构造函数，`Target` 和 `CppCompileWarningSettings` 都是 null。

```csharp
		public void AddEngineThirdPartyPrivateStaticDependencies(ReadOnlyTargetRules target, params string[] moduleNames)
		{
			if (!bUsePrecompiled || target.LinkType == TargetLinkType.Monolithic)
			{
				PrivateDependencyModuleNames.AddRange(moduleNames);
			}
		}
```

| 行 | 干什么 |
|---|---|
| `params string[] moduleNames` | 可变参数，可以一次传多个模块名 |
| `!bUsePrecompiled \|\| LinkType == Monolithic` | 没用预编译数据，或者是单体链接 |
| `PrivateDependencyModuleNames.AddRange(...)` | 批量加进私有依赖 |

> 类比：这是「批量借设备」的助手——`Core.Build.cs` 里 `AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib")` 一行顶三行。

### 3.9 各种 Setup 助手（第 1627~1784 行）

```csharp
		public void SetupIrisSupport(ReadOnlyTargetRules target, bool bAddAsPublicDependency = false)
		{
			if (target.bUseIris == true)
			{
				PublicDefinitions.Add("UE_WITH_IRIS=1");
				PrivateDefinitions.Add("UE_NET_HAS_IRIS_FASTARRAY_BINDING=1");
				// 加 IrisCore
			}
			else
			{
				PublicDefinitions.Add("UE_WITH_IRIS=0");
				// 加 IrisStub
			}
		}
```

| 分支 | 干什么 |
|---|---|
| `bUseIris == true` | 定义 `UE_WITH_IRIS=1` + 加 `IrisCore` |
| 否则 | 定义 `UE_WITH_IRIS=0` + 加 `IrisStub`（桩模块，供 UHT 依赖） |

> ⚠️ `Core.Build.cs` 第 270~277 行**没有**调这个助手，而是自己手写了 `UE_WITH_IRIS=1/0`——因为 Core 注释明说「完全不依赖 Iris」，所以只定义宏、不加模块。

### 3.10 路径属性（第 1532~1565 行）

```csharp
		public string EngineDirectory => Unreal.EngineDirectory.FullName;
		public string ModuleDirectory => Directory.FullName;
		public string PluginDirectory => Plugin == null ? throw ... : Plugin.Directory.FullName;
```

| 属性 | 含义 | `Core.Build.cs` 用在哪 |
|---|---|---|
| `EngineDirectory` | 引擎根目录 | 第 196 行 `Path.Combine(EngineDirectory, "Build", "Android", ...)` |
| `ModuleDirectory` | 本模块目录 | 第 29 行 `Utils.MakePathRelativeTo(ModuleDirectory, ...)` |
| `PluginDirectory` | 插件目录（非插件模块访问会抛异常） | 未用 |

---

## 四、我们这版 vs 引擎的差异

| 项 | 引擎原文 | 我们这版 | 说明 |
|----|---------|---------|------|
| 文件名 | `ModuleRules.cs` | `ModuleRules.cs` | **完全一致** |
| 路径 | `Engine\Source\Programs\UnrealBuildTool\Configuration\` | `ue_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\` | 只换根 |
| 类名 | `ModuleRules` | `ModuleRules` | 完全一致 |
| 代码行 | 2119 行 | 2119 行 | **行数一致**（注释原地中文化，不增行） |
| 英文注释 | 全部 | 0 处 | 全部译成中文 |
| `[Obsolete]` 字符串 | 英文 | **保持英文** | 那是 API 契约，不是注释 |
| 引擎错字 | `VisibileForDll` / `CreateModulRules` / `Failed to removed` | 原样保留 | 一比一照抄 |

> 除注释中文化外，**代码一字不差**。

---

## 五、收尾

到这里 `ModuleRules.cs` 就完了（2119 行）。

**结构**：3 个顶层枚举 + 1 个 `ModuleRules` 类（7 个嵌套枚举 + 6 个嵌套辅助类 + 约 150 个属性 + 约 20 个方法）。

**关键概念**：
- `Target` 是 `init` 属性 → 必须在构造函数里 `: base(Target)` 传进去
- `CppCompileWarningSettings` 是 UE5.6 新结构 → 8 个旧警告属性全部转发到它
- `Public*` vs `Private*` → 是否对外传递
- `DependencyModuleNames` vs `IncludePathModuleNames` → 连库 vs 只要头
- `AddEngineThirdPartyPrivateStaticDependencies` → 批量加第三方静态依赖的助手

---

## 待确认

1. 2119 行代码看懂了吗？这份是**答疑文件**，不用抄（主线仍是 `Core.Build.cs`）。
2. 重点看三处：`Target` 的 `init` 语义（第 667 行）、`CppCompileWarningSettings` 的转发（第 982 行）、`AddEngineThirdPartyPrivateStaticDependencies` 的实现（第 1599 行）。
3. 看完说「下一步」，回主线继续 `Core.Build.cs` 验收 → `Core_APL.xml`。

---

## 源码依据

- 引擎：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（2119 行 / 76049 字节）
- 使用方：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行）
- 你的还原：`ue_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（答疑用，可不建）