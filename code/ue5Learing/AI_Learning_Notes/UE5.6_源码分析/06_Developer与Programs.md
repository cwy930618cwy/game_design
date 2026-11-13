# 06 - Developer 与 Programs（构建工具链）

> 路径：`c:\Program Files\Epic Games\UE_5.6\Engine\Source\Developer\` + `Programs\`
> 这两个目录负责**编译、打包、调试**整个引擎和游戏。

---

## 一、Developer 模块（3,309 文件）

### 1.1 构建工具 ⭐⭐⭐

| 模块 | 作用 | 重要性 |
|------|------|--------|
| **UnrealBuildTool (UBT)** | 构建工具（编译/链接） | ⭐⭐⭐ |
| **UnrealHeaderTool (UHT)** | 头文件工具（反射代码生成） | ⭐⭐⭐ |
| **TargetPlatform** | 目标平台管理 | ⭐⭐ |
| **AutomationController** | 自动化控制器 | ⭐⭐ |
| **Commandlets** | 命令行小程序 | ⭐⭐ |

### 1.2 UBT — UnrealBuildTool

**核心职责**：
1. 解析 `.Target.cs` 和 `.Build.cs`
2. 决定哪些模块需要编译
3. 调用编译器（MSVC/Clang）
4. 链接生成 DLL/EXE

**关键概念**：

```csharp
// LyraGame.Target.cs 示例
public class LyraGameTarget : TargetRules
{
    public LyraGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderModules = true;
        ExtraModuleNames.Add("LyraGame");
    }
}
```

```csharp
// LyraGame.Build.cs 示例
public class LyraGame : ModuleRules
{
    public LyraGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore"
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "GameplayAbilities", "GameplayTags", "ModularGameplay"
        });
    }
}
```

**编译流程**：
```
1. 读取 *.Target.cs → 确定 Target（Editor/Game/Server）
2. 读取 *.Build.cs → 确定模块依赖
3. 生成 Makefile → 调用 UHT 生成反射代码
4. 调用编译器 → 生成 .obj
5. 链接 → 生成 .dll / .exe
```

### 1.3 UHT — UnrealHeaderTool

**核心职责**：
1. 扫描 `UCLASS/USTRUCT/UENUM/UPROPERTY/UFUNCTION` 宏
2. 生成反射代码（`.generated.h`）
3. 生成序列化/复制代码

**生成的文件**：
```
MyClass.h           ← 你写的头文件
MyClass.generated.h ← UHT 生成的反射代码
```

**为什么需要 UHT？**
- UE 的反射系统需要在编译期收集元信息
- UHT 读取宏 → 生成 C++ 代码 → 注册到引擎
- 这样运行时就能用 `FindPropertyByName` 等反射 API

### 1.4 TargetPlatform

管理不同平台的编译设置：

| 类 | 作用 |
|----|------|
| `ITargetPlatform` | 目标平台接口 |
| `FWindowsTargetPlatform` | Windows 平台 |
| `FLinuxTargetPlatform` | Linux 平台 |
| `FIOSPlatform` | iOS 平台 |
| `FAndroidPlatform` | Android 平台 |

### 1.5 Commandlets — 命令行小程序

| Commandlet | 作用 |
|------------|------|
| `Cook` | 烘焙资产 |
| `Stage` | 暂存文件 |
| `Package` | 打包 |
| `Archive` | 归档 |
| `ResavePackages` | 重新保存资产 |
| `FixupReferencers` | 修复引用 |
| `GenerateReports` | 生成报告 |

用法：
```bash
UE5Editor.exe MyProject -run=Cook -TargetPlatform=Windows
```

---

## 二、调试与分析工具

| 模块 | 作用 |
|------|------|
| `HotReload` | 热重载（编辑器运行时修改代码） |
| `LiveCoding` | 实时编码（更现代的热重载） |
| `Profiler` | 性能分析器 |
| `ProfilerMessages` | 分析消息 |
| `TraceAnalysis` | 追踪分析 |
| `TraceInsights` | 追踪洞察 |
| `TraceServices` | 追踪服务 |
| `Zen` | Zen 存储服务（DDC 后端） |
| `DerivedDataCache` | DDC 管理 |
| `DesktopPlatform` | 桌面平台工具 |
| `IoStoreUtilities` | IoStore 工具 |
| `OodleDataCompression` | Oodle 压缩 |

---

## 三、Programs 目录（独立程序）

### 3.1 核心程序

| 程序 | 作用 | 何时运行 |
|------|------|---------|
| **UnrealBuildTool** | 构建工具 | 每次编译 |
| **UnrealHeaderTool** | 头文件工具 | 每次编译 |
| **UnrealLightmass** | 光照烘焙 | 构建光照 |
| **ShaderCompileWorker** | 着色器编译 | 首次打开/切换平台 |
| **EpicWebHelper** | Web 辅助 | 编辑器内嵌浏览器 |
| **CrashReportClient** | 崩溃报告 | 崩溃时 |
| **UnrealPak** | Pak 打包 | 打包时 |
| **UnrealInsights** | 性能洞察 GUI | 分析性能 |
| **SwitchboardListener** | 虚拟制片切换板 | VP 场景 |

### 3.2 ShaderCompileWorker

**为什么需要？**
- 着色器编译非常慢（几分钟到几十分钟）
- 单独进程并行编译，不阻塞编辑器
- 支持分布式编译（多台机器）

**工作流程**：
```
1. 编辑器发现需要着色器
2. 提交任务给 ShaderCompileWorker
3. Worker 编译完成返回结果
4. 编辑器缓存结果（DDC）
```

### 3.3 UnrealInsights

**新一代性能分析工具**，替代旧的 Profiler：

| 功能 | 说明 |
|------|------|
| CPU 追踪 | 函数级耗时分析 |
| GPU 追踪 | 绘制调用分析 |
| 内存追踪 | 分配/泄漏检测 |
| 加载时间 | 资产加载分析 |
| Networking | 网络延迟分析 |

用法：
```bash
# 启动追踪
-trace=default,cpu,gpu,frame,bookmark

