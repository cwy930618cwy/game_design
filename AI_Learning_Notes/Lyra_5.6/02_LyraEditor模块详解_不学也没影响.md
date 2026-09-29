# LyraEditor 模块详解 —— 不学它也不影响做游戏

> **定位**：快速搞清 `Source/LyraEditor/` 里到底有什么。**结论先放这：初学阶段完全可以跳过，不影响你做任何玩家看得到的游戏功能。**
>
> **关联**：[01_Lyra项目模块划分](./01_Lyra项目模块划分_LyraGame与LyraEditor.md)
>
> **一句话**：`LyraEditor` = 一堆"**只在编辑器里跑的开发期工具**"（资产校验、命令行批处理、编辑器菜单/样式）。玩家永远看不到，也进不了正式包。

---

## 一、先回答：不学它有影响吗？

**几乎没有影响。** 分情况看：

| 你的目标 | 需要 LyraEditor 吗 |
|---|---|
| 做玩家能玩到的功能（角色/武器/技能/UI） | ❌ 完全不需要，全在 LyraGame |
| 理解游戏运行逻辑 | ❌ 不需要 |
| 想给编辑器加个"一键校验资产"按钮 | ✅ 才需要 |
| 想做 CI/打包流水线的自动检查脚本 | ✅ 才需要（Commandlet） |
| 想自己定制编辑器菜单/工具栏 | ✅ 才需要 |

> **学习建议**：前期 99% 精力放 `LyraGame/`。`LyraEditor/` 等你项目成熟、需要"开发期提效工具"时再回来看，半小时就能看懂。

---

## 二、目录全景（真实结构）

```
LyraEditor/
├── Commandlets/          ← 命令行工具（不开编辑器就能跑的批处理）
│   └── ContentValidationCommandlet.h/.cpp
│
├── Validation/           ← 【最大块，10 个文件】资产校验器
│   ├── EditorValidator.h/.cpp                    ← 校验基类
│   ├── EditorValidator_Blueprints.h/.cpp         ← 校验蓝图
│   ├── EditorValidator_Load.h/.cpp               ← 校验资源加载
│   ├── EditorValidator_MaterialFunctions.h/.cpp  ← 校验材质函数
│   └── EditorValidator_SourceControl.h/.cpp      ← 校验源码控制(版本管理)
│
├── Private/              ← 编辑器私有小工具
│   ├── AssetTypeActions_LyraContextEffectsLibrary.h/.cpp  ← 资产类型右键菜单
│   ├── GameEditorStyle.h/.cpp                            ← 编辑器样式
│   └── LyraContextEffectsLibraryFactory.h/.cpp           ← 资产工厂(双击创建)
│
├── Utilities/            ← 杂项小工具
│
├── LyraEditor.cpp/.h     ← 模块入口
└── LyraEditorEngine.cpp/.h  ← 自定义编辑器引擎(PIE 启动钩子)
```

---

## 三、逐个说清楚里面是啥

### ① Validation/ —— 资产校验器（核心，占大半）

**作用**：美术/策划提交资源前，自动检查"合不合规"，把错误拦在编辑器里，别等打包才发现问题。

看真实源码 `EditorValidator.h`，它继承引擎的 `UEditorValidatorBase`，提供：

```cpp
// 校验一批包（资源），收集所有 Warning/Error
static bool ValidatePackages(..., TArray<FString>& OutAllWarningsAndErrors, ...);
// 校验项目设置
static bool ValidateProjectSettings();
// 判断是不是在未 cook 的目录里（测试地图等特殊处理）
static bool IsInUncookedFolder(const FString& PackageName, ...);
```

里面有个巧妙的工具类 `FLyraValidationMessageGatherer`——**拦截引擎日志**，把所有 Warning/Error 收集起来统一报告：

```cpp
// 重写 Serialize，把日志里的警告/错误抓出来存好
virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, ...) override {
    if (Verbosity <= ELogVerbosity::Warning) {
        AllWarningsAndErrors.Add(MessageString);  // 收集起来
    }
}
```

**具体 5 个校验器**：

| 校验器 | 查什么 |
|---|---|
| `EditorValidator_Blueprints` | 蓝图有没有编译错误、引用断裂 |
| `EditorValidator_Load` | 资源能不能正常加载 |
| `EditorValidator_MaterialFunctions` | 材质函数用得对不对 |
| `EditorValidator_SourceControl` | 版本管理（Perforce）相关的合规性 |
| `EditorValidator`（基类） | 通用校验逻辑 + 项目设置校验 |

> **场景**：团队里美术提交贴图前，这些校验器自动跑一遍，命名不对/尺寸不符就报错拦截。

---

### ② Commandlets/ —— 命令行工具

**作用**：**不用打开编辑器 GUI**，纯命令行就能跑的批处理任务。主要给 CI（持续集成）流水线用。

