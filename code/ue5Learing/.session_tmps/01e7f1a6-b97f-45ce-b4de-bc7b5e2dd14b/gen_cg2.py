# -*- coding: utf-8 -*-
import io, os

ENGINE  = r'D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'
RESTORE = r'E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Public\CoreGlobals.h'

T = {}

T[1]   = '// 版权所有 Epic Games, Inc. 保留所有权利。'
T[54]  = '// 临时日志分类，一般不要把用到它的代码提交进仓库'
T[55]  = 'CORE_API DECLARE_LOG_CATEGORY_EXTERN(LogTemp, Log, All);'
T[57]  = '// 平台专属日志分类，在这里声明是为了方便任何地方直接使用'
T[58]  = '// 需要再套一层宏，才能把「宏」当参数传给「宏」'
T[105] = '// #crashReport: 2014-08-19 合并成一个，重构。'
T[118] = '\t/** 让已有引用失效。调用后不要再使用 FCoreTexts。 */'
T[121] = '\t// 禁止拷贝'
T[144] = '/** 为 true 时，这个可执行文件能运行所有游戏（游戏以 DLL 形式加载）。 */'
T[147] = '/** 从游戏里保存时，这个开关允许加载「仅编辑器」属性。 */'
T[150] = '/** 禁止加载不在脚本文件里的对象；脚本编译期间用。 */'
T[153] = '/** 构造对象时，在主机平台走快速路径…… */'
T[156] = '/** 允许 AActor 在编辑器里从特定入口执行脚本，例如跑构造脚本时。 */'
T[159] = '/** 给 CDO 里新实例化的组件强制使用模板名。 */'
T[162] = '/** 蓝图编译后是否正在做垃圾回收。 */'
T[165] = '/** 是否正在重建蓝图实例；cook 后的构建里永远不该为 true。 */'
T[168] = '/** Actor 和对象是否正在被重新实例化。 */'
T[171] = '/** 用于冲刷资源流式加载的辅助函数。 */'
T[174] = '/** UE-as-a-library（把引擎当库嵌入）功能用的设置。 */'
T[177] = '\t/** 是否通过 UELibrary 初始化。若为 false，其他字段的值都不应被采信。'
T[178] = '\t    （原文第二行：none of the other field values should be acknowledged.） */'
T[181] = '\t/** 把引擎嵌入到哪个窗口句柄 */'
T[184] = '\t/** 覆盖后的嵌入视口宽度 */'
T[187] = '\t/** 覆盖后的嵌入视口高度 */'
T[191] = '/** 把 UE 当库使用时的设置 */'
T[200] = '/** 为 true 时，即使跑 commandlet 也初始化 RHI 并搭渲染场景。 */'
T[203] = '/** 为 true 时，即使跑 commandlet 也初始化音频。 */'
T[209] = '/**'
T[210] = '*\t是否在编辑器里。'
T[211] = '*\t注意：PIE（Play In Editor）时它仍为 true；那种情况应改用 GWorld->HasBegunPlay。'
T[212] = '*/'
T[217] = '/** 表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。'
T[218] = 'UI 泵消息循环自然返回前，游戏线程做不了任何事。'
T[219] = '*/'
T[221] = '/** 是否是第一次进入 UI 消息泵循环。 */'
T[226] = '// 代码分析期间定义成变量，避免大量「常量 && 表达式」警告'
T[237] = '#endif // WITH_EDITORONLY_DATA'
T[244] = '/**'
T[245] = '* 检查这个可执行文件是否在跑 commandlet（编辑器式环境下的自定义命令行处理）。'
T[246] = '*/'
T[256] = '/**'
T[257] = '* 检查这个可执行文件是否在跑 cook commandlet。'
T[258] = '*/'
T[268] = '/**'
T[269] = '* 检查这个可执行文件是否在跑 DLC cook commandlet。'
T[270] = '*/'
T[280] = '/**'
T[281] = ' * 返回当前正在跑的 commandlet 的 UClass。'
T[282] = ' */'
T[292] = '/**'
T[293] = ' * 返回命令行参数 -Run=<CommandletName> 指定的 commandlet 名字。'
T[294] = ' * 参数形式：-Run=<CommandletName>。'
T[295] = ' * '
T[296] = ' * 给需要在引擎启动早期、commandlet 还没初始化时就知道名字的代码用。'
T[297] = ' * （原文：early on during engine startup, before the commandlet has been initialized.）'
T[298] = ' * 能用 GetRunningCommandletClass()->GetName() 就优先用它。'
T[299] = ' * （原文：If possible, code should prefer calling ... instead.）'
T[300] = ' * '
T[301] = ' * @return 最终会运行的 commandlet 名字。'
T[302] = ' * 没有 commandlet 则返回空字符串。'
T[303] = ' */'
T[306] = '/**'
T[307] = ' * 检查跑 commandlet 时是否也要初始化 RHI 并搭渲染场景。'
T[308] = ' */'
T[329] = '/**'
T[330] = ' * 多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0。'
T[331] = ' * 单进程运行时 GetMultiprocessId 返回 0。'
T[332] = ' * 基于命令行判断，典型场景是多进程 cook 的 cookworker。'
T[333] = ' * 需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。'
T[334] = ' * （原文：Systems that need to write a separate file per running process should use this id as part of their file descriptor.）'
T[335] = ' */'
T[358] = '/**'
T[359] = '* 下面这些在引擎首次启动时设置。'
T[360] = '*/'
T[362] = '/**'
T[363] = '* 指定引擎是否作为构建机进程启动。'
T[364] = '*/'
T[367] = '/**'
T[368] = '* 决定是否输出日志文本。为 true 时不输出任何日志。'
T[369] = '*/'
T[376] = '/**'
T[377] = '* 设为 true 后，只允许在引擎 tick 开始时设置 RequestingExit，'
T[378] = '* 可消除设置 RequestExit 时的未定义行为风险。'
T[379] = '*'
T[380] = '* 这个开关需要在所有平台/用法上验证过，才能改成默认值。'
T[381] = '*/'
T[386] = '/**'
T[387] = ' * 引擎启动时所有初始模块完成各自加载阶段后返回 true；'
T[388] = ' * 对应事件 FCoreDelegates::OnAllModuleLoadingPhasesComplete。'
T[389] = ' */'
T[396] = '/**'
T[397] = ' * 检查是否收到退出请求，收到就把 GIsRequestingExit 置位。'
T[398] = ' */'
T[408] = '// 请求引擎在能安全退出时尽快退出'
T[409] = '// ReasonString 不是可选的，要写清为什么请求退出'
T[413] = '/**'
T[414] = '*\t全局开关，控制是否显示屏幕上的警告/消息。'
T[415] = '*\t控制台命令 DISABLEALLSCREENMESSAGES 关闭。'
T[416] = '*\t控制台命令 ENABLEALLSCREENMESSAGES 打开。'
T[417] = '*\t控制台命令 TOGGLEALLSCREENMESSAGES 切换。'
T[418] = '*/'
T[422] = '/* 是否正在截屏/录屏 */'
T[431] = '/** 编辑器 ini 位置，按引擎版本存（所有项目共享），首次运行时在版本间迁移。 */'
T[436] = '/** 编辑器「按项目」的 ini 文件，按项目存。 */'
T[459] = '/** 游戏调试工具的 exec 处理器，支持 editactor 这类命令 */'
T[462] = '/** 当前是否在异步加载代码路径里 */'
T[465] = '/** 暂停异步包加载。 */'
T[468] = '/** 恢复异步包加载。 */'
T[471] = '/** 暂停异步包加载（引擎原文如此，实际是查询是否暂停）。 */'
T[474] = '/** 异步加载是否使用独立线程 */'
T[487] = '/** 返回当前激活的加载器类型，没有则返回默认值。 */'
T[490] = '/** 暂停「异步 IO 完成后触发的纹理更新」。 */'
T[493] = '/** 恢复「异步 IO 完成后触发的纹理更新」。 */'
T[504] = '/** 编辑器当前是否正在加载包 */'
T[515] = '/** cooker 当前是否正在加载包 */'
T[518] = '/** GWorld 是否指向 PIE（Play In Editor）世界 */'
T[529] = '/** 编辑器里表示当前正在处理的 PIE 世界上下文实例号 */'
T[540] = '/** PIE 是否尝试从 PlayerStart 开始播放 */'
T[544] = '/** 运行时是否要求纹理尺寸为 2 的幂 */'
T[547] = '/** FPlatformTime::Seconds() 首次初始化的时间点（非常早） */'
T[550] = '/** 引擎初始化时的系统时间。 */'
T[553] = '/** 是否还在初始加载过程中。 */'
T[556] = '/* 是否使用事件驱动加载器 */'
T[559] = '/** 是否正在从 UClass 取 VTablePtr */'
T[562] = '/** 稳定递增的帧计数器。 */'
T[567] = '/** 上一次跑 GC 时的 GFrameCounter。 */'
T[570] = '/** 输入采样时间（单位：CPU 周期）。 */'
T[573] = '/** 每帧渲染场景前自增一次；分屏模式下所有视图共用一次自增（不是每视图一次）。 */'
T[576] = '/** 待重命名；渲染线程版帧号请用 View.ViewFamily->FrameNumber 或从 GFrameTime 传下去。 */'
T[579] = '/** 是否是游戏运行的第一个实例。 */'
T[583] = '/** 一帧被判定为「卡顿」的阈值（毫秒）。 */'
T[586] = '/** 保存压缩数据时，把数据切成多大的块 */'
T[589] = '/** 主线程/游戏线程的线程 ID */'
T[592] = '/** 渲染线程的线程 ID（如果有） */'
T[596] = '/** Slate 线程的线程 ID（如果有） */'
T[599] = '/** GGameThreadId 是否已经设置过 */'
T[602] = '/** 决定做哪种 trace，NAME_None 表示不做。 */'
T[605] = '/** 日志里时间怎么打印。 */'
T[608] = '/** 日志里分类怎么打印。 */'
T[611] = '/** 日志里详细级别怎么打印。 */'
T[615] = '/** 轻量统计和 FGameThreadHitchHeartBeat 用它，在 Shipping 包里打印卡顿时的调用栈。 */'
T[619] = '/** 统计系统是否要为 PIX 这类工具发命名事件。 */'
T[622] = '/** 详细统计是否也生成外部性能分析器的命名事件。'
T[623] = '* 线程睡眠/等待统计和超高频周期统计默认关闭。'
T[624] = '* GCycleStatsShouldEmitNamedEvents 为 0 时本开关无效。'
T[625] = '*/'
T[628] = '/** 关闭一些会打断演示的警告和小功能 */'
T[631] = '/** Core 包的名字。 */'
T[632] = '//@包名迁移期标记，迁移完成后删掉双重检查'
T[634] = '//@包名迁移期标记，迁移完成后删掉双重检查'
T[637] = '/** 当前是否在跑单元测试。 */'
T[640] = '/** 是否在主循环之外泵消息 */'
T[643] = '/** 是否正在泵消息 */'
T[646] = '/** 启用各种编辑器和 HMD 的 hack，让实验性 VR 编辑器功能可用（可能牺牲其他系统） */'
T[651] = '/** 是否忽略已附加的调试器。 */'
T[654] = '#endif // #if !UE_BUILD_SHIPPING'
T[669] = '\tEParallelThread\t\t\t\t= 1 << 30, // 多个线程或任务参与时用（通常是并行 for）；会跳过具名线程标签的唯一性检查。'
T[679] = '/**'
T[680] = ' * 这个类用来给执行上下文（线程或任务）打标签，之后可在调用栈里查询状态。'
T[681] = ' * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。'
T[682] = ' *'
T[683] = ' * @param InTag 要用的标签'
T[684] = ' */'
T[696] = '\t/**'
T[697] = '\t * 清掉 EStaticInit 标签，让主 OS 线程没有激活标签。'
T[698] = '\t * 这样从主线程调用 IsInGameThread() 这类函数才能正常工作。'
T[699] = '\t */'
T[702] = '\t/**'
T[703] = '\t * 恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数'
T[704] = '\t * 在检查线程状态（如 IsInGameThread()）时能正常工作。'
T[705] = '\t * （原文：state (from functions like IsInGameThread()).）'
T[706] = '\t */'
T[709] = '\t/**'
T[710] = '\t* 交换标签，只在「线程上下文在不同线程间迁移」时用。'
T[711] = '\t*/'
T[723] = '/**'
T[724] = ' * 这个类也能给执行上下文打标签，但只在「还没打过标签」时才打。'
T[725] = ' * 通常给 IsInRenderingThread / IsInGameThread 这类函数用。'
T[726] = ' *'
T[727] = ' * @param InTag 要用的标签'
T[728] = ' */'
T[737] = '/**'
T[738] = ' * 确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。'
T[739] = ' *'
T[740] = ' * @param CtorSignature 当前正在跑的构造函数签名，'
T[741] = ' *\t\t用来拼出正确的错误信息。'
T[742] = ' * （原文：construct proper error message.）'
T[743] = ' */'
T[746] = '/** @return 是否从游戏线程调用。 */'
T[749] = '/** @return 是否从「并行 for 里的游戏线程」调用。 */'
T[754] = '/** @return 是否从音频线程调用（不是「调用了音频函数的线程」）。 */'
T[757] = '/** @return 是否从 Slate 线程调用（不是「调用了 Slate 函数的线程」）。 */'
T[760] = '/** @return 是否从渲染线程调用；单线程渲染时任何线程都算。 */'
T[763] = '/** @return 是否从渲染线程调用，或从任何非游戏线程调用；单线程渲染时游戏线程也算。 */'
T[766] = '/** @return 是否从任一渲染线程调用；任何线程都可安全调用，不会断言。 */'
T[769] = '/** @return 是否从渲染线程调用。 */'
T[770] = '// 与 IsInRenderingThread 不同，单线程渲染时它永远返回 false；只有真正独立的渲染线程才返回 true。主要用于检查'
T[773] = '/** @return 异步加载线程启用时看是否在该线程；否则看是否在游戏线程跑异步加载代码。 */'
T[776] = '/** 渲染用的线程对象 */'
T[780] = '/** @return RHI 线程是否在运行 */'
T[783] = '/** @return 是否从 RHI 线程调用；单线程渲染时任何线程都算。 */'
T[786] = '/** @return 是否从「标记为 RHI 任务」的线程调用。 */'
T[793] = '/** 启动加载计时器 */'
T[824] = '/** 帮助在调试器里可视化弱指针的数组 */'
T[831] = '\t/** 帮助在调试器里可视化对象路径的数组 */'
T[834] = '\t/** 帮助在调试器里可视化对象句柄的数组 */'
T[842] = '/** @return 是否在跑 cook-on-the-fly（边跑边 cook）。 */'
T[849] = '/* 用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。'
T[850] = ' * 目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。'
T[851] = ' * （原文：on undefined behavior.）'
T[852] = ' */'

with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:
    lines = f.read().split('\n')

out = []
for i, line in enumerate(lines, start=1):
    out.append(T.get(i, line))

os.makedirs(os.path.dirname(RESTORE), exist_ok=True)
with io.open(RESTORE, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(out))
print('restore lines =', len(out), 'engine lines =', len(lines))


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