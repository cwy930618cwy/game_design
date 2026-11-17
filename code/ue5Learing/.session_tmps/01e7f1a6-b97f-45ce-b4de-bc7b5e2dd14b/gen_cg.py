# -*- coding: utf-8 -*-
import io, os

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'

pre = {}
rep = {}


def add(line, text):
    pre[line] = '// ' + text


add(2,  '头文件防重复包含：同一编译单元里多次 include 只展开一次。')

add(4,  '引入 FString：本文件大量全局变量是 FString 类型（各种 ini 路径）。')

add(5,  '引入基础类型层：uint8 / uint32 / uint64 / int32 / TCHAR 都来自这里。')

add(6,  '引入平台 TLS 接口：FTaskTagScope 用 thread_local 记录当前线程标签。')

add(7,  '引入日志宏：DECLARE_LOG_CATEGORY_EXTERN 在这里声明全局日志分类。')

add(8,  '引入编译开关宏：WITH_ENGINE / WITH_EDITOR / UE_BUILD_SHIPPING 等。')

add(9,  '引入 ENUM_CLASS_FLAGS 宏：给 ETaskTag 生成位运算。')

add(10, '引入输出设备接口：GError / GWarn / GLogConsole 的类型来源。')

add(11, '引入 CPU 性能分析追踪宏：SCOPED_BOOT_TIMING 里用到 TRACE_CPUPROFILER_EVENT_SCOPE_STR。')

add(12, '引入 TAtomic：TSAN_ATOMIC 宏展开后用到。')

add(13, '引入 FName / FLazyName：GCurrentTraceName / GLongCorePackageName 的类型。')

add(15, '引入标准库 atomic：GIsReinstancing 用 std::atomic<bool>。')

add(17, '前置声明 Error：错误对象类型，本文件只用到指针。')

add(18, '前置声明 FChunkedFixedUObjectArray：调试可视化用的对象数组。')

add(19, '前置声明 FConfigCacheIni：GConfig 的类型。')

add(20, '前置声明 FExec：GDebugToolExec 的类型。')

add(21, '前置声明 FFixedUObjectArray：旧版对象数组。')

add(22, '前置声明 FOutputDeviceConsole：GLogConsole 的类型。')

add(23, '前置声明 FOutputDeviceRedirector：GetGlobalLogSingleton 的返回类型。')

add(24, '前置声明 FRunnableThread：GRenderingThread 的类型。')

add(25, '前置声明 FText：FCoreTexts 里全是 FText 引用。')

add(26, '前置声明 ITransaction：GUndo 的类型。')

add(27, '前置声明 UClass：PRIVATE_GRunningCommandletClass 的类型。')

log_cats = [
    (29, 'LogHAL', 'HAL 平台抽象层日志'),
    (30, 'LogSerialization', '序列化日志'),
    (31, 'LogUnrealMath', '数学库日志'),
    (32, 'LogUnrealMatrix', '矩阵运算日志'),
    (33, 'LogContentComparisonCommandlet', '内容比对命令行工具日志'),
    (34, 'LogNetPackageMap', '网络包映射日志（默认 Warning 级）'),
    (35, 'LogNetSerialization', '网络序列化日志（默认 Warning 级）'),
    (36, 'LogMemory', '内存分配日志'),
    (37, 'LogProfilingDebugging', '性能分析调试日志'),
    (38, 'LogCore', 'Core 模块自身日志'),
    (39, 'LogOutputDevice', '输出设备日志'),
    (41, 'LogSHA', 'SHA 哈希日志（默认 Warning 级）'),
    (42, 'LogStats', '统计系统日志'),
    (43, 'LogStreaming', '流式加载日志（默认 Display 级）'),
    (44, 'LogInit', '引擎初始化日志'),
    (45, 'LogExit', '引擎退出日志'),
    (46, 'LogExec', '控制台命令执行日志（默认 Warning 级）'),
    (47, 'LogScript', '脚本执行日志（默认 Warning 级）'),
    (48, 'LogLocalization', '本地化日志'),
    (49, 'LogLongPackageNames', '长包名日志'),
    (50, 'LogProcess', '进程相关日志'),
    (51, 'LogLoad', '加载流程日志'),
    (52, 'LogVirtualization', '虚拟化（VA）日志'),
]
for ln, name, desc in log_cats:
    add(ln, '%s —— %s。' % (name, desc))

add(54, '说明：临时日志分类，一般不要把用到它的代码提交进仓库。')

add(55, 'LogTemp —— 临时调试日志分类，随手打日志用。')

add(57, '说明：平台专属日志分类在这里声明，方便任何地方直接用。')

add(58, '说明：需要再套一层宏，才能把「宏」当参数传给「宏」。')

add(59, '定义辅助宏：把 A 原样转给 DECLARE_LOG_CATEGORY_EXTERN，让平台宏能当分类名。')

add(60, '如果平台定义了 PLATFORM_GLOBAL_LOG_CATEGORY（平台全局日志分类名）。')

add(61, '用辅助宏声明该平台全局日志分类。')

add(62, '条件编译结束。')

add(63, '如果平台定义了 PLATFORM_GLOBAL_LOG_CATEGORY_ALT（备用平台日志分类名）。')

add(64, '用辅助宏声明该备用分类。')

add(65, '条件编译结束。')

add(68, '取全局日志重定向器单例（GLog 宏就是调它）。')

add(70, '打一个启动耗时点（Message 是阶段名）。')

add(72, '把所有启动耗时点汇总输出。')

add(74, '启动耗时作用域：构造时记开始时间，析构时自动上报耗时。')

add(76, '阶段名。')

add(77, '开始时间（秒）。')

add(78, '构造函数：只传阶段名。')

add(79, '构造函数：阶段名 + 后缀（同名阶段可区分）。')

add(80, '析构函数：自动上报耗时。')

add(83, '引擎活动追踪作用域：把一段代码标记成一个可被 Insights 追踪的活动。')

add(85, '构造函数：printf 风格格式化字符串 + 可变参数。')

add(86, '构造函数：直接传 FString。')

add(87, '析构函数：结束活动。')

add(91, '宏 SCOPED_BOOT_TIMING(x)：同时打 CPU 追踪事件 + 建启动耗时作用域，变量名自动生成。')

add(92, '宏 UE_SCOPED_ENGINE_ACTIVITY(Fmt, ...)：建引擎活动作用域，变量名自动生成。')

add(95, '宏 GLog：取全局日志重定向器，等价于 GetGlobalLogSingleton()。')

add(96, 'GConfig —— 全局配置缓存（读写 ini 用）。')

add(97, 'GUndo —— 全局撤销事务对象。')

add(98, 'GLogConsole —— 全局控制台输出设备。')

add(99, 'GError —— 全局错误输出设备（弹错误框用）。')

add(100, 'GWarn —— 全局警告/反馈上下文（弹对话框用）。')

add(103, 'GErrorHist —— 错误历史缓冲区（16384 个 TCHAR）。')

add(105, '说明：2014-08-19 的待办，计划把两个错误缓冲合并重构。')

add(106, 'GErrorExceptionDescription —— 异常描述缓冲区（4096 个 TCHAR）。')

add(108, 'FCoreTexts —— 引擎内置的本地化文本（True/False/Yes/No/None）。')

add(110, 'True 的本地化文本。')

add(111, 'False 的本地化文本。')

add(112, 'Yes 的本地化文本。')

add(113, 'No 的本地化文本。')

add(114, 'None 的本地化文本。')

add(116, '取全局唯一实例。')

add(118, '说明：让已有引用失效，调用后不要再使用 FCoreTexts。')

add(119, '销毁全局实例。')

add(121, '说明：禁止拷贝。')

add(122, '删除拷贝构造。')

add(123, '删除拷贝赋值。')

add(125, '构造函数：5 个文本全部传入。')

add(126, '初始化列表：逐个绑定引用。')

add(127, '构造函数体开始（空）。')

add(128, '构造函数体结束。')

add(129, '结构体结束。')

