# 05 - Editor 模块详解

> 路径：`c:\Program Files\Epic Games\UE_5.6\Engine\Source\Editor\`
> 规模：7,511 文件（只在编辑器下编译，不打进游戏包）

## 一、模块定位

Editor 模块提供**编辑器 UI 和资产工具**，与游戏运行时完全隔离。Lyra 的 `LyraEditor` 模块就是基于这套体系扩展的。

---

## 二、核心编辑器模块

### 2.1 UnrealEd — 编辑器核心（1,465 文件）⭐

**最重要的编辑器模块**，包含：

| 子系统 | 作用 |
|--------|------|
| `UEditorEngine` | 编辑器引擎 |
| `FEditorViewportClient` | 视口客户端 |
| `FLevelEditorViewportClient` | 关卡编辑器视口 |
| `FAssetThumbnailPool` | 资产缩略图池 |
| `FEditorTransaction` | 撤销/重做事务 |
| `FClipboard` | 剪贴板 |
| `FContentBrowserModule` | 内容浏览器集成 |

关键类：
```cpp
class UUnrealEdEngine : public UGameEngine;      // 编辑器引擎
class FEditorViewportClient : public FCommonViewportClient;  // 视口
class FAssetThumbnailPool;                        // 缩略图缓存
```

### 2.2 LevelEditor — 关卡编辑器（91 文件）

| 类 | 作用 |
|----|------|
| `FLevelEditorModule` | 关卡编辑器模块 |
| `FLevelEditorViewportClient` | 主视口 |
| `FLevelEditorActionContext` | 操作上下文 |
| `FLevelViewportLayoutEntity` | 视口布局 |

### 2.3 BlueprintGraph — 蓝图图表（303 文件）⭐

蓝图的**可视化节点系统**：

| 类 | 作用 |
|----|------|
| `UK2Node` | 蓝图节点基类 |
| `UK2Node_CallFunction` | 函数调用节点 |
| `UK2Node_VariableGet/Set` | 变量节点 |
| `UK2Node_Event` | 事件节点 |
| `UK2Node_Branch` | 分支节点 |
| `UK2Node_ForLoop` | 循环节点 |
| `FBlueprintEditorUtils` | 蓝图编辑工具 |

### 2.4 Kismet — 蓝图编辑器（167 文件）

旧称 Kismet，现在主要指蓝图编辑器的**UI 和交互**：

| 类 | 作用 |
|----|------|
| `FKismetEditorUtilities` | 蓝图编辑工具 |
| `FKismetCompilerContext` | 编译上下文 |
| `FKismetDebugger` | 蓝图调试器 |

### 2.5 KismetCompiler — 蓝图编译器（20 文件）

蓝图的**字节码编译器**：

| 类 | 作用 |
|----|------|
| `FKismetCompiler` | 编译器主体 |
| `FKismetCompilerContext` | 编译上下文 |
| `FKismetFunctionContext` | 函数上下文 |
| `FKismetTerm` | 表达式项 |

### 2.6 PropertyEditor — 属性面板（295 文件）

**Details 面板**的实现：

| 类 | 作用 |
|----|------|
| `IDetailsView` | 属性视图接口 |
| `FDetailsView` | 属性视图实现 |
| `IPropertyTypeCustomization` | 属性类型自定义 |
| `FPropertyValueImpl` | 属性值 |
| `FDetailLayoutBuilder` | 布局构建器 |

### 2.7 ContentBrowser — 内容浏览器（117 文件）

| 类 | 作用 |
|----|------|
| `FContentBrowserModule` | 内容浏览器模块 |
| `FAssetData` | 资产数据 |
| `FAssetRegistryModule` | 资产注册表 |
| `SContentBrowser` | 主控件 |

### 2.8 SceneOutliner — 场景大纲（87 文件）

**World Outliner** 的实现：

| 类 | 作用 |
|----|------|
| `FSceneOutlinerModule` | 大纲模块 |
| `SSceneOutliner` | 主控件 |
| `FSceneOutlinerTreeNode` | 树节点 |

### 2.9 Sequencer — 过场动画编辑器（394 文件）

| 类 | 作用 |
|----|------|
| `FSequencerModule` | Sequencer 模块 |
| `FSequencer` | 主编辑器 |
| `FSequencerTrackEditor` | 轨道编辑器 |
| `FMovieSceneTrackEditor` | 影片轨道编辑 |

### 2.10 UMGEditor — UMG 编辑器（233 文件）

Widget Blueprint 编辑器：

| 类 | 作用 |
|----|------|
| `FUMGEditorModule` | UMG 模块 |
| `FWidgetBlueprintEditor` | Widget 蓝图编辑器 |
| `FWidgetPaletteCategory` | 调色板分类 |
| `FWidgetTemplateClass` | Widget 模板 |

---

## 三、资产编辑器模块

| 模块 | 文件数 | 作用 |
|------|--------|------|
| `MaterialEditor` | 92 | 材质编辑器 |
| `Persona` | 327 | 骨骼网格编辑器（Animation Blueprint 也用） |
| `AnimationBlueprintEditor` | 55 | 动画蓝图编辑器 |
| `AnimGraph` | 317 | 动画图表 |
| `BehaviorTreeEditor` | 91 | 行为树编辑器 |
| `StaticMeshEditor` | 26 | 静态网格编辑器 |
| `SkeletalMeshEditor` | 17 | 骨骼网格编辑器 |
| `TextureEditor` | 21 | 纹理编辑器 |
| `FontEditor` | 15 | 字体编辑器 |
| `CurveEditor` | 150 | 曲线编辑器 |
| `DataTableEditor` | 14 | 数据表编辑器 |
| `WorldPartitionEditor` | 70 | 世界分区编辑器 |
| `LandscapeEditor` | 87 | 地形编辑器 |
| `FoliageEdit` | 49 | 植被编辑器 |
| `PhysicsAssetEditor` | 68 | 物理资产编辑器 |
| `SkeletonEditor` | 48 | 骨骼编辑器 |

---

## 四、工具类模块

| 模块 | 作用 |
|------|------|
| `DetailCustomizations` | 属性自定义显示（181 .h） |
| `ComponentVisualizers` | 组件可视化（如碰撞体显示） |
| `ClassViewer` | 类查看器 |
| `OutputLog` | 输出日志 |
| `SessionFrontend` | 会话前端 |
| `DeviceProfileEditor` | 设备配置编辑器 |
| `GameProjectGeneration` | 游戏项目生成向导 |
| `LocalizationDashboard` | 本地化仪表盘 |
| `StatsViewer` | 统计查看器 |
| `PixelInspector` | 像素检查器 |

---

## 五、编辑器扩展机制

### 5.1 模块依赖
```csharp
// 你的 Editor 模块的 Build.cs
PublicDependencyModuleNames.AddRange(new string[] {
    "UnrealEd",
    "EditorFramework",
    "BlueprintGraph",
    "Kismet",
    "PropertyEditor",
    "ContentBrowser",
    "Slate",
    "SlateCore"
});
```

### 5.2 扩展点

| 扩展方式 | 用途 |
|---------|------|
| `FExtender` | 扩展菜单/工具栏 |
| `FTabSpawnerEntry` | 注册新标签页 |
| `FNomadTabSpawnerEntry` | 注册独立窗口 |
| `IPropertyTypeCustomization` | 自定义属性显示 |
| `IDetailCustomization` | 自定义类详情 |
| `FAssetTypeActions_Base` | 自定义资产操作 |
| `UEditorUtilityWidget` | 编辑器工具控件 |

### 5.3 Lyra 中的应用

Lyra 的 `LyraEditor` 模块就用了这些扩展机制：
- **资产验证器** — 继承 `UDataValidator`
- **内容浏览器扩展** — 右键菜单
- **Actor Factory** — 快速创建 Actor

---

## 六、Slate UI 框架

Editor 模块大量使用 Slate：

| 类 | 作用 |
|----|------|
| `SWidget` | 控件基类 |
| `SCompoundWidget` | 复合控件 |
| `SLeafWidget` | 叶子控件 |
| `SBox` / `SVerticalBox` / `SHorizontalBox` | 布局容器 |
| `SButton` / `STextBlock` / `SEditableTextBox` | 基础控件 |
| `SListView` / `STreeView` / `SComboBox` | 列表控件 |
| `SDockTab` / `SDockStack` | 停靠标签 |

---

## 七、学习建议

1. **先看 UnrealEd** — 理解编辑器架构
2. **再看 BlueprintGraph + Kismet** — 理解蓝图系统
3. **按需看资产编辑器** — 根据需求深入
4. **参考 LyraEditor** — 看实际扩展案例

## 八、下一步

- [03_源码目录全景](./03_源码目录全景.md) — 回到全景
- [06_Developer与Programs](./06_Developer与Programs.md) — 构建工具链
- [07_Lyra中的实际应用](./07_Lyra中的实际应用.md) — Lyra 如何用这些模块
