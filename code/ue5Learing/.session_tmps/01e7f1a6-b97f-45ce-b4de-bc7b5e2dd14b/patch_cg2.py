# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg.py'
s = io.open(p, encoding='utf-8').read()

blocks = [
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

lines = []
lines.append('')
lines.append('# ---- 多行块注释统一压成单行 // 注释，其余行清空 ----')
lines.append('BLOCKS = [')
for a, b, txt in blocks:
    lines.append('    (%d, %d, %r),' % (a, b, txt))
lines.append(']')
lines.append('for _a, _b, _txt in BLOCKS:')
lines.append('    rep[_a] = "// " + _txt')
lines.append('    for _i in range(_a + 1, _b + 1):')
lines.append('        rep[_i] = ""')
lines.append('        pre.pop(_i, None)')
lines.append('')

marker = "with io.open(ENGINE, 'r', encoding='utf-8', newline='') as f:"
assert marker in s
s = s.replace(marker, '\n'.join(lines) + '\n' + marker, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')