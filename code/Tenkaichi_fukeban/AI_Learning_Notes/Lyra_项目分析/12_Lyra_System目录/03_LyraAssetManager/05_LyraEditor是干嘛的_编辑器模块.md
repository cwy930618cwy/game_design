# `LyraEditor` 是干嘛的？—— 编辑器扩展模块

> **定位**：讲清 `Source/LyraEditor` 这个模块的用途，以及它和 `LyraGame` 的关系（呼应"跨模块"理解）。
>
> **一句话**：`LyraEditor` 是**只在编辑器里用的工具模块**——装的是"给开发者用的辅助工具"（资源检查、内容校验、命令行批处理等），**不进游戏发布包**。它依赖 `LyraGame`（要调游戏代码），所以 `LyraGame` 里那些要被它调的函数才标了 `UE_API`。

---

## 一、先破题：编辑器模块是什么

UE 项目里通常有两类模块：

| 模块 | 用途 | 什么时候跑 | 进发布包吗 |
|------|------|-----------|-----------|
| **运行时模块**（如 `LyraGame`） | 游戏逻辑，玩家玩的游戏 | 游戏运行时（打包后） | ✅ 进 |
| **编辑器模块**（如 `LyraEditor`） | 给开发者的工具 | **只在编辑器里** | ❌ 不进 |

> **核心**：`LyraEditor` 是"**开发期的工具箱**"——玩家永远看不到它，只有开发者在 UE 编辑器里工作、或跑批处理脚本时才用。

---

## 二、LyraEditor 里都有啥（真实目录）

```
Source/LyraEditor/
├── LyraEditor.Build.cs          ← 模块定义（声明依赖 LyraGame）
├── LyraEditor.cpp/.h            ← 模块入口
├── LyraEditorEngine.cpp/.h      ← 自定义编辑器引擎（扩展编辑器行为）
│
├── Validation/                  ← 【核心】资源校验器
│   ├── EditorValidator.cpp/.h              ← 校验器基类
│   ├── EditorValidator_Load.cpp/.h         ← 检查资源能否加载
│   ├── EditorValidator_Blueprints.cpp/.h   ← 检查蓝图
│   ├── EditorValidator_MaterialFunctions.cpp/.h ← 检查材质函数
│   └── EditorValidator_SourceControl.cpp/.h ← 检查版本控制
│
├── Commandlets/                 ← 【核心】命令行工具（批处理）
│   └── ContentValidationCommandlet.cpp/.h  ← 批量校验内容
│
├── Utilities/                   ← 各种小工具
│   ├── CheckChaosMeshCollision.cpp         ← 检查物理碰撞
│   ├── CreateRedirectorPackage.cpp         ← 创建重定向包
│   └── DiffCollectionReferenceSupport.cpp  ← 比对资源引用
│
└── Private/                     ← 私有实现
```

**四大类东西**：校验器、命令行工具、编辑器引擎扩展、小工具。

---

## 三、逐类讲：它们干什么

### 3.1 Validation（资源校验器）—— 检查资源有没有问题

这是 LyraEditor 的**核心**。校验器 = "**资源质检员**"。

```cpp
// EditorValidator_Load.h —— 检查资源能不能正常加载
class UEditorValidator_Load : public UEditorValidator
{
    // 检查一个资源加载时有没有警告/错误
    virtual EDataValidationResult ValidateLoadedAsset_Implementation(...) override;
};
```

**作用**：当你**保存资源**或**提交代码**时，自动检查：
- 这个资源能正常加载吗？（`EditorValidator_Load`）
- 蓝图有没有错误？（`EditorValidator_Blueprints`）
- 材质函数有没有问题？（`EditorValidator_MaterialFunctions`）
- 版本控制状态对不对？（`EditorValidator_SourceControl`）

> **类比**：像工厂的"质检员"，产品（资源）出厂前自动检查一遍，有瑕疵就报警，不让问题资源进版本库。

### 3.2 Commandlets（命令行工具）—— 批处理

```cpp
// ContentValidationCommandlet.h —— 批量校验内容
class UContentValidationCommandlet : public UCommandlet
{
    virtual int32 Main(const FString& Params) override;  // 命令行入口
};
```

