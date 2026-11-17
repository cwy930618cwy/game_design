# -*- coding: utf-8 -*-
import io, re

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_pf.py'
s = io.open(p, encoding='utf-8').read()

# 在 ' 紧跟 pre[ 的位置插入换行
s2 = re.sub(r"(')\s*(pre\[\d+\] = )", r"\1\n\n\2", s)
io.open(p, 'w', encoding='utf-8', newline='').write(s2)
print('patched, changed =', s != s2)