add(131, '如果没定义 DISABLE_LEGACY_CORE_TEXTS，或它被定义为 0（即仍启用旧全局文本）。')

add(132, '标记 GTrue 已废弃（4.23 起），改用 FCoreTexts::Get().True。')

add(133, 'GTrue —— 旧版全局 True 文本（已废弃）。')

add(134, '标记 GFalse 已废弃（4.23 起）。')

add(135, 'GFalse —— 旧版全局 False 文本（已废弃）。')

add(136, '标记 GYes 已废弃（4.23 起）。')

add(137, 'GYes —— 旧版全局 Yes 文本（已废弃）。')

add(138, '标记 GNo 已废弃（4.23 起）。')

add(139, 'GNo —— 旧版全局 No 文本（已废弃）。')

add(140, '标记 GNone 已废弃（4.23 起）。')

add(141, 'GNone —— 旧版全局 None 文本（已废弃）。')

add(142, '条件编译结束。')

add(144, '说明：为 true 时，这个可执行文件能运行所有游戏（游戏以 DLL 形式加载）。')

add(145, 'GIsGameAgnosticExe —— 是否「游戏无关」的可执行文件。')

add(147, '说明：从游戏里保存时，这个开关允许加载「仅编辑器」属性。')

add(148, 'GForceLoadEditorOnly —— 强制加载仅编辑器属性。')

add(150, '说明：禁止加载不在脚本文件里的对象，脚本编译期间用。')

add(151, 'GVerifyObjectReferencesOnly —— 只校验对象引用。')

add(153, '说明：构造对象时在主机平台走快速路径。')

add(154, 'GFastPathUniqueNameGeneration —— 唯一名生成走快速路径。')

add(156, '说明：允许 AActor 在编辑器里从特定入口执行脚本（如构造脚本）。')

add(157, 'GAllowActorScriptExecutionInEditor —— 允许编辑器内执行 Actor 脚本。')

add(159, '说明：给 CDO 里新实例化的组件强制使用模板名。')

add(160, 'GCompilingBlueprint —— 是否正在编译蓝图。')

add(162, '说明：蓝图编译后是否正在做垃圾回收。')

add(163, 'GIsGCingAfterBlueprintCompile —— 蓝图编译后正在 GC。')

add(165, '说明：是否正在重建蓝图实例；cook 后的构建里永远不该为 true。')

add(166, 'GIsReconstructingBlueprintInstances —— 正在重建蓝图实例。')

add(168, '说明：Actor 和对象是否正在被重新实例化。')

add(169, 'GIsReinstancing —— 正在重新实例化（原子布尔，多线程可见）。')

add(171, '说明：用于冲刷资源流式加载的辅助函数指针。')

add(172, 'GFlushStreamingFunc —— 冲刷流式加载的函数指针。')

add(174, '说明：UE-as-a-library（把引擎当库嵌入）功能用的设置。')

add(175, 'FUELibraryOverrideSettings —— 嵌入模式下的覆盖设置。')

add(177, '说明：是否通过 UELibrary 初始化；为 false 时下面所有字段都无效。')

add(179, 'bIsEmbedded —— 是否嵌入模式，默认 false。')

add(181, '说明：把引擎嵌入到哪个窗口句柄。')

add(182, 'WindowHandle —— 宿主窗口句柄，默认空。')

add(184, '说明：覆盖后的嵌入视口宽度。')

add(185, 'WindowWidth —— 视口宽度，默认 0。')

add(187, '说明：覆盖后的嵌入视口高度。')

add(188, 'WindowHeight —— 视口高度，默认 0。')

add(189, '结构体结束。')

add(191, '说明：把 UE 当库使用时的设置。')

add(192, 'GUELibraryOverrideSettings —— 全局嵌入设置实例。')

add(194, 'GIsRunningUnattendedScript —— 是否在跑无人值守脚本。')

add(196, '如果编了引擎（WITH_ENGINE）。')

add(197, 'PRIVATE_GIsRunningCommandlet —— 是否在跑命令行工具（commandlet）。')

add(198, 'PRIVATE_GRunningCommandletClass —— 正在跑的 commandlet 的 UClass。')

add(200, '说明：为 true 时，即使跑 commandlet 也初始化 RHI 并搭渲染场景。')

add(201, 'PRIVATE_GAllowCommandletRendering —— commandlet 下允许渲染。')

add(203, '说明：为 true 时，即使跑 commandlet 也初始化音频。')

add(204, 'PRIVATE_GAllowCommandletAudio —— commandlet 下允许音频。')

add(205, '条件编译结束。')

add(207, '如果编了「仅编辑器数据」（WITH_EDITORONLY_DATA）。')

add(209, '说明：是否在编辑器里。注意 PIE（Play In Editor）时它仍为 true；')

add(210, '那种情况应改用 GWorld->HasBegunPlay 判断。')

add(211, '说明结束。')

add(213, 'GIsEditor —— 是否在编辑器里。')

add(214, 'GIsImportingT3D —— 是否正在导入 T3D。')

add(215, 'GIsTransacting —— 是否正在事务（撤销/重做）中。')

add(217, '说明：游戏线程是否正卡在深层调用栈里，同时编辑器 UI 在泵消息；')

add(218, 'UI 泵消息循环自然返回前，游戏线程做不了任何事。')

add(219, '说明结束。')

add(220, 'GIntraFrameDebuggingGameThread —— 是否处于帧内调试（游戏线程被暂停）。')

add(221, '说明：是否是第一次进入 UI 消息泵循环。')

add(222, 'GFirstFrameIntraFrameDebugging —— 是否首帧帧内调试。')

add(224, '否则如果是在做代码分析（USING_CODE_ANALYSIS）。')

add(226, '说明：代码分析期间把它们定义成变量，避免大量「常量 && 表达式」警告。')

add(227, 'GIsEditor —— 代码分析期用变量形式。')

add(228, 'GIntraFrameDebuggingGameThread —— 代码分析期用变量形式。')

add(229, 'GFirstFrameIntraFrameDebugging —— 代码分析期用变量形式。')

add(231, '否则（普通运行时）。')

add(233, 'GIsEditor 直接定义成常量 false。')

add(234, 'GIntraFrameDebuggingGameThread 直接定义成常量 false。')

add(235, 'GFirstFrameIntraFrameDebugging 直接定义成常量 false。')

add(237, '条件编译结束（WITH_EDITORONLY_DATA）。')

add(239, '如果编了编辑器（WITH_EDITOR）。')

add(240, 'PRIVATE_GIsRunningCookCommandlet —— 是否在跑 cook commandlet。')

add(241, 'PRIVATE_GIsRunningDLCCookCommandlet —— 是否在跑 DLC cook commandlet。')

add(242, '条件编译结束。')

add(244, '说明：检查这个可执行文件是否在跑 commandlet（编辑器式环境下的自定义命令行处理）。')

add(245, '说明结束。')

add(247, 'IsRunningCommandlet —— 是否在跑 commandlet。')

add(249, '如果编了引擎。')

add(250, '返回引擎版的实际标志。')

add(251, '否则。')

add(252, '没有引擎就永远 false。')

add(253, '条件编译结束。')

add(254, '函数结束。')

add(256, '说明：检查是否在跑 cook commandlet。')

add(257, '说明结束。')

add(259, 'IsRunningCookCommandlet —— 是否在跑 cook commandlet。')

add(261, '如果编了编辑器。')

add(262, '返回编辑器版的实际标志。')

add(263, '否则。')

add(264, '没有编辑器就永远 false。')

add(265, '条件编译结束。')

add(266, '函数结束。')

add(268, '说明：检查是否在跑 DLC cook commandlet。')

add(269, '说明结束。')

add(271, 'IsRunningDLCCookCommandlet —— 是否在跑 DLC cook commandlet。')

add(273, '如果编了编辑器。')

add(274, '返回编辑器版的实际标志。')

add(275, '否则。')

add(276, '没有编辑器就永远 false。')

