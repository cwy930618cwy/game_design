# 06 — `LyraEditor.Build.cs` 复制文件

> 依据：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs`（**28 行 / 891 B，占位版**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。
> ⚠️ 这是 **S00 占位版**，不是 Lyra 正式版；S22 会用完整版覆盖。

---

## 完整代码

```csharp
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
```

> ⚠️ **你现有文件顶部有 8 行「说明：这是 S00 阶段的占位模块…」注释块**（第 1-8 行）。
> 按铁律第 4 条「零教学噪声」，**这一整块要删掉** —— 注释只写在被注释代码的正上方，不许写「文件级说明块」。
> 删完你的文件代码行 = 17 行，与下表一致。

---

**本段新元素**（每行一个，只列首次出现）：

- `ModuleRules` —— 所有 `*.Build.cs` 的基类，UBT 用它描述模块（`ModuleRules.cs:103`）
- `ReadOnlyTargetRules` —— 目标规则的**只读视图**，模块只能读不能改目标配置（`ReadOnlyTargetRules.cs:16`）
- `PCHUsageMode.UseExplicitOrSharedPCHs` —— PCH 策略枚举值，UE5 默认推荐（`BuildSettingsVersion` 相关）
- `PublicDependencyModuleNames` —— 公共依赖列表；会「传染」给依赖本模块的其他模块（`ModuleRules.cs`）
- `LyraEditor` —— 本模块名；必须与 `Source\LyraEditor\` 目录名、`LyraEditor.Target.cs` 里的 `ExtraModuleNames` 三者一致

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraEditor\LyraEditor.Build.cs`（**在 `Source\LyraEditor\` 目录下**） |
| 类名 | `public class LyraEditor : ModuleRules`，与文件名前缀一致 |
| 构造函数 | `public LyraEditor(ReadOnlyTargetRules Target) : base(Target)` |
| `PCHUsage` | `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;` |
| 依赖数 | **5 个**：`Core` / `CoreUObject` / `Engine` / `UnrealEd` / `LyraGame` |
| 尾逗号 | `"LyraGame"` 后面**没有**逗号（与源一致） |
| 注释 | 所有 `//` 开头的行**全部删掉**，一行都不留 |
| 缩进 | Tab（不是空格） |
| 代码行 | 剔注释后 **17 行**，与源逐字一致（源 28 行 − 顶部说明块 8 行 − 空行 3 行 = 17） |

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs:1-28`（占位版全文，891 B）
- `E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.Build.cs:18-26`（5 个公共依赖）
- `E:\ue5\LyraStarterGame\Source\LyraEditor\LyraEditor.Build.cs:1-60`（Lyra 正式版 1385 B，13 公共 + 12 私有，S22 才用）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs:103,1440`（`ModuleRules` 类与 `ModuleRules(ReadOnlyTargetRules)` 构造函数）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Programs\UnrealBuildTool\Configuration\ReadOnlyTargetRules.cs:16`（`ReadOnlyTargetRules` 类声明）