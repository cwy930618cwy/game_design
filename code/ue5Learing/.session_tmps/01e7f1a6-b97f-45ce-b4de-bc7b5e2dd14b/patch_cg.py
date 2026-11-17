# -*- coding: utf-8 -*-
import io, re

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg.py'
s = io.open(p, encoding='utf-8').read()
s2 = re.sub(r"(\))\s*(add\(\d+, )", r"\1\n\n\2", s)
io.open(p, 'w', encoding='utf-8', newline='').write(s2)
print('changed =', s != s2)