add(277, '条件编译结束。')

add(278, '函数结束。')

add(280, '说明：返回当前正在跑的 commandlet 的 UClass。')

add(281, '说明结束。')

add(283, 'GetRunningCommandletClass —— 取当前 commandlet 的 UClass。')

add(285, '如果编了引擎。')

add(286, '返回引擎版的实际指针。')

add(287, '否则。')

add(288, '没有引擎就返回空指针。')

add(289, '条件编译结束。')

add(290, '函数结束。')

add(292, '说明：返回命令行参数 -Run=<CommandletName> 指定的 commandlet 名字。')

add(293, '说明：给需要在引擎启动早期、commandlet 还没初始化时就知道名字的代码用。')

add(294, '说明：能用 GetRunningCommandletClass()->GetName() 就优先用它。')

add(295, '说明：空行。')

add(296, '说明：@return 最终会运行的 commandlet 名字；没有则返回空字符串。')

add(297, '说明结束。')

add(304, 'GetCommandletNameFromCmdline —— 从命令行取 commandlet 名字。')

add(306, '说明：检查跑 commandlet 时是否也要初始化 RHI 并搭渲染场景。')

add(307, '说明结束。')

add(309, 'IsAllowCommandletRendering —— commandlet 下是否允许渲染。')

add(311, '如果编了引擎。')

add(312, '返回引擎版的实际标志。')

add(313, '否则。')

add(314, '没有引擎就永远 false。')

add(315, '条件编译结束。')

add(316, '函数结束。')

add(318, 'IsAllowCommandletAudio —— commandlet 下是否允许音频。')

add(320, '如果编了引擎。')

add(321, '返回引擎版的实际标志。')

add(322, '否则。')

add(323, '没有引擎就永远 false。')

add(324, '条件编译结束。')

add(325, '函数结束。')

add(327, '命名空间 UE：引擎新版公共 API 都放这里。')

add(329, '说明：多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0；')

add(330, '单进程运行时返回 0。基于命令行判断，典型场景是多进程 cook 的 cookworker。')

add(331, '需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。')

add(332, '说明结束。')

add(336, 'GetMultiprocessId —— 取当前进程的多进程编号。')

add(337, '命名空间结束。')

add(339, '命名空间 UE::Private：引擎内部实现细节，外部不要直接调。')

add(341, 'SetMultiprocessId —— 设置当前进程的多进程编号（内部用）。')

add(342, '命名空间结束。')

add(344, 'FIsDuplicatingClassForReinstancing —— 可隐式转 bool 的包装类，用来标记「正在为重新实例化复制类」。')

add(346, '公有区。')

add(347, '赋值运算符：接受 bool，返回自身引用，支持链式赋值。')

add(348, '类型转换运算符：隐式转成 bool。')

add(349, '类结束。')

add(351, 'GEdSelectionLock —— 编辑器选择是否被锁定。')

add(352, 'GIsClient —— 当前是否是客户端。')

add(353, 'GIsServer —— 当前是否是服务器。')

add(354, 'GIsCriticalError —— 是否发生了致命错误（TSAN 原子布尔）。')

add(355, 'GIsRunning —— 引擎是否在运行（TSAN 原子布尔）。')

add(356, 'GIsDuplicatingClassForReinstancing —— 是否正在为重新实例化复制类。')

add(358, '说明：下面这些在引擎首次启动时设置。')

add(359, '说明结束。')

add(362, '说明：指定引擎是否作为构建机进程启动。')

add(363, '说明结束。')

add(365, 'GIsBuildMachine —— 是否是构建机进程。')

add(367, '说明：决定是否输出日志文本；为 true 时不输出任何日志。')

add(368, '说明结束。')

add(370, 'GIsSilent —— 静默模式（不输出日志）。')

add(372, 'GIsSlowTask —— 是否有慢任务在进行（会弹进度条）。')

add(373, 'GSlowTaskOccurred —— 是否发生过慢任务。')

add(374, 'GIsGuarded —— 是否处于异常保护块内。')

add(376, '说明：设为 true 后，只允许在引擎 tick 开始时设置 RequestingExit，')

add(377, '可消除设置 RequestExit 时的未定义行为风险。')

add(378, '说明：这个开关需要在所有平台/用法上验证过，才能改成默认值。')

add(379, '说明结束。')

add(382, '如果没定义 UE_SET_REQUEST_EXIT_ON_TICK_ONLY。')

add(383, '默认定义为 0（关闭）。')

add(384, '条件编译结束。')

add(386, '说明：引擎启动时所有初始模块完成各自加载阶段后返回 true；')

add(387, '对应事件 FCoreDelegates::OnAllModuleLoadingPhasesComplete。')

add(388, '说明结束。')

add(390, 'IsEngineStartupModuleLoadingComplete —— 启动模块加载是否全部完成。')

add(391, 'SetEngineStartupModuleLoadingComplete —— 标记启动模块加载完成。')

add(393, '标记 GIsRequestingExit 已废弃（4.24 起），改用 IsEngineExitRequested() / RequestEngineExit()。')

add(394, 'GIsRequestingExit —— 是否已请求退出引擎（已废弃）。')

add(396, '说明：检查是否收到退出请求，收到就把 GIsRequestingExit 置位。')

add(397, '说明结束。')

add(399, 'BeginExitIfRequested —— 有退出请求就开始退出流程。')

add(401, 'IsEngineExitRequested —— 是否已请求退出引擎。')

add(403, '临时关闭废弃警告（下面要读已废弃的 GIsRequestingExit）。')

add(404, '返回退出请求标志。')

add(405, '恢复废弃警告。')

add(406, '函数结束。')

add(408, '说明：请求引擎在能安全退出时尽快退出。')

add(409, '说明：ReasonString 不是可选的，要写清为什么请求退出。')

add(410, 'RequestEngineExit —— 请求退出（C 字符串版）。')

add(411, 'RequestEngineExit —— 请求退出（FString 版）。')

add(413, '说明：全局开关，控制是否显示屏幕上的警告/消息。')

add(414, '说明：控制台命令 DISABLEALLSCREENMESSAGES 关闭，ENABLEALLSCREENMESSAGES 打开，')

add(415, 'TOGGLEALLSCREENMESSAGES 切换。')

add(416, '说明结束。')

add(419, 'GAreScreenMessagesEnabled —— 屏幕消息是否启用。')

add(420, 'GScreenMessagesRestoreState —— 恢复用的原状态。')

add(422, '说明：是否正在截屏/录屏。')

add(423, 'GIsDumpingMovie —— 是否在导出序列帧（0 表示不导出）。')

add(424, 'GIsHighResScreenshot —— 是否在做高分辨率截图。')

add(425, 'GScreenshotResolutionX —— 截图宽度。')

add(426, 'GScreenshotResolutionY —— 截图高度。')

add(427, 'GMakeCacheIDIndex —— 缓存 ID 生成计数器。')

add(429, 'GEngineIni —— 引擎主 ini 文件路径。')

add(431, '说明：编辑器 ini 位置，按引擎版本存（所有项目共享），首次运行时在版本间迁移。')

add(432, 'GEditorLayoutIni —— 编辑器布局 ini。')

add(433, 'GEditorKeyBindingsIni —— 编辑器快捷键 ini。')

add(434, 'GEditorSettingsIni —— 编辑器设置 ini。')

add(436, '说明：编辑器「按项目」的 ini 文件，按项目存。')

add(437, 'GEditorIni —— 编辑器项目 ini。')

add(438, 'GEditorPerProjectIni —— 编辑器每项目独立 ini。')

add(440, 'GCompatIni —— 兼容性 ini。')

add(441, 'GLightmassIni —— Lightmass 光照烘焙 ini。')

add(442, 'GScalabilityIni —— 画质可伸缩性 ini。')

add(443, 'GHardwareIni —— 硬件配置 ini。')

add(444, 'GInputIni —— 输入配置 ini。')

add(445, 'GGameIni —— 游戏配置 ini。')

