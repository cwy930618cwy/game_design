# -*- coding: utf-8 -*-
import io, json

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\ipfl_data.json'
with io.open(p, 'r', encoding='utf-8-sig') as f:
    d = json.loads(f.read(), strict=False)

# 宏续行内部不能插注释（会截断宏定义），删掉 33~37 的 pre
removed = []
for k in ['33', '34', '35', '36', '37', '38']:
    if k in d.get('pre', {}):
        d['pre'].pop(k)
        removed.append('pre[%s]' % k)

# 块注释 25~27 压成单行，避免重复
for k in ['25', '26', '27']:
    d.get('pre', {}).pop(k, None)
    d.get('rep', {}).pop(k, None)
d['rep']['25'] = '// 包装底层文件系统，把每次调用都记进日志。'
d['rep']['26'] = ''
d['rep']['27'] = ''

with io.open(p, 'w', encoding='utf-8', newline='') as f:
    f.write(json.dumps(d, ensure_ascii=False, indent=2))

print('removed =', removed)