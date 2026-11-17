# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_md2.py'
s = io.open(p, encoding='utf-8').read()
s = s.replace("'))segments.append((", "'))\n\nsegments.append((")
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('ok')