add(446, 'GGameUserSettingsIni —— 玩家用户设置 ini。')

add(447, 'GRuntimeOptionsIni —— 运行时选项 ini。')

add(448, 'GInstallBundleIni —— 安装包 ini。')

add(449, 'GDeviceProfilesIni —— 设备配置档 ini。')

add(450, 'GGameplayTagsIni —— GameplayTag 配置 ini。')

add(452, 'GNearClippingPlane —— 近裁剪面距离。')

add(453, 'GNearClippingPlane_RenderThread —— 渲染线程用的近裁剪面距离。')

add(455, 'GExitPurge —— 退出时是否做彻底清理。')

add(456, 'GInternalProjectName —— 内部项目名（最多 64 个 TCHAR）。')

add(457, 'GForeignEngineDir —— 外部引擎目录（跨引擎引用时用）。')

add(459, '说明：游戏调试工具的 exec 处理器，支持 editactor 这类命令。')

add(460, 'GDebugToolExec —— 调试工具 exec 处理器。')

add(462, '说明：当前是否在异步加载代码路径里。')

add(463, 'IsAsyncLoading —— 函数指针，判断是否在异步加载。')

add(465, '说明：暂停异步包加载。')

add(466, 'SuspendAsyncLoading —— 暂停异步加载的函数指针。')

add(468, '说明：恢复异步包加载。')

add(469, 'ResumeAsyncLoading —— 恢复异步加载的函数指针。')

add(471, '说明：异步加载是否被暂停（引擎原文注释写的是 Suspends，实际是查询）。')

add(472, 'IsAsyncLoadingSuspended —— 查询异步加载是否暂停。')

add(474, '说明：异步加载是否使用独立线程。')

add(475, 'IsAsyncLoadingMultithreaded —— 查询异步加载是否多线程。')

add(477, 'ELoaderType —— 当前使用的包加载器类型。')

add(479, 'NotInitialized —— 还没初始化。')

add(480, 'LegacyLoader —— 旧版加载器。')

add(481, 'EditorPackageLoader —— 编辑器包加载器。')

add(482, 'ZenLoader —— Zen 加载器（新存储后端）。')

add(483, '枚举体结束。')

add(485, 'LexToString —— 把 ELoaderType 转成字符串。')

add(487, '说明：返回当前激活的加载器类型，没有则返回默认值。')

add(488, 'GetLoaderType —— 函数指针，取当前加载器类型。')

add(490, '说明：暂停「异步 IO 完成后触发的纹理更新」。')

add(491, 'SuspendTextureStreamingRenderTasks —— 暂停纹理流式渲染任务。')

add(493, '说明：恢复「异步 IO 完成后触发的纹理更新」。')

add(494, 'ResumeTextureStreamingRenderTasks —— 恢复纹理流式渲染任务。')

add(496, 'FIsEditorLoadingPackage —— 已废弃（5.5），可隐式转 bool 的包装类。')

add(498, '公有区。')

add(499, '赋值运算符：接受 bool。')

add(500, '类型转换运算符：隐式转 bool。')

add(501, '类结束。')

add(503, '临时关闭废弃警告（下面要用已废弃的类型）。')

add(504, '说明：编辑器当前是否正在加载包。')

add(505, '标记 GIsEditorLoadingPackage 已废弃（5.5），改用 UE::GetIsEditorLoadingPackage() / SetIsEditorLoadingPackage()。')

add(506, 'GIsEditorLoadingPackage —— 编辑器是否正在加载包（已废弃）。')

add(507, '恢复废弃警告。')

add(509, '命名空间 UE。')

add(511, 'GetIsEditorLoadingPackage —— 查询编辑器是否正在加载包。')

add(512, 'SetIsEditorLoadingPackage —— 设置该标志。')

add(513, '命名空间结束。')

add(515, '说明：cooker 当前是否正在加载包。')

add(516, 'GIsCookerLoadingPackage —— cooker 是否正在加载包。')

add(518, '说明：GWorld 是否指向 PIE（Play In Editor）世界。')

add(519, 'GIsPlayInEditorWorld —— 是否在 PIE 世界里。')

add(521, 'FPlayInEditorID —— 已废弃（5.5），可隐式转 int32 的包装类。')

add(523, '公有区。')

add(524, '赋值运算符：接受 int32。')

add(525, '类型转换运算符：隐式转 int32。')

add(526, '类结束。')

add(528, '临时关闭废弃警告。')

add(529, '说明：编辑器里表示当前正在处理的 PIE 世界上下文实例号。')

add(530, '标记 GPlayInEditorID 已废弃（5.5），改用 UE::GetPlayInEditorID() / SetPlayInEditorID()。')

add(531, 'GPlayInEditorID —— 当前 PIE 实例号（已废弃）。')

add(532, '恢复废弃警告。')

add(534, '命名空间 UE。')

add(536, 'GetPlayInEditorID —— 取当前 PIE 实例号。')

add(537, 'SetPlayInEditorID —— 设置当前 PIE 实例号。')

add(538, '命名空间结束。')

add(540, '说明：PIE 是否尝试从 PlayerStart 开始播放。')

add(541, '标记 GIsPIEUsingPlayerStart 已废弃（4.25），该变量已不再被设置。')

add(542, 'GIsPIEUsingPlayerStart —— PIE 是否用 PlayerStart（已废弃）。')

add(544, '说明：运行时是否要求纹理尺寸为 2 的幂。')

add(545, 'GPlatformNeedsPowerOfTwoTextures —— 平台是否要求 2 的幂纹理。')

add(547, '说明：FPlatformTime::Seconds() 首次初始化的时间点（非常早）。')

add(548, 'GStartTime —— 引擎启动时间（秒）。')

add(550, '说明：引擎初始化时的系统时间。')

add(551, 'GSystemStartTime —— 系统启动时间字符串。')

add(553, '说明：是否还在初始加载过程中。')

add(554, 'GIsInitialLoad —— 是否处于初始加载。')

add(556, '说明：是否使用事件驱动加载器。')

add(557, 'GEventDrivenLoaderEnabled —— 事件驱动加载器是否启用。')

add(559, '说明：是否正在从 UClass 取 VTablePtr。')

add(560, 'GIsRetrievingVTablePtr —— 是否正在取 VTablePtr。')

add(562, '说明：稳定递增的帧计数器。')

add(563, 'GFrameCounter —— 帧计数器（64 位）。')

add(565, 'GFrameCounterRenderThread —— 渲染线程侧的帧计数器。')

add(567, '说明：上一次跑 GC 时的 GFrameCounter。')

add(568, 'GLastGCFrame —— 上次 GC 的帧号。')

add(570, '说明：输入采样时间（单位：CPU 周期）。')

add(571, 'GInputTime —— 输入采样时间。')

add(573, '说明：每帧渲染场景前自增一次；分屏模式下所有视图共用一次自增（不是每视图一次）。')

add(574, 'GFrameNumber —— 帧号（32 位）。')

add(576, '说明：待重命名；渲染线程版帧号请用 View.ViewFamily->FrameNumber 或从 GFrameTime 传下去。')

add(577, 'GFrameNumberRenderThread —— 渲染线程侧帧号。')

add(579, '说明：是否是游戏运行的第一个实例。')

add(580, '标记 GIsFirstInstance 已废弃（5.1），改用 FPlatformProcess::IsFirstInstance()。')

add(581, 'GIsFirstInstance —— 是否首个实例（已废弃）。')

add(583, '说明：一帧被判定为「卡顿」的阈值（毫秒）。')

add(584, 'GHitchThresholdMS —— 卡顿阈值毫秒数。')

add(586, '说明：保存压缩数据时，把数据切成多大的块。')

add(587, 'GSavingCompressionChunkSize —— 压缩分块大小。')

add(589, '说明：主线程/游戏线程的线程 ID。')

add(590, 'GGameThreadId —— 游戏线程 ID。')

add(592, '说明：渲染线程的线程 ID（如果有）。')

