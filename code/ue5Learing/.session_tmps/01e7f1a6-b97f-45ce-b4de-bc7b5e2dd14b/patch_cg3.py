# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg2.py'
s = io.open(p, encoding='utf-8').read()

old_start = s.index("T[292] = ")
old_end = s.index("T[306] = ")
new_block = """T[292] = '/**'
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
"""
s = s[:old_start] + new_block + s[old_end:]
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')