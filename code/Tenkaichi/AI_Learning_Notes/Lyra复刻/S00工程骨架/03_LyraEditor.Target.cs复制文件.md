# 03 — `LyraEditor.Target.cs` 复制文件

> 依据：`E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs`（**24 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor.Target.cs`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。

---

```csharp
// 版权声明：Epic Games 保留所有权利

// 引入 UnrealBuildTool 命名空间；TargetRules / TargetInfo / TargetType 都在这里
using UnrealBuildTool;
// 引入泛型集合；本文件其实没直接用到，但 Lyra 原样保留
using System.Collections.Generic;

// 定义 LyraEditor 目标类；类名必须与文件名 LyraEditor.Target.cs 的前缀一致
public class LyraEditorTarget : TargetRules
{
	// 构造函数；UBT 创建目标时调用
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		// 目标类型 = 编辑器；含完整编辑器 UI、资产编辑器、蓝图编译器
		Type = TargetType.Editor;

		// 本目标要编译的模块列表；游戏模块 + 编辑器模块
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });

		// 如果不是「编译全部模块」模式
		if (!bBuildAllModules)
		{
			// 禁止「原生指针作为类成员」；强制用 TObjectPtr / TWeakObjectPtr 等智能指针
			// 这是 UE5 的 GC 安全要求，裸指针成员容易导致悬垂引用
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		// 应用 Lyra 所有目标共用的设置；方法定义在 LyraGame.Target.cs 里
		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		// 启用 RemoteSession 插件；配合手机 App「Unreal Remote 2」做触屏开发调试
		EnablePlugins.Add("RemoteSession");
	}
}
```

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraEditor.Target.cs`（在 `Source\` 根下） |
| 类名 | `LyraEditorTarget`，与文件名前缀一致 |
| 目标类型 | `Type = TargetType.Editor;` |
| 模块列表 | `ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });` |
| 指针限制 | `NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;` 在 `if (!bBuildAllModules)` 里 |
| 共用设置 | 调用了 `LyraGameTarget.ApplySharedLyraTargetSettings(this);` |
| 插件 | `EnablePlugins.Add("RemoteSession");` |
| 注释 | 所有 `//` 开头的行**全部删掉**，一行都不留 |
| 缩进 | Tab（不是空格） |

---

## 源码依据

- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:1-24`（全文）
- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:10`（`TargetType.Editor`）
- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:12`（两个模块）
- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:14-17`（指针成员限制）
- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:19`（调用共用设置）
- `E:\ue5\LyraStarterGame\Source\LyraEditor.Target.cs:22`（`RemoteSession`）