看真实源码 `ContentValidationCommandlet.h`：

```cpp
UCLASS()
class UContentValidationCommandlet : public UCommandlet {
    virtual int32 Main(const FString& Params) override;  // 命令行入口
private:
    bool AutoExportMCPTemplates(...);   // 自动导出模板
    bool AutoExportDadContent(...);     // 自动导出内容
    TArray<FString> GetAllChangedFiles(...);  // 拿 Perforce 改动的文件
    bool LaunchP4(const FString& Args, ...);  // 调 Perforce 命令
};
```

> **场景**：每次有人提交代码，CI 机器上跑一句命令 `UE5.exe MyProject -run=ContentValidation`，自动检查这次改动有没有破坏资源。**全程无界面，适合服务器跑。**

---

### ③ Private/ —— 编辑器小工具

| 文件 | 作用 |
|---|---|
| `AssetTypeActions_LyraContextEffectsLibrary` | 给特定资产加**右键菜单**（比如"用这个资产创建XXX"） |
| `LyraContextEffectsLibraryFactory` | **资产工厂**——内容浏览器里"右键→创建"时用的那个 |
| `GameEditorStyle` | 自定义**编辑器外观/样式**（图标、颜色等） |

> 这些都是"让编辑器更好用"的小点缀，和游戏运行无关。

---

### ④ LyraEditorEngine —— 自定义编辑器引擎

**作用**：接管编辑器的启动流程，尤其是 **PIE（Play In Editor，编辑器内试玩）** 的启动钩子。

看真实源码 `LyraEditorEngine.h`：

```cpp
UCLASS()
class ULyraEditorEngine : public UUnrealEdEngine {  // 继承引擎的编辑器引擎
    virtual void Init(IEngineLoop*) override;       // 编辑器初始化
    virtual void Start() override;                  // 编辑器启动
    virtual void Tick(float, bool) override;        // 每帧
    // PIE 实例创建前的钩子（Lyra 用它做在线多人试玩的特殊设置）
    virtual FGameInstancePIEResult PreCreatePIEInstances(...) override;
};
```

> **场景**：你在编辑器点"Play"试玩多人模式时，Lyra 需要预先配置一些在线参数——这个类就是干这个的。**只影响"编辑器里试玩"，不影响正式包。**

---

## 四、一张图看懂它们和"游戏运行"的关系

```
┌─────────────────────────────────────────────┐
│              玩家手里的正式包                 │
│   ┌───────────────────────────────────────┐ │
│   │  LyraGame（运行时）                     │ │
│   │  角色/武器/技能/UI/输入...  ← 只有这些  │ │
│   └───────────────────────────────────────┘ │
└─────────────────────────────────────────────┘
           ↑ 编译时参与 / 打进包

╔═════════════════════════════════════════════╗
║  LyraEditor（仅编辑器）—— 玩家永远拿不到      ║
║  ├─ Validation  提交前查错                    ║
║  ├─ Commandlets 命令行批处理                  ║
║  ├─ Private     右键菜单/工厂/样式            ║
║  └─ EditorEngine PIE 试玩钩子                 ║
╚═════════════════════════════════════════════╝
           ↑ 只在开发者编辑器里跑
```

---

## 五、常见误区

| 误区 | 正确理解 |
|------|---------|
| "不学 LyraEditor 就做不了游戏" | ❌ 玩家向功能全在 LyraGame |
| "LyraEditor 会被打进玩家包" | ❌ 打包目标不含它 |
| "Validation 是游戏里的反作弊" | ❌ 是开发期的资产合规检查 |
| "Commandlet 是游戏启动命令" | ❌ 是不开编辑器的命令行批处理 |
| "EditorEngine 是游戏的引擎" | ❌ 只是编辑器(PIE)专用钩子 |

---

## 六、总结速查

```
LyraEditor = 开发期工具集合（玩家看不到，进不了包）
  ├─ Validation/   资产校验（提交前查错，最大块）
  ├─ Commandlets/  命令行批处理（CI 用，无界面）
  ├─ Private/      编辑器小工具（右键菜单/工厂/样式）
  └─ EditorEngine  PIE 试玩启动钩子

什么时候看它：
  ✗ 做游戏功能 → 不用看
  ✓ 要写资产校验/编辑器工具/CI 脚本 → 才看

关系：依赖 LyraGame，反之不行
```

**一句话**：`LyraEditor` 全是"**开发期提效工具**"——资产校验（Validation）、命令行批处理（Commandlets）、编辑器右键菜单/样式（Private）、试玩钩子（EditorEngine）。**玩家永远拿不到，初学完全可以跳过，不影响做任何游戏功能。**

---

## 七、下一步

- 主力深入 `LyraGame/` 内部子系统（Character/AbilitySystem/Weapons...）
- 等项目成熟需要"自动校验资产"时，再回来细看 Validation/
