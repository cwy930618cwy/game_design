// 引入 UnrealBuildTool 命名空间；ModuleRules / ReadOnlyTargetRules / PCHUsageMode 都在这里
using UnrealBuildTool;

// 定义 LyraEditor 模块规则类；类名必须与文件名前缀 LyraEditor 一致
public class LyraEditor : ModuleRules
// 类体开始
{
	// 构造函数；UBT 编译本模块时调用；参数是「只读的目标规则」，模块不许改目标配置
	public LyraEditor(ReadOnlyTargetRules Target) : base(Target)
	// 构造函数体开始
	{
		// PCH 使用模式：优先共享 PCH，没有则用显式 PCH；UE5 推荐写法，加快编译
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共依赖模块：本模块头文件里 include 了它们；依赖会向下传染给依赖 LyraEditor 的模块
		PublicDependencyModuleNames.AddRange(
			// 字符串数组开始
			new string[] {
				// 最底层：FString / TArray / 容器 / 数学库
				"Core",
				// UObject 系统：UCLASS / UPROPERTY / 反射 / GC
				"CoreUObject",
				// 引擎主体：AActor / UActorComponent / UWorld / 渲染 / 物理
				"Engine",
				// 编辑器基础设施：FEdMode / 编辑器窗口基类 / 资产编辑器框架
				"UnrealEd",
				// ★ 主游戏模块；编辑器模块要操作 LyraGame 里的类，所以必须依赖它
				"LyraGame"
				// 数组结束（最后一个元素后没有逗号，与源一致）
			}
			// 添加公共依赖结束
		);
		// 构造函数结束
	}
// 类体结束
}