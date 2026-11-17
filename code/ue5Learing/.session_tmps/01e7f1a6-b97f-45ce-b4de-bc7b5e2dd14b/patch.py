# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_pf.py'
s = io.open(p, encoding='utf-8').read()

bad = "pre[k] = '// ' + vpre[113]"
i = s.index(bad)
j = s.index('\n', i)
line = s[i:j]
rest = line[len("pre[k] = '// ' + v"):]
s = s[:i] + "pre[k] = '// ' + v" + '\n\n' + rest + s[j:]
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')