add(593, '标记 GRenderThreadId 已废弃（4.26），改用 IsInActualRenderingThread()。')

add(594, 'GRenderThreadId —— 渲染线程 ID（已废弃）。')

add(596, '说明：Slate 线程的线程 ID（如果有）。')

add(597, 'GSlateLoadingThreadId —— Slate 加载线程 ID。')

add(599, '说明：GGameThreadId 是否已经设置过。')

add(600, 'GIsGameThreadIdInitialized —— 游戏线程 ID 是否已初始化。')

add(602, '说明：决定做哪种 trace，NAME_None 表示不做。')

add(603, 'GCurrentTraceName —— 当前 trace 名。')

add(605, '说明：日志里时间怎么打印。')

add(606, 'GPrintLogTimes —— 日志时间格式（TSAN 原子）。')

add(608, '说明：日志里分类怎么打印。')

add(609, 'GPrintLogCategory —— 是否打印日志分类（TSAN 原子）。')

add(611, '说明：日志里详细级别怎么打印。')

add(612, 'GPrintLogVerbosity —— 是否打印详细级别（TSAN 原子）。')

add(614, '如果启用了卡顿检测（USE_HITCH_DETECTION）。')

add(615, '说明：轻量统计和 FGameThreadHitchHeartBeat 用它，在 Shipping 包里打印卡顿时的调用栈。')

add(616, 'GHitchDetected —— 是否检测到卡顿（TSAN 原子）。')

add(617, '条件编译结束。')

add(619, '说明：统计系统是否要为 PIX 这类工具发命名事件。')

add(620, 'GCycleStatsShouldEmitNamedEvents —— 是否发命名事件。')

add(622, '说明：详细统计是否也生成外部性能分析器的命名事件；')

add(623, '线程睡眠/等待统计和超高频周期统计默认关闭；')

add(624, 'GCycleStatsShouldEmitNamedEvents 为 0 时本开关无效。')

add(625, '说明结束。')

add(626, 'GShouldEmitVerboseNamedEvents —— 详细统计是否发命名事件。')

add(628, '说明：关闭一些会打断演示的警告和小功能。')

add(629, 'GIsDemoMode —— 演示模式。')

add(631, '说明：Core 包的名字。')

add(632, '说明：包名迁移期的临时标记，迁移完成后删掉双重检查。')

add(633, 'GLongCorePackageName —— Core 包长名。')

add(634, '说明：包名迁移期的临时标记，迁移完成后删掉双重检查。')

add(635, 'GLongCoreUObjectPackageName —— CoreUObject 包长名。')

add(637, '说明：当前是否在跑单元测试。')

add(638, 'GIsAutomationTesting —— 是否在自动化测试。')

add(640, '说明：是否在主循环之外泵消息。')

add(641, 'GPumpingMessagesOutsideOfMainLoop —— 主循环外泵消息。')

add(643, '说明：是否正在泵消息。')

add(644, 'GPumpingMessages —— 是否正在泵消息。')

add(646, '说明：启用各种编辑器和 HMD 的 hack，让实验性 VR 编辑器功能可用（可能牺牲其他系统）。')

add(647, 'GEnableVREditorHacks —— 是否启用 VR 编辑器 hack。')

add(649, '如果不是 Shipping 构建。')

add(651, '说明：是否忽略已附加的调试器。')

add(652, 'GIgnoreDebugger —— 是否忽略调试器。')

add(654, '条件编译结束（!UE_BUILD_SHIPPING）。')

add(656, 'ETaskTag —— 执行上下文（线程或任务）的标签，用来判断「我现在在哪个线程」。')

add(658, 'ENone —— 无标签。')

add(659, 'EStaticInit —— 静态初始化阶段。')

add(660, 'EGameThread —— 游戏线程。')

add(661, 'ESlateThread —— Slate UI 线程。')

add(662, 'EAudioThread —— 音频线程（5.0 起已废弃，不再使用）。')

add(663, 'ERenderingThread —— 渲染线程。')

add(664, 'ERhiThread —— RHI 线程。')

add(665, 'EAsyncLoadingThread —— 异步加载线程。')

add(666, 'EEventThread —— 事件线程。')

add(668, 'ENamedThreadBits —— 所有「具名线程」位的掩码。')

add(669, 'EParallelThread —— 并行标记位；多个线程/任务参与时用，跳过具名线程唯一性检查。')

add(670, 'EWorkerThread —— 工作线程（并行 + 独立位）。')

add(671, 'EParallelRenderingThread —— 并行渲染线程。')

add(672, 'EParallelGameThread —— 并行游戏线程。')

add(673, 'EParallelRhiThread —— 并行 RHI 线程。')

add(674, '枚举体结束。')

add(676, '给 ETaskTag 生成位运算与判位工具。')

add(679, '说明：这个类用来给执行上下文（线程或任务）打标签，之后可在调用栈里查询状态；')

add(680, '通常给 IsInRenderingThread / IsInGameThread 这类函数用。')

add(681, '说明：@param InTag 要用的标签。')

add(682, '说明结束。')

add(685, 'FTaskTagScope —— 标签作用域，构造时打标签、析构时恢复。')

add(687, '友元：FRunnableThread 可访问私有成员。')

add(688, '友元：FRenderingThread 可访问私有成员。')

add(689, '每个线程各自一份「当前激活标签」（thread_local）。')

add(690, '取静态线程 ID。')

add(691, '父标签（用于析构时恢复）。')

add(692, '本作用域的标签。')

add(693, '是否只在当前无标签时才打标签。')

add(695, '公有区。')

add(696, '说明：清掉 EStaticInit 标签，让主 OS 线程没有激活标签；')

add(697, '这样从主线程调用 IsInGameThread() 这类函数才能正常工作。')

add(698, '说明结束。')

add(700, 'SetTagNone —— 清空当前线程标签。')

add(702, '说明：恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数')

add(703, '在检查线程状态（如 IsInGameThread()）时能正常工作。')

add(704, '说明结束。')

add(707, 'SetTagStaticInit —— 恢复静态初始化标签。')

add(709, '说明：交换标签，只在「线程上下文在不同线程间迁移」时用。')

add(710, '说明结束。')

add(712, 'SwapTag —— 交换当前标签并返回旧标签。')

add(714, '公有区。')

add(715, '构造函数：进入作用域时打上 InTag。')

add(716, '析构函数：离开作用域时恢复父标签。')

add(718, 'GetCurrentTag —— 取当前线程的标签。')

add(719, 'IsCurrentTag —— 当前标签是否等于 InTag。')

add(720, 'IsRunningDuringStaticInit —— 是否处于静态初始化阶段。')

add(721, '类结束。')

add(723, '说明：这个类也能给执行上下文打标签，但只在「还没打过标签」时才打；')

add(724, '通常给 IsInRenderingThread / IsInGameThread 这类函数用。')

add(725, '说明：@param InTag 要用的标签。')

add(726, '说明结束。')

add(729, 'FOptionalTaskTagScope —— 已废弃（5.6），因为 FTaskTagScope 现在默认就是「可选」行为。')

add(731, '公有区。')

add(732, '构造函数：默认标签 ENone，直接转给父类。')

add(733, '构造函数体开始。')

add(734, '构造函数体结束。')

add(735, '类结束。')

add(737, '说明：确保当前线程正处于「从 UClass 取 vtable 指针」的过程中；')

add(738, '@param CtorSignature 当前正在跑的构造函数签名，用来拼错误信息。')

add(739, '说明结束。')

add(744, 'EnsureRetrievingVTablePtrDuringCtor —— 校验取 vtable 的时机。')

add(746, '说明：@return 是否从游戏线程调用。')

add(747, 'IsInGameThread —— 是否在游戏线程。')

add(749, '说明：@return 是否从「并行 for 里的游戏线程」调用。')

add(750, 'IsInParallelGameThread —— 是否在并行游戏线程。')

add(752, 'IsAudioThreadRunning —— 音频线程是否在运行。')

add(754, '说明：@return 是否从音频线程调用（不是「调用了音频函数的线程」）。')

