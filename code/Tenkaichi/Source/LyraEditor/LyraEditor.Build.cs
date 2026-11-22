// 版权声明：Epic Games 保留所有权利
//
// 说明：这是 S00 阶段的「占位模块」。
// LyraEditor.Target.cs 的 ExtraModuleNames 引用了 LyraEditor 模块，
// 但完整的编辑器模块要到 S22 才建。若此处没有 Build.cs，UBT 生成
// IntelliSense/工程文件时会报 "Could not find definition for module 'LyraEditor'"
// 并导致 Rider 的包含路径模型失效（引擎头文件全部报红）。
// 先放一个最小可解析的模块定义，等 S22 再用 Lyra 正式内容替换。

using UnrealBuildTool;

public class LyraEditor : ModuleRules
{
	public LyraEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"LyraGame"
			}
		);
	}
}