# 打开 Insights
UnrealInsights.exe MyTrace.utrace
```

---

## 四、构建流程全解

### 4.1 首次打开工程

```
1. 双击 .uproject
2. 检查引擎版本匹配
3. 生成项目文件（.sln / .xcodeproj）
4. 编译引擎（如果需要）
5. 编译项目代码
6. 打开编辑器
```

### 4.2 编译过程

```
1. UBT 读取 Target.cs
2. UBT 读取 Build.cs（递归解析依赖）
3. UHT 扫描头文件生成反射代码
4. 编译器编译每个 .cpp → .obj
5. 链接器合并 .obj → .dll/.exe
6. 复制到 Binaries/
```

### 4.3 Cook 过程

```
1. 加载所有资产
2. 转换为目标平台格式
3. 压缩（Oodle/Zlib）
4. 打包进 .pak 文件
5. 生成元数据
```

### 4.4 Package 过程

```
1. Cook 资产
2. Stage 文件（.exe/.dll/.pak）
3. 签名（可选）
4. 生成安装包
5. 生成补丁（可选）
```

---

## 五、常用命令行

```bash
# 生成项目文件
GenerateProjectFiles.bat

# 编译（Development Editor）
Build.bat LyraEditor Win64 Development

# 编译（Shipping Game）
Build.bat LyraGame Win64 Shipping

# Cook
RunUAT.bat Cook -project="path/to/Lyra.uproject" -platform=Win64

# Package
RunUAT.bat BuildCookRun -project="path/to/Lyra.uproject" -platform=Win64 -clientconfig=Shipping
```

---

## 六、学习建议

1. **理解 UBT/UHT** — 这是编译 UE 项目的核心
2. **看懂 Build.cs** — 模块依赖管理
3. **会用命令行** — 自动化构建
4. **了解 Cook/Package** — 发布游戏

## 七、下一步

- [03_源码目录全景](./03_源码目录全景.md) — 回到全景
- [07_Lyra中的实际应用](./07_Lyra中的实际应用.md) — Lyra 如何用这些模块
- [08_文件类型与宏速查](./08_文件类型与宏速查.md) — 速查手册