add(755, 'IsInAudioThread —— 是否在音频线程。')

add(757, '说明：@return 是否从 Slate 线程调用（不是「调用了 Slate 函数的线程」）。')

add(758, 'IsInSlateThread —— 是否在 Slate 线程。')

add(760, '说明：@return 是否从渲染线程调用；单线程渲染时任何线程都算。')

add(761, 'IsInRenderingThread —— 是否在渲染线程。')

add(763, '说明：@return 是否从渲染线程调用，或从任何非游戏线程调用；单线程渲染时游戏线程也算。')

add(764, 'IsInParallelRenderingThread —— 是否在并行渲染线程。')

add(766, '说明：@return 是否从任一渲染线程调用；任何线程都可安全调用，不会断言。')

add(767, 'IsInAnyRenderingThread —— 是否在任一渲染线程。')

add(769, '说明：@return 是否从渲染线程调用。')

add(770, '说明：与 IsInRenderingThread 不同，单线程渲染时它永远返回 false；只有真正独立的渲染线程才返回 true。')

add(771, 'IsInActualRenderingThread —— 是否在真正的渲染线程。')

add(773, '说明：@return 异步加载线程启用时看是否在该线程；否则看是否在游戏线程跑异步加载代码。')

add(774, 'IsInAsyncLoadingThread —— 函数指针，判断是否在异步加载线程。')

add(776, '说明：渲染用的线程对象。')

add(777, '标记 GRenderingThread 已废弃（4.26），改用 GIsThreadedRendering 或 IsInActualRenderingThread()。')

add(778, 'GRenderingThread —— 渲染线程对象（已废弃）。')

add(780, '说明：@return RHI 线程是否在运行。')

add(781, 'IsRHIThreadRunning —— RHI 线程是否在运行。')

add(783, '说明：@return 是否从 RHI 线程调用；单线程渲染时任何线程都算。')

add(784, 'IsInRHIThread —— 是否在 RHI 线程。')

add(786, '说明：@return 是否从「标记为 RHI 任务」的线程调用。')

add(787, 'IsInParallelRHIThread —— 是否在并行 RHI 线程。')

add(789, 'GIsRunningRHIInSeparateThread_InternalUseOnly —— RHI 是否跑在独立线程（内部用）。')

add(790, 'GIsRunningRHIInDedicatedThread_InternalUseOnly —— RHI 是否跑在专用线程（内部用）。')

add(791, 'GIsRunningRHIInTaskThread_InternalUseOnly —— RHI 是否跑在任务线程（内部用）。')

add(793, '说明：启动加载计时器。')

add(794, '如果不是 Shipping 构建。')

add(795, 'NotifyLoadingStateChanged —— 通知加载状态变化（真实实现）。')

add(796, 'FScopedLoadingState —— 加载状态作用域，构造时通知「开始加载」，析构时通知「结束加载」。')

add(798, '加载阶段名。')

add(799, '构造函数：记下阶段名并通知开始。')

add(800, '初始化列表：保存阶段名。')

add(801, '构造函数体开始。')

add(802, '通知「进入加载状态」。')

add(803, '构造函数体结束。')

add(804, '析构函数：通知结束。')

add(805, '析构函数体开始。')

add(806, '通知「退出加载状态」。')

add(807, '析构函数体结束。')

add(808, '结构体结束。')

add(809, '否则（Shipping 构建）。')

add(810, 'NotifyLoadingStateChanged —— Shipping 下空实现。')

add(811, '函数体开始。')

add(812, '函数体结束（什么都不做）。')

add(813, 'FScopedLoadingState —— Shipping 下的空壳版本。')

add(814, '结构体开始。')

add(815, '构造函数：什么都不做。')

add(816, '构造函数体开始。')

add(817, '构造函数体结束。')

add(818, '结构体结束。')

add(819, '条件编译结束。')

add(821, 'GetEmitDrawEvents —— 是否发出绘制事件（调试用）。')

add(822, 'SetEmitDrawEvents —— 设置是否发出绘制事件。')

add(824, '说明：帮助在调试器里可视化弱指针的数组。')

add(825, '前置声明 FChunkedFixedUObjectArray。')

add(827, 'GCoreObjectArrayForDebugVisualizers —— 调试可视化用的对象数组指针。')

add(829, '命名空间 UE::CoreUObject::Private：CoreUObject 内部实现细节。')

add(831, '说明：帮助在调试器里可视化对象路径的数组。')

add(832, '前置声明 FStoredObjectPathDebug。')

add(834, '说明：帮助在调试器里可视化对象句柄的数组。')

add(835, '前置声明 FObjectHandlePackageDebugData。')

add(836, '命名空间结束。')

add(837, '前置声明 UE::Core::FVisualizerDebuggingState。')

add(838, 'GCoreComplexObjectPathDebug —— 对象路径调试可视化数据。')

add(839, 'GCoreObjectHandlePackageDebugData —— 对象句柄调试可视化数据。')

add(840, 'GCoreDebuggingState —— 调试可视化状态。')

add(842, '说明：@return 是否在跑 cook-on-the-fly（边跑边 cook）。')

add(843, 'IsRunningCookOnTheFly —— 是否在跑 cook-on-the-fly。')

add(845, '命名空间 UE。')

add(846, 'IsUsingZenPakFileStreaming —— 是否使用 Zen 的 pak 文件流式加载。')

add(847, '命名空间结束。')

add(849, '说明：用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。')

add(850, '目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。')

add(851, '说明结束。')

add(853, '标记 IsThisNotNull 已废弃（5.6）。')

add(854, 'IsThisNotNull —— 检查 this 是否非空（已废弃）。')

