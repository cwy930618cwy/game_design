// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using EpicGames.Core;
using System.Collections.Generic;
using UnrealBuildBase;
using Microsoft.Extensions.Logging;

public class TenkaichiGameTarget : TargetRules
{
	public TenkaichiGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange(new string[] { "TenkaichiGame" });

		TenkaichiGameTarget.ApplySharedTenkaichiTargetSettings(this);
	}

	private static bool bHasWarnedAboutShared = false;

	internal static void ApplySharedTenkaichiTargetSettings(TargetRules Target)
	{
		ILogger Logger = Target.Logger;
		
		Target.DefaultBuildSettings = BuildSettingsVersion.V5;
		Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
		bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
		bool bIsDedicatedServer = Target.Type == TargetType.Server;
		if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
		{
			Target.CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;

			Target.bUseLoggingInShipping = true;
			Target.bTrackRHIResourceInfoForTest = true;

			if (bIsShipping && !bIsDedicatedServer)
			{
				// 确保校验 HTTPS 流量的证书
				Target.bDisableUnverifiedCertificates = true;

				// 取消下面几行注释，可以锁定命令行参数处理
				// 这样只会解析指定的命令行参数
				//Target.GlobalDefinitions.Add("UE_COMMAND_LINE_USES_ALLOW_LIST=1");
				//Target.GlobalDefinitions.Add("UE_OVERRIDE_COMMAND_LINE_ALLOW_LIST=\"-space -separated -list -of -commands\"");

				// 取消这行注释，可以过滤掉你不想写进日志文件的敏感命令行参数
				//（比如你要上传日志的时候）
				//Target.GlobalDefinitions.Add("FILTER_COMMANDLINE_LOGGING=\"-some_connection_id -some_other_arg\"");
			}

			if (bIsShipping || bIsTest)
			{
				// 禁止读取烘焙时生成的 / 非 ufs 的 ini 文件
				Target.bAllowGeneratedIniWhenCooked = false;
				Target.bAllowNonUFSIniWhenCooked = false;
			}

			if (Target.Type != TargetType.Editor)
			{
				// 运行时用不到路径追踪，只用于拍美图，而且这个 DLL 很大
				Target.DisablePlugins.Add("OpenImageDenoise");

				// 降低 AssetRegistry 常驻数据的内存占用，代价是查询更耗 CPU
				Target.GlobalDefinitions.Add("UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1");
			}

			TenkaichiGameTarget.ConfigureGameFeaturePlugins(Target);
		}
		else
		{
			// !!!!!!!!!!!! 警告 !!!!!!!!!!!!!
			// 这里面的任何改动都不能影响 PCH 生成，否则要把
			// 目标设为 TargetBuildEnvironment.Unique

			// 这只能在编辑器或 Unique 构建环境下生效
			if (Target.Type == TargetType.Editor)
			{
				TenkaichiGameTarget.ConfigureGameFeaturePlugins(Target);
			}
			else
			{
				// 共享的整块构建无法启用/禁用插件或改任何选项，因为它要复用已安装的引擎二进制
				if (!bHasWarnedAboutShared)
				{
					bHasWarnedAboutShared = true;
					Logger.LogWarning("TenkaichiGameEOS and dynamic target options are disabled when packaging from an installed version of the engine");
				}
			}
		}
	}

	static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
	{
		if (Target.Type == TargetType.Editor)
		{
			// 若 return true，编辑器构建会编译所有 GameFeature 插件，但不一定会全部加载。
			// 这样你就能在编辑器里启用插件而无需重新编译代码。
			// return true;
		}

		bool bIsBuildMachine = (Environment.GetEnvironmentVariable("IsBuildMachine") == "1");
		if (bIsBuildMachine)
		{
			// 这可以用来给构建机启用所有插件
			// return true;
		}

		// 默认情况下，使用编辑器里插件浏览器设置的默认插件规则
		// 这点很重要，因为对于通过启动器安装的引擎版本，这段代码可能根本不会执行
		return false;
	}

	private static Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new Dictionary<string, JsonObject>();

	// 配置我们想要启用哪些 GameFeature 插件
	// 这是一个比较简单的实现，但你也可以根据当前分支的目标发布版本，
	// 来构建不同的插件，比如在 main 分支启用开发中的功能，
	// 而在当前发布分支禁用它们。
	static public void ConfigureGameFeaturePlugins(TargetRules Target)
	{
		ILogger Logger = Target.Logger;
		Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", Target.Version.BranchName);

		bool bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(Target);

		// 加载所有 GameFeature 的 .uplugin 描述文件
		List<FileReference> CombinedPluginList = new List<FileReference>();

		List<DirectoryReference> GameFeaturePluginRoots = Unreal.GetExtensionDirs(Target.ProjectFile.Directory, Path.Combine("Plugins", "GameFeatures"));
		foreach (DirectoryReference SearchDir in GameFeaturePluginRoots)
		{
			CombinedPluginList.AddRange(PluginsBase.EnumeratePlugins(SearchDir));
		}

		if (CombinedPluginList.Count > 0)
		{
			Dictionary<string, List<string>> AllPluginReferencesByName = new Dictionary<string, List<string>>();

			foreach (FileReference PluginFile in CombinedPluginList)
			{
				if (PluginFile != null && FileReference.Exists(PluginFile))
				{
					bool bEnabled = false;
					bool bForceDisabled = false;
					try
					{
						JsonObject RawObject;
						if (!AllPluginRootJsonObjectsByName.TryGetValue(PluginFile.GetFileNameWithoutExtension(), out RawObject))
						{
							RawObject = JsonObject.Read(PluginFile);
							AllPluginRootJsonObjectsByName.Add(PluginFile.GetFileNameWithoutExtension(), RawObject);
						}

						// 校验所有 GameFeaturePlugin 默认都是禁用的
						// 如果 EnabledByDefault 为 true 而插件被禁用，插件名会被嵌入可执行文件
						// 如果这是问题，就启用这个警告，并让 GameFeature 编辑器插件模板对新插件禁用 EnabledByDefault
						bool bEnabledByDefault = false;
						if (!RawObject.TryGetBoolField("EnabledByDefault", out bEnabledByDefault) || bEnabledByDefault == true)
						{
							//Log.TraceWarning("GameFeaturePlugin {0}, does not set EnabledByDefault to false. This is required for built-in GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// 校验所有 GameFeaturePlugin 都设置为显式加载
						// 这点很重要，因为 GameFeature 插件期望在项目启动后才被加载
						bool bExplicitlyLoaded = false;
						if (!RawObject.TryGetBoolField("ExplicitlyLoaded", out bExplicitlyLoaded) || bExplicitlyLoaded == false)
						{
							Logger.LogWarning("GameFeaturePlugin {0}, does not set ExplicitlyLoaded to true. This is required for GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// 你还可以在这里读一个项目专属的额外字段，比如：
						//string PluginReleaseVersion;
						//if (RawObject.TryGetStringField("MyProjectReleaseVersion", out PluginReleaseVersion))
						//{
						//		bEnabled = SomeFunctionOf(PluginReleaseVersion, CurrentReleaseVersion) || bBuildAllGameFeaturePlugins;
						//}

						if (bBuildAllGameFeaturePlugins)
						{
							// 我们处于「想要所有 GameFeature 插件」的模式，除了那些无法加载或编译的
							bEnabled = true;
						}

						// 防止在非编辑器构建里使用仅编辑器插件
						bool bEditorOnly = false;
						if (RawObject.TryGetBoolField("EditorOnly", out bEditorOnly))
						{
							if (bEditorOnly && (Target.Type != TargetType.Editor) && !bBuildAllGameFeaturePlugins)
							{
								// 该插件仅编辑器可用，而我们正在构建非编辑器目标，所以禁用
								bForceDisabled = true;
							}
						}
						else
						{
							// EditorOnly 是可选的
						}

						// 有些插件只应在特定分支可用
						string RestrictToBranch;
						if (RawObject.TryGetStringField("RestrictToBranch", out RestrictToBranch))
						{
							if (!Target.Version.BranchName.Equals(RestrictToBranch, StringComparison.OrdinalIgnoreCase))
							{
								// 该插件只属于某个特定分支，而这不是那个分支
								bForceDisabled = true;
								Logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to other branches. Disabling.", PluginFile.GetFileNameWithoutExtension());
							}
							else
							{
								Logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to this branch. Leaving enabled.", PluginFile.GetFileNameWithoutExtension());
							}
						}

						// 插件可以被标记为 NeverBuild，它会覆盖上面的判断
						bool bNeverBuild = false;
						if (RawObject.TryGetBoolField("NeverBuild", out bNeverBuild) && bNeverBuild)
						{
							// 该插件被标记为永不编译，所以不编译
							bForceDisabled = true;
							Logger.LogDebug("GameFeaturePlugin {Name} was marked as NeverBuild, disabling.", PluginFile.GetFileNameWithoutExtension());
						}

						// 记录插件引用，供后续校验
						JsonObject[] PluginReferencesArray;
						if (RawObject.TryGetObjectArrayField("Plugins", out PluginReferencesArray))
						{
							foreach (JsonObject ReferenceObject in PluginReferencesArray)
							{
								bool bRefEnabled = false;
								if (ReferenceObject.TryGetBoolField("Enabled", out bRefEnabled) && bRefEnabled == true)
								{
									string PluginReferenceName;
									if (ReferenceObject.TryGetStringField("Name", out PluginReferenceName))
									{
										string ReferencerName = PluginFile.GetFileNameWithoutExtension();
										if (!AllPluginReferencesByName.ContainsKey(ReferencerName))
										{
											AllPluginReferencesByName[ReferencerName] = new List<string>();
										}
										AllPluginReferencesByName[ReferencerName].Add(PluginReferenceName);
									}
								}
							}
						}
					}
					catch (Exception ParseException)
					{
						Logger.LogWarning("Failed to parse GameFeaturePlugin file {Name}, disabling. Exception: {1}", PluginFile.GetFileNameWithoutExtension(), ParseException.Message);
						bForceDisabled = true;
					}

					// 禁用的优先级高于启用
					if (bForceDisabled)
					{
						bEnabled = false;
					}

					// 打印这个插件的最终决定
					Logger.LogDebug("ConfigureGameFeaturePlugins() has decided to {Action} feature {Name}", bEnabled ? "enable" : (bForceDisabled ? "disable" : "ignore"), PluginFile.GetFileNameWithoutExtension());

					// 启用或禁用它
					if (bEnabled)
					{
						Target.EnablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
					else if (bForceDisabled)
					{
						Target.DisablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
				}
			}

			// 如果你用了类似发布版本的东西，可以考虑做引用校验，
			// 确保较早发布版本的插件不依赖较晚发布版本的内容
		}
	}
}