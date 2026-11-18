# -*- coding: utf-8 -*-
import io, json

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\icm_data1.json'
with io.open(p, 'r', encoding='utf-8-sig') as f:
    d = json.loads(f.read(), strict=False)

# 536 行是 #if TRACK_CONSOLE_FIND_COUNT，pre 注释应保留，rep 不应存在
removed = []
if '536' in d.get('rep', {}):
    d['rep'].pop('536')
    removed.append('rep[536]')

with io.open(p, 'w', encoding='utf-8', newline='') as f:
    f.write(json.dumps(d, ensure_ascii=False, indent=2))

print('removed =', removed)