rep[1] = '// 版权所有 Epic Games, Inc. 保留所有权利。'
rep[54] = '// 临时日志分类，一般不要把用到它的代码提交进仓库'
rep[57] = '// 平台专属日志分类，在这里声明是为了方便任何地方直接使用'
rep[58] = '// 需要再套一层宏，才能把「宏」当参数传给「宏」'
rep[105] = '// #crashReport: 2014-08-19 合并成一个，重构。'
rep[118] = '\t/** 让已有引用失效。调用后不要再使用 FCoreTexts。 */'
rep[121] = '\t// 禁止拷贝'
rep[177] = '\t/** 是否通过 UELibrary 初始化。若为 false，其他字段的值都不应被采信。 */'
rep[181] = '\t/** 把引擎嵌入到哪个窗口句柄 */'
rep[184] = '\t/** 覆盖后的嵌入视口宽度 */'
rep[187] = '\t/** 覆盖后的嵌入视口高度 */'
rep[191] = '/** 把 UE 当库使用时的设置 */'
rep[200] = '/** 为 true 时，即使跑 commandlet 也初始化 RHI 并搭渲染场景。 */'
rep[203] = '/** 为 true 时，即使跑 commandlet 也初始化音频。 */'
rep[209] = '/**'
rep[210] = '*\t是否在编辑器里。'
rep[211] = '*\t注意：PIE（Play In Editor）时它仍为 true；那种情况应改用 GWorld->HasBegunPlay。'
rep[212] = '*/'
rep[217] = '/** 表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。'
rep[218] = 'UI 泵消息循环自然返回前，游戏线程做不了任何事。 */'
rep[221] = '/** 是否是第一次进入 UI 消息泵循环。 */'
rep[226] = '// 代码分析期间定义成变量，避免大量「常量 && 表达式」警告'
rep[244] = '/**'
rep[245] = '* 检查这个可执行文件是否在跑 commandlet（编辑器式环境下的自定义命令行处理）。'
rep[246] = '*/'
rep[256] = '/**'
rep[257] = '* 检查这个可执行文件是否在跑 cook commandlet。'
rep[258] = '*/'
rep[268] = '/**'
rep[269] = '* 检查这个可执行文件是否在跑 DLC cook commandlet。'
rep[270] = '*/'
rep[280] = '/**'
rep[281] = ' * 返回当前正在跑的 commandlet 的 UClass。'
rep[282] = ' */'
rep[292] = '/**'
rep[293] = ' * 返回命令行参数 -Run=<CommandletName> 指定的 commandlet 名字。'
rep[294] = ' * '
rep[295] = ' * 给需要在引擎启动早期、commandlet 还没初始化时就知道名字的代码用。'
rep[296] = ' * 能用 GetRunningCommandletClass()->GetName() 就优先用它。'
rep[297] = ' * '
rep[298] = ' * @return 最终会运行的 commandlet 名字；没有 commandlet 则返回空字符串。'
rep[299] = ' */'
rep[306] = '/**'
rep[307] = ' * 检查跑 commandlet 时是否也要初始化 RHI 并搭渲染场景。'
rep[308] = ' */'
rep[329] = '/**'
rep[330] = ' * 多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0。'
rep[331] = ' * 单进程运行时 GetMultiprocessId 返回 0。基于命令行判断，典型场景是多进程 cook 的 cookworker。'
rep[332] = ' * 需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。'
rep[333] = ' */'
rep[358] = '/**'
rep[359] = '* 下面这些在引擎首次启动时设置。'
rep[360] = '*/'
rep[362] = '/**'
rep[363] = '* 指定引擎是否作为构建机进程启动。'
rep[364] = '*/'
rep[367] = '/**'
rep[368] = '* 决定是否输出日志文本。为 true 时不输出任何日志。'
rep[369] = '*/'
rep[376] = '/**'
rep[377] = '* 设为 true 后，只允许在引擎 tick 开始时设置 RequestingExit，'
rep[378] = '* 可消除设置 RequestExit 时的未定义行为风险。'
rep[379] = '*'
rep[380] = '* 这个开关需要在所有平台/用法上验证过，才能改成默认值。'
rep[381] = '*/'
rep[386] = '/**'
rep[387] = ' * 引擎启动时所有初始模块完成各自加载阶段后返回 true；'
rep[388] = ' * 对应事件 FCoreDelegates::OnAllModuleLoadingPhasesComplete。'
rep[389] = ' */'
rep[396] = '/**'
rep[397] = ' * 检查是否收到退出请求，收到就把 GIsRequestingExit 置位。'
rep[398] = ' */'
rep[408] = '// 请求引擎在能安全退出时尽快退出'
rep[409] = '// ReasonString 不是可选的，要写清为什么请求退出'
rep[413] = '/**'
rep[414] = '*\t全局开关，控制是否显示屏幕上的警告/消息。'
rep[415] = '*\t控制台命令 DISABLEALLSCREENMESSAGES 关闭，ENABLEALLSCREENMESSAGES 打开，'
rep[416] = '*\tTOGGLEALLSCREENMESSAGES 切换。'
rep[417] = '*/'
rep[422] = '/* 是否正在截屏/录屏 */'
rep[431] = '/** 编辑器 ini 位置，按引擎版本存（所有项目共享），首次运行时在版本间迁移。 */'
rep[436] = '/** 编辑器「按项目」的 ini 文件，按项目存。 */'
rep[459] = '/** 游戏调试工具的 exec 处理器，支持 editactor 这类命令 */'
rep[462] = '/** 当前是否在异步加载代码路径里 */'
rep[465] = '/** 暂停异步包加载。 */'
rep[468] = '/** 恢复异步包加载。 */'
rep[471] = '/** 暂停异步包加载（引擎原文如此，实际是查询是否暂停）。 */'
rep[474] = '/** 异步加载是否使用独立线程 */'
rep[487] = '/** 返回当前激活的加载器类型，没有则返回默认值。 */'
rep[490] = '/** 暂停「异步 IO 完成后触发的纹理更新」。 */'
rep[493] = '/** 恢复「异步 IO 完成后触发的纹理更新」。 */'
rep[504] = '/** 编辑器当前是否正在加载包 */'
rep[515] = '/** cooker 当前是否正在加载包 */'
rep[518] = '/** GWorld 是否指向 PIE（Play In Editor）世界 */'
rep[529] = '/** 编辑器里表示当前正在处理的 PIE 世界上下文实例号 */'
rep[540] = '/** PIE 是否尝试从 PlayerStart 开始播放 */'
rep[544] = '/** 运行时是否要求纹理尺寸为 2 的幂 */'
rep[547] = '/** FPlatformTime::Seconds() 首次初始化的时间点（非常早） */'
rep[550] = '/** 引擎初始化时的系统时间。 */'
rep[553] = '/** 是否还在初始加载过程中。 */'
rep[556] = '/* 是否使用事件驱动加载器 */'
rep[559] = '/** 是否正在从 UClass 取 VTablePtr */'
rep[562] = '/** 稳定递增的帧计数器。 */'
rep[567] = '/** 上一次跑 GC 时的 GFrameCounter。 */'
rep[570] = '/** 输入采样时间（单位：CPU 周期）。 */'
rep[573] = '/** 每帧渲染场景前自增一次；分屏模式下所有视图共用一次自增（不是每视图一次）。 */'
rep[576] = '/** 待重命名；渲染线程版帧号请用 View.ViewFamily->FrameNumber 或从 GFrameTime 传下去。 */'
rep[579] = '/** 是否是游戏运行的第一个实例。 */'
rep[583] = '/** 一帧被判定为「卡顿」的阈值（毫秒）。 */'
rep[586] = '/** 保存压缩数据时，把数据切成多大的块 */'
rep[589] = '/** 主线程/游戏线程的线程 ID */'
rep[592] = '/** 渲染线程的线程 ID（如果有） */'
rep[596] = '/** Slate 线程的线程 ID（如果有） */'
rep[599] = '/** GGameThreadId 是否已经设置过 */'
rep[602] = '/** 决定做哪种 trace，NAME_None 表示不做。 */'
rep[605] = '/** 日志里时间怎么打印。 */'
rep[608] = '/** 日志里分类怎么打印。 */'
rep[611] = '/** 日志里详细级别怎么打印。 */'
rep[615] = '/** 轻量统计和 FGameThreadHitchHeartBeat 用它，在 Shipping 包里打印卡顿时的调用栈。 */'
rep[619] = '/** 统计系统是否要为 PIX 这类工具发命名事件。 */'
rep[622] = '/** 详细统计是否也生成外部性能分析器的命名事件。'
rep[623] = '* 线程睡眠/等待统计和超高频周期统计默认关闭。'
rep[624] = '* GCycleStatsShouldEmitNamedEvents 为 0 时本开关无效。'
rep[625] = '*/'
rep[628] = '/** 关闭一些会打断演示的警告和小功能 */'
rep[631] = '/** Core 包的名字。 */'
rep[632] = '//@包名迁移期标记，迁移完成后删掉双重检查'
rep[634] = '//@包名迁移期标记，迁移完成后删掉双重检查'
rep[637] = '/** 当前是否在跑单元测试。 */'
rep[640] = '/** 是否在主循环之外泵消息 */'
rep[643] = '/** 是否正在泵消息 */'
rep[646] = '/** 启用各种编辑器和 HMD 的 hack，让实验性 VR 编辑器功能可用（可能牺牲其他系统） */'
rep[651] = '/** 是否忽略已附加的调试器。 */'
rep[669] = '\tEParallelThread\t\t\t\t= 1 << 30, // 多个线程或任务参与时用（通常是并行 for）；会跳过具名线程标签的唯一性检查。'
rep[679] = '/**'
rep[680] = ' * 这个类用来给执行上下文（线程或任务）打标签，之后可在调用栈里查询状态。'
rep[681] = ' * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。'
rep[682] = ' *'
rep[683] = ' * @param InTag 要用的标签'
rep[684] = ' */'
rep[696] = '\t/**'
rep[697] = '\t * 清掉 EStaticInit 标签，让主 OS 线程没有激活标签。'
rep[698] = '\t * 这样从主线程调用 IsInGameThread() 这类函数才能正常工作。'
rep[699] = '\t */'
rep[702] = '\t/**'
rep[703] = '\t * 恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数'
rep[704] = '\t * 在检查线程状态（如 IsInGameThread()）时能正常工作。'
rep[705] = '\t */'
rep[709] = '\t/**'
rep[710] = '\t* 交换标签，只在「线程上下文在不同线程间迁移」时用。'
rep[711] = '\t*/'
rep[723] = '/**'
rep[724] = ' * 这个类也能给执行上下文打标签，但只在「还没打过标签」时才打。'
rep[725] = ' * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。'
rep[726] = ' *'
rep[727] = ' * @param InTag 要用的标签'
rep[728] = ' */'
rep[737] = '/**'
rep[738] = ' * 确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。'
rep[739] = ' *'
rep[740] = ' * @param CtorSignature 当前正在跑的构造函数签名，用来拼错误信息。'
rep[741] = ' */'
rep[746] = '/** @return 是否从游戏线程调用。 */'
rep[749] = '/** @return 是否从「并行 for 里的游戏线程」调用。 */'
rep[754] = '/** @return 是否从音频线程调用（不是「调用了音频函数的线程」）。 */'
rep[757] = '/** @return 是否从 Slate 线程调用（不是「调用了 Slate 函数的线程」）。 */'
rep[760] = '/** @return 是否从渲染线程调用；单线程渲染时任何线程都算。 */'
rep[763] = '/** @return 是否从渲染线程调用，或从任何非游戏线程调用；单线程渲染时游戏线程也算。 */'
rep[766] = '/** @return 是否从任一渲染线程调用；任何线程都可安全调用，不会断言。 */'
rep[769] = '/** @return 是否从渲染线程调用。 */'
rep[770] = '// 与 IsInRenderingThread 不同，单线程渲染时它永远返回 false；只有真正独立的渲染线程才返回 true。主要用于检查'
rep[773] = '/** @return 异步加载线程启用时看是否在该线程；否则看是否在游戏线程跑异步加载代码。 */'
rep[776] = '/** 渲染用的线程对象 */'
rep[780] = '/** @return RHI 线程是否在运行 */'
rep[783] = '/** @return 是否从 RHI 线程调用；单线程渲染时任何线程都算。 */'
rep[786] = '/** @return 是否从「标记为 RHI 任务」的线程调用。 */'
rep[793] = '/** 启动加载计时器 */'
rep[824] = '/** 帮助在调试器里可视化弱指针的数组 */'
rep[831] = '\t/** 帮助在调试器里可视化对象路径的数组 */'
rep[834] = '\t/** 帮助在调试器里可视化对象句柄的数组 */'
rep[842] = '/** @return 是否在跑 cook-on-the-fly（边跑边 cook）。 */'
rep[849] = '/* 用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。'
rep[850] = ' * 目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。'
rep[851] = ' */'