**作用**：可以在**命令行**（不打开编辑器界面）批量执行任务，比如：
- 一次性检查项目里所有资源
- 配合 CI/CD（持续集成）自动跑校验

> **Commandlet = 能命令行运行的批处理工具**。正常在编辑器里点按钮做的事，Commandlet 能命令行批量做（适合自动化流水线）。

### 3.3 LyraEditorEngine（编辑器引擎扩展）—— 改编辑器行为

```cpp
// LyraEditorEngine.h —— 自定义编辑器引擎
class ULyraEditorEngine : public UUnrealEdEngine
{
    virtual void Init(...) override;      // 编辑器初始化时
    virtual void Start() override;        // 编辑器启动时
    virtual void Tick(...) override;      // 编辑器每帧
    virtual FGameInstancePIEResult PreCreatePIEInstances(...) override;  // PIE 前
};
```

**作用**：重写编辑器引擎的行为——比如点"运行"（PIE）前做点特殊处理。

> **PIE** = Play In Editor，在编辑器里点"运行"测试游戏。这个类能在 PIE 前后插自定义逻辑。

### 3.4 Utilities（小工具）—— 零散辅助功能

- `CheckChaosMeshCollision`：检查 Chaos 物理网格碰撞
- `CreateRedirectorPackage`：创建资源重定向包（资源移动后留个"路标"）
- `DiffCollectionReferenceSupport`：比对资源集合引用

> 这些是一次性的辅助脚本，用不到就放着。

---

## 四、重点：LyraEditor ↔ LyraGame 的关系（呼应跨模块）

### 4.1 铁证：LyraEditor 依赖 LyraGame

`LyraEditor.Build.cs` 第 35 行：

```csharp
PublicDependencyModuleNames.AddRange(
    new string[] {
        "Core", "Engine", ...,
        "LyraGame",   // ← LyraEditor 声明：我要用 LyraGame 的代码
    }
);
```

**含义**：`LyraEditor`（编辑器工具）要调用 `LyraGame`（游戏代码）。

### 4.2 为什么编辑器工具要调游戏代码？

因为校验器要**检查游戏资源**，就得懂游戏的类型：

```
LyraEditor 的校验器要检查：
  - LyraGame 里定义的 ULyraGameData 资源对不对
  - LyraGame 的 ULyraAssetManager 加载逻辑
  - LyraGame 的角色、技能数据
        ↓
所以 LyraEditor 必须 #include 并调用 LyraGame 的类
        ↓
这就是"跨模块调用"！
```

### 4.3 所以 UE_API 的真正用途（闭环了）

```
LyraGame 里的 ULyraAssetManager::Get() 标了 UE_API
        ↓ 为什么？
因为 LyraEditor（别家厨房）校验时要调它
        ↓
LyraEditor 校验器代码：
  #include "System/LyraAssetManager.h"   ← 看菜单
  ULyraAssetManager::Get()               ← 跨模块真调用
        ↓ 因为 Get() 标了 UE_API（LyraGame 给了许可）
成功链接 ✅
```

> **现在完全闭环了**：`UE_API` 就是为了让 `LyraEditor` 这类**编辑器工具模块**能调用 `LyraGame` 的游戏代码。

---

## 五、一张图理清两个模块

```
【LyraGame 模块（LyraGame.dll）】—— 玩家玩的游戏
  System/LyraAssetManager     ← 标 UE_API 的函数（Get、GetGameData...）
  System/LyraGameData
  Character/LyraHealthComponent
  GameModes/LyraGameMode
       ↑ 内部互相调用：同模块，不用 UE_API
       │
       │ 提供"许可"（UE_API 导出）
       ↓
【LyraEditor 模块（LyraEditor.dll）】—— 开发者的工具（不进发布包）
  Validation/EditorValidator_Load    ← 检查 LyraGame 的资源
  Validation/EditorValidator_Blueprints
  Commandlets/ContentValidationCommandlet  ← 批量校验
  LyraEditorEngine                   ← 扩展编辑器
       ↓ 调 LyraGame 的类（跨模块）
       ↓ 靠 LyraGame 标了 UE_API 的函数才能链接成功
```

---

## 六、为什么编辑器代码要单独一个模块？

**为什么不把校验器塞进 LyraGame？** 因为：

