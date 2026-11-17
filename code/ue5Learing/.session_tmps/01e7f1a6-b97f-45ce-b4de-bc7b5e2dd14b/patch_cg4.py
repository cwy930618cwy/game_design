# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg2.py'
s = io.open(p, encoding='utf-8').read()

# 1) #endif 行尾是宏名，按经验 06 不翻译，恢复原文
s = s.replace(
    "T[237] = '#endif // WITH_EDITORONLY_DATA（仅编辑器数据）'",
    "T[237] = '#endif // WITH_EDITORONLY_DATA'")
s = s.replace(
    "T[654] = '#endif // #if !UE_BUILD_SHIPPING（非 Shipping 构建）'",
    "T[654] = '#endif // #if !UE_BUILD_SHIPPING'")

# 2) 329~335 块注释：5 行内容 + 收尾
old_start = s.index("T[329] = ")
old_end = s.index("T[358] = ")
new_block = """T[329] = '/**'
T[330] = ' * 多进程运行时，导演进程 MultiprocessId 为 0，工作进程大于 0。'
T[331] = ' * 单进程运行时 GetMultiprocessId 返回 0。'
T[332] = ' * 基于命令行判断，典型场景是多进程 cook 的 cookworker。'
T[333] = ' * 需要「每个进程写独立文件」的系统，应把这个 id 拼进文件名。'
T[334] = ' * （原文：Systems that need to write a separate file per running process should use this id as part of their file descriptor.）'
T[335] = ' */'
"""
s = s[:old_start] + new_block + s[old_end:]
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')