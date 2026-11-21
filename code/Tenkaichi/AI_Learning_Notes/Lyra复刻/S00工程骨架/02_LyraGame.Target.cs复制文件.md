# 02 — `LyraGame.Target.cs` 复制文件

> 依据：`E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs`（**279 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame.Target.cs`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**（被注释掉的代码行要抄）。

---

```csharp
// 版权声明：Epic Games 保留所有权利
// Copyright Epic Games, Inc. All Rights Reserved.

// 引入 UnrealBuildTool 命名空间；TargetRules / TargetInfo / TargetType 都在这里
using UnrealBuildTool;
// 引入 System；用到 Environment.GetEnvironmentVariable
using System;
// 引入 System.IO；用到 Path.Combine
using System.IO;
// 引入 EpicGames.Core；用到 JsonObject / FileReference / DirectoryReference
using EpicGames.Core;
// 引入泛型集合；用到 List / Dictionary
using System.Collections.Generic;
// 引入 UnrealBuildBase；用到 Unreal.GetExtensionDirs / PluginsBase.EnumeratePlugins
using UnrealBuildBase;
// 引入日志抽象；用到 ILogger
using Microsoft.Extensions.Logging;

// 定义 LyraGame 目标类；类名必须与文件名 LyraGame.Target.cs 的前缀一致
public class LyraGameTarget : TargetRules
{
	// 构造函数；UBT 创建目标时调用，TargetInfo 携带命令行传入的平台/配置信息
	public LyraGameTarget(TargetInfo Target) : base(Target)
	{
		// 目标类型 = 游戏；打包成可执行游戏，不含编辑器
		Type = TargetType.Game;

		// 本目标要编译的模块列表；只有 LyraGame 一个运行时模块
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });

		// 应用 Lyra 所有目标共用的设置；方法定义在本文件下方
		LyraGameTarget.ApplySharedLyraTargetSettings(this);
	}

	// 静态标志位：是否已经警告过「共享引擎构建下无法动态开关插件」
	// 用 static 是为了让同一次 UBT 进程内只警告一次，避免刷屏
	private static bool bHasWarnedAboutShared = false;

	// 所有 Lyra 目标（Game / Editor / Client / Server）共用的设置入口
	// 参数 Target 是调用方的 TargetRules 实例；可能是 LyraGameTarget，也可能是 LyraEditorTarget
	internal static void ApplySharedLyraTargetSettings(TargetRules Target)
	{
		// 取出 UBT 的日志器；用于输出警告/调试信息
		ILogger Logger = Target.Logger;
		
		// 默认编译设置版本 = V5；UE5 的推荐设置，包含更严格的警告与更优的默认值
		Target.DefaultBuildSettings = BuildSettingsVersion.V5;
		// 头文件包含顺序版本 = 最新；避免旧版包含顺序导致的隐式依赖
		Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// 当前配置是否为 Test；测试版带日志、带统计，但不带调试符号
		bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
		// 当前配置是否为 Shipping；发行版无日志、无调试、最高优化
		bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
		// 当前目标是否为专用服务器；无渲染、无 UI
		bool bIsDedicatedServer = Target.Type == TargetType.Server;
		// 判断构建环境：Unique = 本工程独立编译引擎源码；Shared = 复用已安装引擎的预编译二进制
		if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
		{
			// 变量遮蔽（shadow variable）警告提升为错误；防止 lambda 捕获同名变量引发 bug
			Target.ShadowVariableWarningLevel = WarningLevel.Error;

			// Shipping 下也保留日志；Lyra 需要线上日志排查问题
			Target.bUseLoggingInShipping = true;

			// 如果是 Shipping 且不是专用服务器；即玩家客户端
			if (bIsShipping && !bIsDedicatedServer)
			{
				// 强制校验 HTTPS 证书；防止中间人攻击
				Target.bDisableUnverifiedCertificates = true;

				// 下面两行是「命令行白名单」的示例；默认注释掉
				//Target.GlobalDefinitions.Add("UE_COMMAND_LINE_USES_ALLOW_LIST=1");
				//Target.GlobalDefinitions.Add("UE_OVERRIDE_COMMAND_LINE_ALLOW_LIST=\"-space -separated -list -of -commands\"");

				// 下面这行是「过滤敏感命令行参数」的示例；默认注释掉
				//Target.GlobalDefinitions.Add("FILTER_COMMANDLINE_LOGGING=\"-some_connection_id -some_other_arg\"");
			}

			// 如果是 Shipping 或 Test 配置
			if (bIsShipping || bIsTest)
			{
				// 禁止读取「打包时生成的 ini」；只认打包进 pak 的 ini
				Target.bAllowGeneratedIniWhenCooked = false;
				// 禁止读取「非 UFS（未打包）的 ini」
				Target.bAllowNonUFSIniWhenCooked = false;
			}

			// 如果不是编辑器目标；即 Game / Client / Server
			if (Target.Type != TargetType.Editor)
			{
				// 运行时不需要路径追踪器；它只用于出高质量截图，且该 DLL 体积很大
				Target.DisablePlugins.Add("OpenImageDenoise");
			}

			// 扫描并配置 GameFeature 插件；方法定义在本文件下方
			LyraGameTarget.ConfigureGameFeaturePlugins(Target);
		}
		// 共享引擎构建分支；复用已安装引擎的二进制
		else
		{
			// 警告：这里的任何改动都不能影响 PCH 生成，否则必须改用 TargetBuildEnvironment.Unique

			// 共享构建下只有编辑器目标能动态开关插件
			if (Target.Type == TargetType.Editor)
			{
				// 编辑器目标：允许扫描并配置 GameFeature 插件
				LyraGameTarget.ConfigureGameFeaturePlugins(Target);
			}
			// 非编辑器目标
			else
			{
				// 共享单体构建无法开关插件，也无法改任何选项；因为它要复用已安装引擎的二进制
				if (!bHasWarnedAboutShared)
				{
					// 只警告一次
					bHasWarnedAboutShared = true;
					// 输出警告：从已安装引擎打包时，LyraGameEOS 与动态目标选项会被禁用
					Logger.LogWarning("LyraGameEOS and dynamic target options are disabled when packaging from an installed version of the engine");
				}
			}
		}
	}

	// 判断是否要「编译全部 GameFeature 插件」
	// 返回 false = 按插件浏览器里的默认规则（推荐）
	static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
	{
		// 如果是编辑器目标
		if (Target.Type == TargetType.Editor)
		{
			// 返回 true 时，编辑器构建会编译所有 GameFeature 插件，但不一定全部加载
			// 这样你可以在编辑器里启用插件而无需重新编译代码
			// return true;
		}

		// 判断是否在构建机上；环境变量 IsBuildMachine == "1"
		bool bIsBuildMachine = (Environment.GetEnvironmentVariable("IsBuildMachine") == "1");
		// 如果在构建机上
		if (bIsBuildMachine)
		{
			// 构建机上可以返回 true 来编译全部插件
			// return true;
		}

		// 默认：使用插件浏览器里设置的默认规则
		// 这一点很重要，因为对于启动器安装版引擎，这段代码可能根本不会执行
		return false;
	}

	// 缓存：插件名 → 已解析的 .uplugin JSON 对象
	// 避免同一个插件被多次读取磁盘
	private static Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new Dictionary<string, JsonObject>();

	// 配置哪些 GameFeature 插件需要启用
	// 这是一个相当简单的实现，但你可以做得更复杂
	// 例如：根据当前分支的目标发布版本，构建不同的插件
	// （main 分支启用开发中功能，release 分支禁用）
	static public void ConfigureGameFeaturePlugins(TargetRules Target)
	{
		// 取出日志器
		ILogger Logger = Target.Logger;
		// 输出一次分支名；TraceInformationOnce 保证只输出一次
		Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", Target.Version.BranchName);

		// 是否要编译全部 GameFeature 插件
		bool bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(Target);

		// 收集所有 GameFeature 插件的 .uplugin 文件路径
		List<FileReference> CombinedPluginList = new List<FileReference>();

		// 查找所有可能的 GameFeature 插件根目录；支持引擎扩展目录 + 工程目录
		List<DirectoryReference> GameFeaturePluginRoots = Unreal.GetExtensionDirs(Target.ProjectFile.Directory, Path.Combine("Plugins", "GameFeatures"));
		// 遍历每个根目录
		foreach (DirectoryReference SearchDir in GameFeaturePluginRoots)
		{
			// 枚举该目录下所有 .uplugin 文件；加入总列表
			CombinedPluginList.AddRange(PluginsBase.EnumeratePlugins(SearchDir));
		}

		// 如果找到了插件
		if (CombinedPluginList.Count > 0)
		{
			// 记录「插件名 → 它引用的其他插件名列表」；用于后续校验
			Dictionary<string, List<string>> AllPluginReferencesByName = new Dictionary<string, List<string>>();

			// 遍历每个 .uplugin 文件
			foreach (FileReference PluginFile in CombinedPluginList)
			{
				// 文件存在才处理
				if (PluginFile != null && FileReference.Exists(PluginFile))
				{
					// 最终是否启用
					bool bEnabled = false;
					// 是否被强制禁用；优先级高于 bEnabled
					bool bForceDisabled = false;
					// 解析 .uplugin 可能抛异常，用 try 包住
					try
					{
						// 解析后的 JSON 对象
						JsonObject RawObject;
						// 先查缓存；没命中才读磁盘
						if (!AllPluginRootJsonObjectsByName.TryGetValue(PluginFile.GetFileNameWithoutExtension(), out RawObject))
						{
							// 读取并解析 .uplugin
							RawObject = JsonObject.Read(PluginFile);
							// 写入缓存
							AllPluginRootJsonObjectsByName.Add(PluginFile.GetFileNameWithoutExtension(), RawObject);
						}

						// 校验：所有 GameFeature 插件默认都应该是禁用的
						// 如果 EnabledByDefault 为 true 且插件被禁用，插件名会被嵌进可执行文件
						// 如果这是个问题，可以打开下面的警告，并修改 GameFeature 插件模板
						bool bEnabledByDefault = false;
						// 没设 EnabledByDefault 或设成 true 时进入
						if (!RawObject.TryGetBoolField("EnabledByDefault", out bEnabledByDefault) || bEnabledByDefault == true)
						{
							// 警告已注释掉；Lyra 默认不报
							//Log.TraceWarning("GameFeaturePlugin {0}, does not set EnabledByDefault to false. This is required for built-in GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// 校验：所有 GameFeature 插件都必须显式加载
						// 因为 GameFeature 插件期望在工程启动之后才被加载
						bool bExplicitlyLoaded = false;
						// 没设 ExplicitlyLoaded 或设成 false 时进入
						if (!RawObject.TryGetBoolField("ExplicitlyLoaded", out bExplicitlyLoaded) || bExplicitlyLoaded == false)
						{
							// 没设 ExplicitlyLoaded = true 就警告
							Logger.LogWarning("GameFeaturePlugin {0}, does not set ExplicitlyLoaded to true. This is required for GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// 这里可以读取额外的工程自定义字段；例如发布版本号
						//string PluginReleaseVersion;
						//if (RawObject.TryGetStringField("MyProjectReleaseVersion", out PluginReleaseVersion))
						//{
						//		bEnabled = SomeFunctionOf(PluginReleaseVersion, CurrentReleaseVersion) || bBuildAllGameFeaturePlugins;
						//}

						// 如果要求编译全部插件
						if (bBuildAllGameFeaturePlugins)
						{
							// 全部启用；除了无法加载/编译的
							bEnabled = true;
						}

						// 防止在非编辑器构建里使用「仅编辑器」插件
						bool bEditorOnly = false;
						// 读取 EditorOnly 字段
						if (RawObject.TryGetBoolField("EditorOnly", out bEditorOnly))
						{
							// 是仅编辑器插件，且当前不是编辑器目标，且不要求全编译 → 强制禁用
							if (bEditorOnly && (Target.Type != TargetType.Editor) && !bBuildAllGameFeaturePlugins)
							{
								// 该插件仅编辑器可用，而我们在编非编辑器目标，所以禁用
								bForceDisabled = true;
							}
						}
						// 没写 EditorOnly 字段
						else
						{
							// EditorOnly 是可选字段
						}

						// 某些插件只允许在特定分支使用
						string RestrictToBranch;
						// 读取 RestrictToBranch 字段
						if (RawObject.TryGetStringField("RestrictToBranch", out RestrictToBranch))
						{
							// 当前分支名与限制分支不一致；忽略大小写比较
							if (!Target.Version.BranchName.Equals(RestrictToBranch, StringComparison.OrdinalIgnoreCase))
							{
								// 该插件属于其他分支 → 强制禁用
								bForceDisabled = true;
								Logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to other branches. Disabling.", PluginFile.GetFileNameWithoutExtension());
							}
							// 分支匹配
							else
							{
								// 保持启用
								Logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to this branch. Leaving enabled.", PluginFile.GetFileNameWithoutExtension());
							}
						}

						// 插件可以标记 NeverBuild；优先级高于上面所有判断
						bool bNeverBuild = false;
						// 读取 NeverBuild 字段且为 true
						if (RawObject.TryGetBoolField("NeverBuild", out bNeverBuild) && bNeverBuild)
						{
							// 标记为永不编译 → 强制禁用
							bForceDisabled = true;
							Logger.LogDebug("GameFeaturePlugin {Name} was marked as NeverBuild, disabling.", PluginFile.GetFileNameWithoutExtension());
						}

						// 记录插件引用关系；供后续校验使用
						JsonObject[] PluginReferencesArray;
						// 读取 Plugins 数组字段
						if (RawObject.TryGetObjectArrayField("Plugins", out PluginReferencesArray))
						{
							// 遍历该插件引用的每个插件
							foreach (JsonObject ReferenceObject in PluginReferencesArray)
							{
								// 引用是否启用
								bool bRefEnabled = false;
								// 引用启用才记录
								if (ReferenceObject.TryGetBoolField("Enabled", out bRefEnabled) && bRefEnabled == true)
								{
									// 被引用插件的名字
									string PluginReferenceName;
									// 读取 Name 字段
									if (ReferenceObject.TryGetStringField("Name", out PluginReferenceName))
									{
										// 引用方插件名；即当前插件
										string ReferencerName = PluginFile.GetFileNameWithoutExtension();
										// 首次出现则创建列表
										if (!AllPluginReferencesByName.ContainsKey(ReferencerName))
										{
											AllPluginReferencesByName[ReferencerName] = new List<string>();
										}
										// 记录引用关系
										AllPluginReferencesByName[ReferencerName].Add(PluginReferenceName);
									}
								}
							}
						}
					}
					// 解析 .uplugin 失败
					catch (Exception ParseException)
					{
						// 警告并强制禁用
						Logger.LogWarning("Failed to parse GameFeaturePlugin file {Name}, disabling. Exception: {1}", PluginFile.GetFileNameWithoutExtension(), ParseException.Message);
						bForceDisabled = true;
					}

					// 禁用优先级高于启用
					if (bForceDisabled)
					{
						bEnabled = false;
					}

					// 输出最终决定
					Logger.LogDebug("ConfigureGameFeaturePlugins() has decided to {Action} feature {Name}", bEnabled ? "enable" : (bForceDisabled ? "disable" : "ignore"), PluginFile.GetFileNameWithoutExtension());

					// 根据决定启用或禁用
					if (bEnabled)
					{
						// 加入启用列表
						Target.EnablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
					// 被强制禁用
					else if (bForceDisabled)
					{
						// 加入禁用列表
						Target.DisablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
				}
			}

			// 如果你使用发布版本号机制，可以考虑在这里做引用校验
			// 确保「较早发布版本的插件」不会依赖「较晚发布版本的内容」
		}
	}
}
```

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraGame.Target.cs`（**在 `Source\` 根下，不在 `Source\LyraGame\` 里**） |
| 类名 | `LyraGameTarget`，与文件名前缀一致 |
| 目标类型 | `Type = TargetType.Game;` |
| 模块列表 | `ExtraModuleNames.AddRange(new string[] { "LyraGame" });` |
| 静态方法 | `ApplySharedLyraTargetSettings` / `ShouldEnableAllGameFeaturePlugins` / `ConfigureGameFeaturePlugins` 三个都在 |
| 返回值 | `ShouldEnableAllGameFeaturePlugins` 最后 `return false;` |
| 注释 | 所有 `//` 开头的**纯注释行**全部删掉；`//Target.GlobalDefinitions...` 这类**被注释掉的代码**要保留 |
| 缩进 | Tab（不是空格） |

---

## 源码依据

- `E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs:1-279`（全文）
- `E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs:11-20`（类声明与构造函数）
- `E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs:24-91`（`ApplySharedLyraTargetSettings`）
- `E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs:93-112`（`ShouldEnableAllGameFeaturePlugins`）
- `E:\ue5\LyraStarterGame\Source\LyraGame.Target.cs:120-278`（`ConfigureGameFeaturePlugins`）