# -*- coding: utf-8 -*-
import io, json

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'
MD      = r'E:\ue5\game_design\code\ue5Learing\AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\答疑文件\13_CoreGlobals.h逐行注释版.md'

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    eng = f.read().split('\n')
with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')
with io.open(r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\emap_cg.json', 'r', encoding='utf-8') as f:
    emap = {int(k): v for k, v in json.load(f).items()}



segments = []
segments.append(('一、版权头 + include + 前置声明', 1, 27,
 ['`#pragma once` —— 头文件保护',
  '`Containers/UnrealString.h` —— `FString`（各种 ini 路径）',
  '`HAL/PlatformTLS.h` —— `thread_local` 支持',
  '`Logging/LogMacros.h` —— `DECLARE_LOG_CATEGORY_EXTERN`',
  '`Misc/Build.h` —— `WITH_ENGINE` / `WITH_EDITOR` / `UE_BUILD_SHIPPING`',
  '`ProfilingDebugging/CpuProfilerTrace.h` —— `TRACE_CPUPROFILER_EVENT_SCOPE_STR`',
  '`Templates/Atomic.h` —— `TSAN_ATOMIC` 宏',
  '`UObject/NameTypes.h` —— `FName` / `FLazyName`',
  '`<atomic>` —— `std::atomic<bool>`',
  '`class Error;` 等 11 个 —— 前置声明，本文件只用到指针'],
 [('第 2 行', '头文件保护'),
  ('第 4~13 行', '10 个 Core 内部 include'),
  ('第 15 行', '标准库 `<atomic>`'),
  ('第 17~27 行', '11 个前置声明')],
 '像出门前列清单：要用的工具先点名，具体长什么样后面再说。'))

segments.append(('二、日志分类声明（24 个）', 29, 65,
 ['`DECLARE_LOG_CATEGORY_EXTERN(名字, 默认级别, 编译期最高级别)` —— 声明一个全局日志分类',
  '`Log` / `Warning` / `Display` —— 默认详细级别',
  '`All` —— 编译期允许的最高级别',
  '`DECLARE_LOG_CATEGORY_EXTERN_HELPER(A,B,C)` —— 辅助宏，让「宏」能当参数传给「宏」',
  '`PLATFORM_GLOBAL_LOG_CATEGORY` —— 平台自定义的全局日志分类名'],
 [('第 29~39 行', '11 个基础日志分类'),
  ('第 41~52 行', '13 个系统日志分类'),
  ('第 55 行', '`LogTemp` 临时调试分类'),
  ('第 59 行', '辅助宏定义'),
  ('第 60~65 行', '平台专属分类（条件编译）')],
 '像给每个部门装一个喇叭：谁要喊话就用自己那个，喊多大声（级别）也定好了。'))

segments.append(('三、日志单例 + 启动耗时点', 68, 72,
 ['`GetGlobalLogSingleton()` —— 取全局日志重定向器单例',
  '`BootTimingPoint(Message)` —— 打一个启动耗时点',
  '`DumpBootTiming()` —— 汇总输出所有耗时点'],
 [('第 68 行', '取日志单例'),
  ('第 70 行', '打耗时点'),
  ('第 72 行', '输出耗时汇总')],
 '像跑步时按秒表：每到一个检查点按一下，最后一起看成绩。'))

segments.append(('四、`FScopedBootTiming`（启动耗时作用域）', 74, 81,
 ['`FScopedBootTiming` —— 构造时记开始时间，析构时自动上报耗时',
  '`FString Message` —— 阶段名',
  '`double StartTime` —— 开始时间（秒）'],
 [('第 76~77 行', '两个成员'),
  ('第 78~79 行', '两个构造函数（带/不带后缀）'),
  ('第 80 行', '析构函数自动上报')],
 '像自动秒表：进作用域开始计时，出作用域自动停表并记录。'))

segments.append(('五、`FEngineTrackedActivityScope`（Insights 活动作用域）', 83, 88,
 ['`FEngineTrackedActivityScope` —— 把一段代码标记成 Unreal Insights 里可见的活动',
  '`const TCHAR* Fmt, ...` —— printf 风格格式化',
  '`const FString& Str` —— 直接传字符串'],
 [('第 85 行', 'printf 风格构造'),
  ('第 86 行', 'FString 构造'),
  ('第 87 行', '析构结束活动')],
 '像给一段代码贴时间轴标签，事后在 Insights 里能看到它占多宽。'))

segments.append(('六、两个作用域宏', 91, 92,
 ['`SCOPED_BOOT_TIMING(x)` —— 同时打 CPU 追踪事件 + 建启动耗时作用域',
  '`UE_SCOPED_ENGINE_ACTIVITY(Fmt, ...)` —— 建引擎活动作用域',
  '`ANONYMOUS_VARIABLE(前缀)` —— 生成唯一变量名，避免同一作用域重名',
  '`## __VA_ARGS__` —— 可变参数透传（GCC 扩展，空参数时去掉前面的逗号）'],
 [('第 91 行', '启动耗时宏'),
  ('第 92 行', '引擎活动宏')],
 '像自动命名：你只管写宏，变量名它自己起，不会撞车。'))

segments.append(('七、核心全局量', 95, 106,
 ['`GLog` —— 宏，等价于 `GetGlobalLogSingleton()`',
  '`GConfig` —— 全局配置缓存（读写 ini）',
  '`GUndo` —— 全局撤销事务',
  '`GLogConsole` —— 全局控制台输出设备',
  '`GError` —— 全局错误输出设备（弹错误框）',
  '`GWarn` —— 全局警告/反馈上下文（弹对话框）',
  '`GErrorHist[16384]` —— 错误历史缓冲区',
  '`GErrorExceptionDescription[4096]` —— 异常描述缓冲区'],
 [('第 95 行', '`GLog` 宏'),
  ('第 96~100 行', '5 个核心全局指针'),
  ('第 103 行', '错误历史缓冲'),
  ('第 106 行', '异常描述缓冲')],
 '像值班室的五部电话：配置、撤销、控制台、报错、警告，各管一摊。'))

segments.append(('八、`FCoreTexts`（本地化文本）', 108, 129,
 ['`FCoreTexts` —— 引擎内置的 True/False/Yes/No/None 本地化文本',
  '`const FText&` —— 引用成员，指向全局实例里的文本',
  '`static const FCoreTexts& Get()` —— 取全局唯一实例',
  '`static void TearDown()` —— 销毁实例（之后引用全失效）',
  '`= delete` —— 删除拷贝构造/赋值，禁止拷贝'],
 [('第 110~114 行', '5 个 FText 引用'),
  ('第 116 行', '取全局实例'),
  ('第 119 行', '销毁实例'),
  ('第 122~123 行', '禁拷贝'),
  ('第 125~128 行', '构造函数')],
 '像一本多语言小词典：True/False/Yes/No/None 五个词，按当前语言给。'))

segments.append(('九、旧版全局文本（已废弃）', 131, 142,
 ['`UE_DEPRECATED(版本, "提示")` —— 标记已废弃，编译时给警告',
  '`DISABLE_LEGACY_CORE_TEXTS` —— 关掉旧全局文本的开关',
  '`GTrue` / `GFalse` / `GYes` / `GNo` / `GNone` —— 5 个旧版全局文本'],
 [('第 131 行', '条件编译'),
  ('第 132~141 行', '5 个废弃变量 + 5 个废弃标记'),
  ('第 142 行', '条件编译结束')],
 '像老式电话亭：还在，但门口贴了「请用手机」的告示。'))

segments.append(('十、蓝图/对象/嵌入相关开关', 144, 194,
 ['`GIsGameAgnosticExe` —— 是否「游戏无关」的可执行文件',
  '`GForceLoadEditorOnly` —— 强制加载仅编辑器属性',
  '`GVerifyObjectReferencesOnly` —— 只校验对象引用',
  '`GFastPathUniqueNameGeneration` —— 唯一名生成走快速路径',
  '`GAllowActorScriptExecutionInEditor` —— 允许编辑器内执行 Actor 脚本',
  '`GCompilingBlueprint` —— 是否正在编译蓝图',
  '`GIsGCingAfterBlueprintCompile` —— 蓝图编译后正在 GC',
  '`GIsReconstructingBlueprintInstances` —— 正在重建蓝图实例',
  '`std::atomic<bool> GIsReinstancing` —— 正在重新实例化（原子）',
  '`GFlushStreamingFunc` —— 冲刷流式加载的函数指针',
  '`FUELibraryOverrideSettings` —— UE-as-a-library 嵌入设置',
  '`GUELibraryOverrideSettings` —— 全局嵌入设置实例',
  '`GIsRunningUnattendedScript` —— 是否在跑无人值守脚本'],
 [('第 145~172 行', '9 个蓝图/对象开关 + 1 个函数指针'),
  ('第 175~189 行', '`FUELibraryOverrideSettings` 结构（4 字段）'),
  ('第 192 行', '全局嵌入设置'),
  ('第 194 行', '无人值守脚本标志')],
 '像一排电闸：编译蓝图、重建实例、重新实例化，各有一个开关。'))

segments.append(('十一、commandlet 私有标志', 196, 205,
 ['`WITH_ENGINE` —— 是否编了引擎',
  '`PRIVATE_GIsRunningCommandlet` —— 是否在跑 commandlet',
  '`PRIVATE_GRunningCommandletClass` —— 正在跑的 commandlet 的 UClass',
  '`PRIVATE_GAllowCommandletRendering` —— commandlet 下允许渲染',
  '`PRIVATE_GAllowCommandletAudio` —— commandlet 下允许音频',
  '`PRIVATE_` 前缀 —— 表示「内部用，外部请走 IsRunningCommandlet()」'],
 [('第 197~198 行', '2 个基础标志'),
  ('第 201 行', '渲染开关'),
  ('第 204 行', '音频开关')],
 '像后台开关箱：标着 PRIVATE，请从前面板（函数）操作。'))

segments.append(('十二、`GIsEditor` 三态定义', 207, 237,
 ['`WITH_EDITORONLY_DATA` —— 是否编了仅编辑器数据',
  '`USING_CODE_ANALYSIS` —— 是否在做静态代码分析',
  '`GIsEditor` —— 是否在编辑器里（三种定义方式）',
  '`GIntraFrameDebuggingGameThread` —— 游戏线程是否被暂停做帧内调试',
  '`GFirstFrameIntraFrameDebugging` —— 是否首帧帧内调试',
  '`#define GIsEditor false` —— 运行时直接定义成常量，零开销'],
 [('第 213~222 行', '编辑器构建：真实变量'),
  ('第 227~229 行', '代码分析：变量形式（避免警告）'),
  ('第 233~235 行', '运行时：宏定义成 false')],
 '像同一个开关三种装法：编辑器里是真开关，分析时是假开关，运行时干脆焊死。'))

segments.append(('十三、commandlet 判断函数（6 个）', 239, 325,
 ['`FORCEINLINE` —— 强制内联，零调用开销',
  '`IsRunningCommandlet()` —— 是否在跑 commandlet',
  '`IsRunningCookCommandlet()` —— 是否在跑 cook commandlet',
  '`IsRunningDLCCookCommandlet()` —— 是否在跑 DLC cook commandlet',
  '`GetRunningCommandletClass()` —— 取当前 commandlet 的 UClass',
  '`GetCommandletNameFromCmdline()` —— 从命令行取 commandlet 名字',
  '`IsAllowCommandletRendering()` —— commandlet 下是否允许渲染',
  '`IsAllowCommandletAudio()` —— commandlet 下是否允许音频'],
 [('第 247~254 行', '`IsRunningCommandlet`'),
  ('第 259~266 行', '`IsRunningCookCommandlet`'),
  ('第 271~278 行', '`IsRunningDLCCookCommandlet`'),
  ('第 283~290 行', '`GetRunningCommandletClass`'),
  ('第 304 行', '`GetCommandletNameFromCmdline`'),
  ('第 309~325 行', '`IsAllowCommandletRendering` / `IsAllowCommandletAudio`')],
 '像前台查询台：六个窗口，问「现在是不是在跑 XX」都能答。'))

segments.append(('十四、多进程 ID', 327, 342,
 ['`namespace UE` —— 引擎新版公共 API 命名空间',
  '`UE::GetMultiprocessId()` —— 取当前进程的多进程编号',
  '`namespace UE::Private` —— 内部实现细节',
  '`UE::Private::SetMultiprocessId()` —— 设置编号（内部用）'],
 [('第 336 行', '公开 getter'),
  ('第 341 行', '内部 setter')],
 '像给每个工人发工牌号：导演是 0 号，工人是 1、2、3…，单干时也是 0 号。'))

segments.append(('十五、重新实例化包装类 + 客户端/服务器标志', 344, 356,
 ['`FIsDuplicatingClassForReinstancing` —— 可隐式转 bool 的包装类',
  '`operator=` —— 赋值运算符，接受 bool',
  '`operator bool()` —— 类型转换运算符',
  '`GEdSelectionLock` —— 编辑器选择是否锁定',
  '`GIsClient` / `GIsServer` —— 客户端 / 服务器标志',
  '`TSAN_ATOMIC(bool)` —— ThreadSanitizer 友好的原子布尔'],
 [('第 347~348 行', '两个运算符重载'),
  ('第 351~353 行', '3 个普通 bool'),
  ('第 354~356 行', '3 个原子/包装类型')],
 '像带锁的开关：包装类让你能像用 bool 一样用它，但内部可以加逻辑。'))

segments.append(('十六、启动/退出/截图相关', 358, 427,
 ['`GIsBuildMachine` —— 是否构建机进程',
  '`GIsSilent` —— 静默模式（不输出日志）',
  '`GIsSlowTask` / `GSlowTaskOccurred` / `GIsGuarded` —— 慢任务/异常保护',
  '`UE_SET_REQUEST_EXIT_ON_TICK_ONLY` —— 只在 tick 开始允许请求退出',
  '`IsEngineStartupModuleLoadingComplete()` —— 启动模块加载是否完成',
  '`GIsRequestingExit` —— 是否已请求退出（已废弃）',
  '`IsEngineExitRequested()` —— 是否已请求退出（新 API）',
  '`RequestEngineExit(ReasonString)` —— 请求退出',
  '`GAreScreenMessagesEnabled` —— 屏幕消息是否启用',
  '`GIsDumpingMovie` / `GIsHighResScreenshot` —— 导出序列帧 / 高清截图',
  '`GScreenshotResolutionX/Y` —— 截图分辨率',
  '`GMakeCacheIDIndex` —— 缓存 ID 计数器'],
 [('第 365~374 行', '启动状态标志'),
  ('第 382~384 行', '退出时机开关'),
  ('第 390~391 行', '模块加载完成'),
  ('第 394~411 行', '退出请求（新旧 API）'),
  ('第 419~427 行', '屏幕消息 + 截图')],
 '像飞机的仪表盘：启动、退出、截图，每个状态一个灯。'))

segments.append(('十七、各种 ini 路径', 429, 457,
 ['`GEngineIni` —— 引擎主 ini',
  '`GEditorLayoutIni` / `GEditorKeyBindingsIni` / `GEditorSettingsIni` —— 编辑器布局/快捷键/设置（按引擎版本存）',
  '`GEditorIni` / `GEditorPerProjectIni` —— 编辑器项目 ini（按项目存）',
  '`GCompatIni` / `GLightmassIni` / `GScalabilityIni` / `GHardwareIni` —— 兼容/光照/画质/硬件',
  '`GInputIni` / `GGameIni` / `GGameUserSettingsIni` —— 输入/游戏/玩家设置',
  '`GRuntimeOptionsIni` / `GInstallBundleIni` / `GDeviceProfilesIni` / `GGameplayTagsIni` —— 运行时/安装包/设备档/标签',
  '`GNearClippingPlane` —— 近裁剪面',
  '`GExitPurge` —— 退出时彻底清理',
  '`GInternalProjectName[64]` —— 内部项目名',
  '`GForeignEngineDir` —— 外部引擎目录'],
 [('第 429 行', '引擎主 ini'),
  ('第 432~438 行', '编辑器 ini（6 个）'),
  ('第 440~450 行', '系统 ini（11 个）'),
  ('第 452~457 行', '裁剪面 / 退出清理 / 项目名 / 外部引擎')],
 '像一柜子文件夹：每个系统一个 ini，找配置先看名字。'))

segments.append(('十八、异步加载 + 加载器类型', 459, 494,
 ['`GDebugToolExec` —— 调试工具 exec 处理器',
  '`IsAsyncLoading` —— 函数指针，判断是否在异步加载',
  '`SuspendAsyncLoading` / `ResumeAsyncLoading` —— 暂停/恢复异步加载',
  '`IsAsyncLoadingSuspended` —— 查询是否暂停',
  '`IsAsyncLoadingMultithreaded` —— 是否多线程异步加载',
  '`enum class ELoaderType : uint8` —— 加载器类型（4 个值）',
  '`LexToString(ELoaderType)` —— 枚举转字符串',
  '`GetLoaderType` —— 函数指针，取当前加载器',
  '`SuspendTextureStreamingRenderTasks` / `ResumeTextureStreamingRenderTasks` —— 暂停/恢复纹理流式任务'],
 [('第 460 行', '调试 exec'),
  ('第 463~475 行', '5 个异步加载函数指针'),
  ('第 477~483 行', '`ELoaderType` 枚举'),
  ('第 485~488 行', '枚举转字符串 + 取当前加载器'),
  ('第 491~494 行', '纹理流式任务开关')],
 '像换轮胎：旧加载器、编辑器加载器、Zen 加载器，随时能换，换的时候先暂停。'))

segments.append(('十九、编辑器加载包 / PIE 实例号', 496, 538,
 ['`FIsEditorLoadingPackage` —— 已废弃（5.5）的 bool 包装类',
  '`GIsEditorLoadingPackage` —— 编辑器是否正在加载包（已废弃）',
  '`UE::GetIsEditorLoadingPackage()` / `SetIsEditorLoadingPackage()` —— 新 API',
  '`GIsCookerLoadingPackage` —— cooker 是否正在加载包',
  '`GIsPlayInEditorWorld` —— GWorld 是否指向 PIE 世界',
  '`FPlayInEditorID` —— 已废弃（5.5）的 int32 包装类',
  '`GPlayInEditorID` —— 当前 PIE 实例号（已废弃）',
  '`UE::GetPlayInEditorID()` / `SetPlayInEditorID()` —— 新 API',
  '`GIsPIEUsingPlayerStart` —— PIE 是否用 PlayerStart（已废弃）'],
 [('第 496~501 行', '`FIsEditorLoadingPackage` 包装类'),
  ('第 506 行', '废弃的全局标志'),
  ('第 511~512 行', '新 API'),
  ('第 516~519 行', 'cooker / PIE 世界标志'),
  ('第 521~537 行', '`FPlayInEditorID` + 新旧 API'),
  ('第 542 行', '废弃的 PlayerStart 标志')],
 '像换门锁：旧钥匙（包装类）还能用但已贴封条，新钥匙（UE:: 函数）才是正路。'))

segments.append(('二十、帧号 / 线程 ID / 日志开关 / 统计', 540, 654,
 ['`GPlatformNeedsPowerOfTwoTextures` —— 平台是否要求 2 的幂纹理',
  '`GStartTime` —— 引擎启动时间（秒）',
  '`GSystemStartTime` —— 系统启动时间字符串',
  '`GIsInitialLoad` —— 是否初始加载中',
  '`GEventDrivenLoaderEnabled` —— 事件驱动加载器是否启用',
  '`GIsRetrievingVTablePtr` —— 是否正在取 VTablePtr',
  '`GFrameCounter` / `GFrameCounterRenderThread` —— 帧计数器（64 位）',
  '`GLastGCFrame` —— 上次 GC 的帧号',
  '`GInputTime` —— 输入采样时间（CPU 周期）',
  '`GFrameNumber` / `GFrameNumberRenderThread` —— 帧号（32 位）',
  '`GIsFirstInstance` —— 是否首个实例（已废弃）',
  '`GHitchThresholdMS` —— 卡顿阈值（毫秒）',
  '`GSavingCompressionChunkSize` —— 压缩分块大小',
  '`GGameThreadId` / `GRenderThreadId` / `GSlateLoadingThreadId` —— 线程 ID',
  '`GIsGameThreadIdInitialized` —— 游戏线程 ID 是否已初始化',
  '`GCurrentTraceName` —— 当前 trace 名',
  '`GPrintLogTimes` / `GPrintLogCategory` / `GPrintLogVerbosity` —— 日志打印开关',
  '`GHitchDetected` —— 是否检测到卡顿',
  '`GCycleStatsShouldEmitNamedEvents` / `GShouldEmitVerboseNamedEvents` —— 统计命名事件',
  '`GIsDemoMode` —— 演示模式',
  '`GLongCorePackageName` / `GLongCoreUObjectPackageName` —— 包长名',
  '`GIsAutomationTesting` —— 是否自动化测试',
  '`GPumpingMessagesOutsideOfMainLoop` / `GPumpingMessages` —— 消息泵',
  '`GEnableVREditorHacks` —— VR 编辑器 hack',
  '`GIgnoreDebugger` —— 是否忽略调试器'],
 [('第 545~560 行', '平台/启动/加载标志'),
  ('第 563~577 行', '帧计数器与帧号'),
  ('第 581~600 行', '实例/卡顿/压缩/线程 ID'),
  ('第 603~612 行', 'trace 名 + 日志打印开关'),
  ('第 616~626 行', '卡顿检测 + 统计命名事件'),
  ('第 629~647 行', '演示模式 / 包名 / 测试 / 消息泵 / VR hack'),
  ('第 652 行', '忽略调试器（非 Shipping）')],
 '像仪表盘第二排：帧率、线程、日志、统计，全是运行时读数。'))

segments.append(('二十一、`ETaskTag`（线程标签位标志）', 656, 676,
 ['`enum class ETaskTag : int32` —— 执行上下文标签',
  '`ENone` / `EStaticInit` / `EGameThread` / `ESlateThread` —— 基础标签',
  '`EAudioThread` —— 已废弃（5.0）',
  '`ERenderingThread` / `ERhiThread` / `EAsyncLoadingThread` / `EEventThread` —— 其余线程',
  '`ENamedThreadBits` —— 所有具名线程位的掩码',
  '`EParallelThread = 1 << 30` —— 并行标记位',
  '`EWorkerThread` / `EParallelRenderingThread` / `EParallelGameThread` / `EParallelRhiThread` —— 并行组合',
  '`ENUM_CLASS_FLAGS(ETaskTag)` —— 生成位运算'],
 [('第 658~666 行', '8 个基础标签（含 1 个废弃）'),
  ('第 668 行', '具名线程掩码'),
  ('第 669~673 行', '并行标记 + 4 个并行组合'),
  ('第 676 行', '生成位运算')],
 '像给每个工人戴不同颜色的帽子：一眼看出他在哪个车间。'))

segments.append(('二十二、`FTaskTagScope`（标签作用域）', 679, 735,
 ['`FTaskTagScope` —— 构造时打标签、析构时恢复',
  '`friend class FRunnableThread` / `FRenderingThread` —— 友元',
  '`static thread_local ETaskTag ActiveTaskTag` —— 每线程一份当前标签',
  '`ParentTag` / `Tag` / `TagOnlyIfNone` —— 父标签 / 本标签 / 是否只在无标签时打',
  '`SetTagNone()` —— 清空标签',
  '`SetTagStaticInit()` —— 恢复静态初始化标签',
  '`SwapTag(Tag)` —— 交换标签',
  '`GetCurrentTag()` / `IsCurrentTag()` / `IsRunningDuringStaticInit()` —— 查询',
  '`FOptionalTaskTagScope` —— 已废弃（5.6），因为 `FTaskTagScope` 现在默认就是可选行为'],
 [('第 687~693 行', '友元 + 4 个成员'),
  ('第 700~712 行', '3 个静态方法'),
  ('第 715~720 行', '构造/析构 + 3 个查询'),
  ('第 729~735 行', '废弃的 `FOptionalTaskTagScope`')],
 '像进车间换工牌：进门挂上，出门摘掉，中途还能查「我现在挂的是哪块」。'))

segments.append(('二十三、线程判断函数', 737, 791,
 ['`EnsureRetrievingVTablePtrDuringCtor(CtorSignature)` —— 校验取 vtable 的时机',
  '`IsInGameThread()` —— 是否在游戏线程',
  '`IsInParallelGameThread()` —— 是否在并行游戏线程',
  '`IsAudioThreadRunning()` / `IsInAudioThread()` —— 音频线程',
  '`IsInSlateThread()` —— Slate 线程',
  '`IsInRenderingThread()` / `IsInParallelRenderingThread()` / `IsInAnyRenderingThread()` / `IsInActualRenderingThread()` —— 4 个渲染线程判断',
  '`IsInAsyncLoadingThread` —— 函数指针',
  '`GRenderingThread` —— 渲染线程对象（已废弃）',
  '`IsRHIThreadRunning()` / `IsInRHIThread()` / `IsInParallelRHIThread()` —— RHI 线程',
  '`GIsRunningRHIInSeparateThread_InternalUseOnly` 等 3 个 —— RHI 线程模式标志'],
 [('第 744 行', 'vtable 校验'),
  ('第 747~758 行', '游戏/音频/Slate 线程判断'),
  ('第 761~774 行', '4 个渲染线程判断 + 异步加载'),
  ('第 778 行', '废弃的渲染线程对象'),
  ('第 781~791 行', 'RHI 线程判断 + 3 个模式标志')],
 '像问「我现在在哪个车间」：游戏、渲染、RHI、音频、Slate，各有各的门禁。'))

segments.append(('二十四、`FScopedLoadingState`（加载状态作用域）', 793, 819,
 ['`NotifyLoadingStateChanged(bState, Message)` —— 通知加载状态变化',
  '`FScopedLoadingState` —— 构造时通知「开始加载」，析构时通知「结束加载」',
  '`#if !UE_BUILD_SHIPPING` —— 非 Shipping 才有真实实现',
  'Shipping 下 —— 空实现 + 空壳结构体（零开销）'],
 [('第 795~808 行', '非 Shipping：真实实现'),
  ('第 810~818 行', 'Shipping：空实现')],
 '像打卡机：正式员工打卡记录，临时工直接放行不记。'))

segments.append(('二十五、绘制事件 + 调试可视化 + Zen', 821, 847,
 ['`GetEmitDrawEvents()` / `SetEmitDrawEvents()` —— 是否发出绘制事件',
  '`GCoreObjectArrayForDebugVisualizers` —— 调试可视化用的对象数组',
  '`UE::CoreUObject::Private::FStoredObjectPathDebug` —— 对象路径调试数据',
  '`UE::CoreUObject::Private::FObjectHandlePackageDebugData` —— 对象句柄调试数据',
  '`UE::Core::FVisualizerDebuggingState` —— 调试可视化状态',
  '`GCoreComplexObjectPathDebug` / `GCoreObjectHandlePackageDebug` / `GCoreDebuggingState` —— 3 个调试指针',
  '`IsRunningCookOnTheFly()` —— 是否在跑 cook-on-the-fly',
  '`UE::IsUsingZenPakFileStreaming()` —— 是否用 Zen 的 pak 流式加载'],
 [('第 821~822 行', '绘制事件开关'),
  ('第 825~840 行', '调试可视化（3 个前置声明 + 3 个指针）'),
  ('第 843 行', 'cook-on-the-fly'),
  ('第 846 行', 'Zen pak 流式加载')],
 '像调试器里的放大镜：专门给 Visual Studio 看对象用的辅助数据。'))

segments.append(('二十六、`IsThisNotNull`（已废弃）', 849, 854,
 ['`IsThisNotNull(This, Function)` —— 检查 `this` 是否非空',
  '`FORCENOINLINE` —— 强制不内联（保证能断点）',
  '`UE_DEPRECATED(5.6, ...)` —— 已废弃'],
 [('第 849~852 行', '说明块注释'),
  ('第 853 行', '废弃标记'),
  ('第 854 行', '函数声明')],
 '像老式保险丝：还在，但已经贴了「请换新方案」的标签。'))

md = []
md.append('# 13 — `CoreGlobals.h` 逐行注释版（超详细）')
md.append('')
md.append('> 依据：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\CoreGlobals.h`（**854 行**）')
md.append('> 你新建文件：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\CoreGlobals.h`')
md.append('> **✅ 本节可独立编译**——只 include Core 内部头 + 标准库 `<atomic>`。')
md.append('> **注意**：这份是**讲解版**（每行代码上方都有中文注释），还原文件已按它生成。')
md.append('')
md.append('---')
md.append('')
md.append('## 一、这个文件解决什么问题（为什么）')
md.append('')
md.append('引擎里到处都在用 `GIsEditor`、`GIsServer`、`GLog`、`GConfig`、`GFrameCounter` 这些**全局变量**。它们没有类、没有命名空间，直接以 `G` 开头散落在全局作用域里。`CoreGlobals.h` 就是这些全局量的**总登记处**：所有 `extern` 声明集中在这里，真正的定义在 `CoreGlobals.cpp`。')
md.append('')
md.append('**说个场景**：你在游戏代码里写 `if (GIsServer) { ... }`，编译器怎么知道 `GIsServer` 是什么类型、在哪个 DLL 里？答案就是它 include 了 `CoreGlobals.h`，从这里的 `extern CORE_API bool GIsServer;` 拿到声明，链接时再去 Core 模块里找定义。')
md.append('')
md.append('**它还管三件事**：')
md.append('')
md.append('1. **日志分类声明**（`LogHAL` / `LogCore` / `LogTemp` … 共 24 个）')
md.append('2. **线程标签系统**（`ETaskTag` + `FTaskTagScope`，`IsInGameThread()` 就靠它）')
md.append('3. **commandlet 判断**（`IsRunningCommandlet()` / `IsRunningCookCommandlet()` …）')
md.append('')
md.append('> 🔑 **一句话**：`CoreGlobals.h` = **引擎全局量的「户口本」**，854 行里 400 多个 `extern` 声明，谁要用谁查这本。')
md.append('')
md.append('---')
md.append('')
md.append('## 二、逐段代码（每行上方都有中文注释）')
md.append('')

for title, s, e_, new_elems, rows, analogy in segments:
    md.append('### %s（第 %d~%d 行）' % (title, s, e_))
    md.append('')
    md.append('```cpp')
    start = emap[s - 1] + 1 if (s - 1) in emap else 1
    end = emap[e_]
    for ln in range(start, end + 1):
        md.append(ours[ln - 1])
    md.append('```')
    md.append('')
    md.append('**本段新元素**')
    md.append('')
    for ne in new_elems:
        md.append('- ' + ne)
    md.append('')
    md.append('**这段在干什么**')
    md.append('')
    md.append('| 行 | 干什么 |')
    md.append('|---|---|')
    for a, b in rows:
        md.append('| %s | %s |' % (a, b))
    md.append('')
    md.append('> 类比：' + analogy)
    md.append('')

md.append('---')
md.append('')
md.append('## 三、速查：这段代码在干什么（一句话版）')
md.append('')
md.append('| 段 | 行 | 一句话 |')
md.append('|---|---|---|')
for title, s, e_, _, _, _ in segments:
    name = title.split('、', 1)[1]
    md.append('| %s | %d~%d | %s |' % (name, s, e_, name))
md.append('')
md.append('---')
md.append('')
md.append('## 四、C++ 语法小抄（本文件用到的）')
md.append('')
md.append('| 语法 | 含义 | 例子 |')
md.append('|---|---|---|')
md.append('| `extern CORE_API T X;` | 声明一个在别的编译单元定义的全局量 | `extern CORE_API bool GIsServer;` |')
md.append('| `DECLARE_LOG_CATEGORY_EXTERN(A,B,C)` | 声明日志分类 | `DECLARE_LOG_CATEGORY_EXTERN(LogCore, Log, All);` |')
md.append('| `enum class X : int32` | 强类型枚举 | `enum class ETaskTag : int32` |')
md.append('| `1 << n` | 第 n 位置 1 | `EGameThread = 1 << 1` |')
md.append('| `ENUM_CLASS_FLAGS(X)` | 生成位运算与判位工具 | `ENUM_CLASS_FLAGS(ETaskTag);` |')
md.append('| `static thread_local T X;` | 每线程一份的静态变量 | `static thread_local ETaskTag ActiveTaskTag;` |')
md.append('| `FORCEINLINE` | 强制内联 | `FORCEINLINE bool IsRunningCommandlet()` |')
md.append('| `FORCENOINLINE` | 强制不内联 | `CORE_API FORCENOINLINE bool IsThisNotNull(...)` |')
md.append('| `X() = delete` | 禁止调用 | `FCoreTexts(const FCoreTexts&) = delete;` |')
md.append('| `UE_DEPRECATED(v, "msg")` | 标记废弃 | `UE_DEPRECATED(5.5, "This will be removed.")` |')
md.append('| `operator bool() const` | 隐式类型转换 | `CORE_API operator bool() const;` |')
md.append('| `## __VA_ARGS__` | 可变参数透传（GCC 扩展） | `Fmt, ## __VA_ARGS__` |')
md.append('')
md.append('---')
md.append('')
md.append('## 五、我们这版 vs 引擎的差异')
md.append('')
md.append('| 项 | 引擎 | 我们 | 说明 |')
md.append('|---|---|---|---|')
md.append('| 代码行 | 854 行 | 855 行 | 只多 1 行（文件末尾换行） |')
md.append('| 非注释代码行 | 473 行 | 473 行 | **逐字一致** |')
md.append('| 注释语言 | 英文 | 中文 | 全部译成中文 |')
md.append('| 行尾注释 | 英文 | 中文 | 1 处（`EParallelThread`） |')
md.append('| 块注释行数 | 25 个块 | 25 个块，行数一一对应 | 逐行原位翻译，不增不减 |')
md.append('| `#endif // 宏名` | 保留宏名 | 保留宏名 | 宏名是标识符，不翻译 |')
md.append('')
md.append('---')
md.append('')
md.append('## 待确认')
md.append('')
md.append('1. 还原文件已生成：`ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\CoreGlobals.h`（855 行）')
md.append('2. 脚本验收：`codeOurs=473 / codeEngine=473 / diff=0`（剔除行尾注释后逐字一致）')
md.append('3. 确认后 `Public\\` 7 个散头全部完成，下一节进 `Public\\` 子目录')
md.append('')
md.append('---')
md.append('')
md.append('## 源码依据')
md.append('')
md.append('- 引擎：`D:\\ue5\\Epic Games\\UE_5.6\\Engine\\Source\\Runtime\\Core\\Public\\CoreGlobals.h`（854 行）')
md.append('- 还原：`E:\\ue5\\game_design\\code\\ue5Learing\\ue_5.6\\Engine\\Source\\Runtime\\Core\\Public\\CoreGlobals.h`（855 行）')
md.append('- 总 md：`AI_Learning_Notes\\从零复刻ue5\\Engine\\Source\\Runtime\\Core\\Public\\07_CoreGlobals.h_总md.md`')

with io.open(MD, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(md))
print('md lines =', len(md))