1. **发布包不带编辑器工具**：玩家下载的游戏不需要"资源校验器"，单独模块能把它排除在发布包外，**减小游戏体积**。
2. **依赖方向清晰**：编辑器工具依赖游戏代码（`LyraEditor → LyraGame`），反过来不行（游戏不该依赖编辑器）。分开模块强制了这个正确方向。
3. **只在编辑器加载**：编辑器模块只在打开 UE 编辑器时加载，游戏运行时不加载，**省内存**。

> **类比**：`LyraGame` = 餐厅（给顾客吃饭）；`LyraEditor` = 餐厅的"后厨管理系统/卫生检查工具"（只给员工用，顾客看不到）。

---

## 七、JS 对照（帮你理解）

```js
// LyraGame —— 正式产品代码（会打包发布）
export class AssetManager { getGameData() { /* ... */ } }

// LyraEditor —— 开发工具（不打包发布，只在开发时用）
// 比如 webpack 的 devServer、eslint 配置这类
import { AssetManager } from './lyraGame.js';  // 工具要调产品代码
class Validator {
    validate() { AssetManager.getGameData(); /* 检查它 */ }
}
```

| 概念 | JS | UE |
|------|-----|-----|
| 产品代码 | `src/` 里会打包的 | `LyraGame`（进发布包） |
| 开发工具 | devServer / eslint（不打包） | `LyraEditor`（不进发布包） |
| 工具调产品 | `import` 产品代码 | 依赖 `LyraGame` + 调 UE_API 函数 |

> **关键差异**：JS 的"工具不打包"靠打包配置；UE 靠**分模块**——编辑器模块天然不进发布包。

---

## 八、常见疑问速答

| 疑问 | 答案 |
|------|------|
| LyraEditor 是干嘛的？ | 编辑器工具模块（资源校验、命令行批处理、编辑器扩展） |
| 它进游戏发布包吗？ | ❌ 不进，只给开发者用 |
| 为什么依赖 LyraGame？ | 校验器要检查游戏资源，得调游戏代码 |
| 和 UE_API 啥关系？ | LyraEditor 跨模块调 LyraGame，所以后者要标 UE_API |
| Validation 是啥？ | 资源校验器，保存/提交时自动检查资源有没有问题 |
| Commandlet 是啥？ | 能命令行运行的批处理工具（不打开编辑器界面） |
| PIE 是啥？ | Play In Editor，编辑器里点"运行"测试 |
| 为什么不把工具塞进 LyraGame？ | 发布包不带工具（省体积）、依赖方向清晰、只在编辑器加载 |

---

## 九、总结

```
LyraEditor = 编辑器工具模块（开发期用，不进发布包）

里面装什么：
  Validation/     → 资源校验器（检查资源能否加载/蓝图/材质/版本控制）
  Commandlets/    → 命令行批处理工具（批量校验）
  LyraEditorEngine → 扩展编辑器行为（PIE 前后逻辑）
  Utilities/      → 零散小工具

和 LyraGame 的关系：
  LyraEditor 依赖 LyraGame（Build.cs 第35行 "LyraGame"）
  校验器要检查游戏资源 → 得调游戏代码 → 跨模块调用
  所以 LyraGame 里被调的函数要标 UE_API

为什么单独模块：
  发布包不带工具（省体积）
  依赖方向清晰（编辑器→游戏，不能反）
  只在编辑器加载（省内存）
```

**一句话**：`LyraEditor` 是 Lyra 的**编辑器工具模块**——装资源校验器（Validation）、命令行批处理（Commandlets）、编辑器引擎扩展等**只给开发者用**的工具，**不进游戏发布包**；它依赖 `LyraGame`（校验器要检查游戏资源就得调游戏代码），这正是"跨模块调用"的真实例子，也是 `LyraGame` 里那些函数要标 `UE_API` 的原因。

---

## 十、下一步

- 看 `EditorValidator_Load` 具体怎么检查资源加载（`.cpp` 实现）。
- 看 `ContentValidationCommandlet` 怎么命令行批量校验。
- 理解 UE 的模块类型：Runtime（运行时）/ Editor（编辑器）/ Developer（开发者）工具。
- 看 `LyraGame.Build.cs` 对比，理解模块依赖怎么声明。
