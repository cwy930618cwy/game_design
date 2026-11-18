# -*- coding: utf-8 -*-
import io, json, sys

RESTORE = sys.argv[1]
MD      = sys.argv[2]
MAP     = sys.argv[3]
SEG     = sys.argv[4]

with io.open(RESTORE, 'r', encoding='utf-8', newline='') as f:
    ours = f.read().split('\n')
with io.open(MAP, 'r', encoding='utf-8') as f:
    emap = {int(k): v for k, v in json.load(f).items()}
with io.open(SEG, 'r', encoding='utf-8-sig') as f:
    cfg = json.load(f)

md = []
md.append(cfg['title'])
md.append('')
for line in cfg['header']:
    md.append(line)
md.append('')
md.append('---')
md.append('')
md.append('## 一、这个文件解决什么问题（为什么）')
md.append('')
for line in cfg['why']:
    md.append(line)
md.append('')
md.append('---')
md.append('')
md.append('## 二、逐段代码（每行上方都有中文注释）')
md.append('')

for seg in cfg['segments']:
    title = seg['title']
    s = seg['start']
    e = seg['end']
    md.append('### %s（第 %d~%d 行）' % (title, s, e))
    md.append('')
    md.append('```cpp')
    start = emap[s - 1] + 1 if (s - 1) in emap else 1
    end = emap[e]
    for ln in range(start, end + 1):
        md.append(ours[ln - 1])
    md.append('```')
    md.append('')
    md.append('**本段新元素**')
    md.append('')
    for ne in seg['new']:
        md.append('- ' + ne)
    md.append('')
    md.append('**这段在干什么**')
    md.append('')
    md.append('| 行 | 干什么 |')
    md.append('|---|---|')
    for row in seg['rows']:
        md.append('| %s | %s |' % (row[0], row[1]))
    md.append('')
    md.append('> 类比：' + seg['analogy'])
    md.append('')

md.append('---')
md.append('')
md.append('## 三、速查：这段代码在干什么（一句话版）')
md.append('')
md.append('| 段 | 行 | 一句话 |')
md.append('|---|---|---|')
for seg in cfg['segments']:
    name = seg['title'].split('、', 1)[1]
    md.append('| %s | %d~%d | %s |' % (name, seg['start'], seg['end'], name))
md.append('')
md.append('---')
md.append('')
md.append('## 四、C++ 语法小抄（本文件用到的）')
md.append('')
md.append('| 语法 | 含义 | 例子 |')
md.append('|---|---|---|')
for row in cfg['syntax']:
    md.append('| %s | %s | %s |' % (row[0], row[1], row[2]))
md.append('')
md.append('---')
md.append('')
md.append('## 五、我们这版 vs 引擎的差异')
md.append('')
md.append('| 项 | 引擎 | 我们 | 说明 |')
md.append('|---|---|---|---|')
for row in cfg['diff']:
    md.append('| %s | %s | %s | %s |' % (row[0], row[1], row[2], row[3]))
md.append('')
md.append('---')
md.append('')
md.append('## 待确认')
md.append('')
for i, line in enumerate(cfg['todo'], start=1):
    md.append('%d. %s' % (i, line))
md.append('')
md.append('---')
md.append('')
md.append('## 源码依据')
md.append('')
for line in cfg['src']:
    md.append('- ' + line)

with io.open(MD, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(md))
print('md lines =', len(md))