# ---- 多行块注释统一压成单行 // 注释，其余行清空 ----
BLOCKS = [
    (177, 178, '是否通过 UELibrary 初始化。若为 false，其他字段的值都不应被采信。'),
    (209, 212, '是否在编辑器里。注意：PIE（Play In Editor）时它仍为 true；那种情况应改用 GWorld->HasBegunPlay。'),
    (217, 219, '表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。UI 泵消息循环自然返回前，游戏线程做不了任何事。'),
    (244, 246, '检查这个可执行文件是否在跑 commandlet（编辑器式环境下的自定义命令行处理）。'),
    (256, 258, '检查这个可执行文件是否在跑 cook commandlet。'),
    (268, 270, '检查这个可执行文件是否在跑 DLC cook commandlet。'),
    (280, 282, '返回当前正在跑的 commandlet 的 UClass。'),
    (292, 299, '返回命令行参数 -Run=<CommandletName> 指定的 commandlet 名字。给需要在引擎启动早期、commandlet 还没初始化时就知道名字的代码用。能用 GetRunningCommandletClass()->GetName() 就优先用它。@return 最终会运行的 commandlet 名字；没有 commandlet 则返回空字符串。'),
    (306, 308, '检查跑 commandlet 时是否也要初始化 RHI 并搭渲染场景。'),
    (329, 333, '多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0。单进程运行时 GetMultiprocessId 返回 0。基于命令行判断，典型场景是多进程 cook 的 cookworker。需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。'),
    (358, 360, '下面这些在引擎首次启动时设置。'),
    (362, 364, '指定引擎是否作为构建机进程启动。'),
    (367, 369, '决定是否输出日志文本。为 true 时不输出任何日志。'),
    (376, 381, '设为 true 后，只允许在引擎 tick 开始时设置 RequestingExit，可消除设置 RequestExit 时的未定义行为风险。这个开关需要在所有平台/用法上验证过，才能改成默认值。'),
    (386, 389, '引擎启动时所有初始模块完成各自加载阶段后返回 true；对应事件 FCoreDelegates::OnAllModuleLoadingPhasesComplete。'),
    (396, 398, '检查是否收到退出请求，收到就把 GIsRequestingExit 置位。'),
    (413, 417, '全局开关，控制是否显示屏幕上的警告/消息。控制台命令 DISABLEALLSCREENMESSAGES 关闭，ENABLEALLSCREENMESSAGES 打开，TOGGLEALLSCREENMESSAGES 切换。'),
    (622, 625, '详细统计是否也生成外部性能分析器的命名事件。线程睡眠/等待统计和超高频周期统计默认关闭。GCycleStatsShouldEmitNamedEvents 为 0 时本开关无效。'),
    (679, 684, '这个类用来给执行上下文（线程或任务）打标签，之后可在调用栈里查询状态。通常给 IsInRenderingThread / IsInGameThread 这类函数用。@param InTag 要用的标签'),
    (696, 699, '清掉 EStaticInit 标签，让主 OS 线程没有激活标签。这样从主线程调用 IsInGameThread() 这类函数才能正常工作。'),
    (702, 705, '恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数在检查线程状态（如 IsInGameThread()）时能正常工作。'),
    (709, 711, '交换标签，只在「线程上下文在不同线程间迁移」时用。'),
    (723, 728, '这个类也能给执行上下文打标签，但只在「还没打过标签」时才打。通常给 IsInRenderingThread / IsInGameThread 这类函数用。@param InTag 要用的标签'),
    (737, 741, '确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。@param CtorSignature 当前正在跑的构造函数签名，用来拼错误信息。'),
    (849, 851, '用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。'),
]
for _a, _b, _txt in BLOCKS:
    rep[_a] = "// " + _txt
    for _i in range(_a + 1, _b + 1):
        rep[_i] = ""
        pre.pop(_i, None)

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

out = []
for i, line in enumerate(lines, start=1):
    if i in pre:
        out.append(pre[i])
    out.append(rep.get(i, line))

os.makedirs(os.path.dirname(RESTORE), exist_ok=True)
with io.open(RESTORE, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(out))
print('restore lines =', len(out))


def get_code(path, strip_tail):
    res = []
    in_block = False
    with io.open(path, 'r', encoding='utf-8', newline='') as f:
        for l in f.read().split('\n'):
            t = l.strip()
            if in_block:
                if '*/' in t:
                    in_block = False
                continue
            if t == '':
                continue
            if t.startswith('//'):
                continue
            if t.startswith('/*'):
                if '*/' not in t:
                    in_block = True
                continue
            if strip_tail and '//' in t:
                t = t.split('//')[0].strip()
            res.append(t)
    return res


for strip_tail in (False, True):
    o = get_code(RESTORE, strip_tail)
    e = get_code(ENGINE, strip_tail)
    diff = 0
    for i in range(max(len(o), len(e))):
        a = o[i] if i < len(o) else '<none>'
        b = e[i] if i < len(e) else '<none>'
        if a != b:
            diff += 1
            if diff <= 6:
                print('DIFF[%d] OURS: %s' % (i, a))
                print('DIFF[%d] ENG : %s' % (i, b))
    print('strip_tail=%s codeOurs=%d codeEngine=%d diff=%d' % (strip_tail, len(o), len(e), diff))