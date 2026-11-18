# -*- coding: utf-8 -*-
import io, json

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\membase_data.json'
with io.open(p, 'r', encoding='utf-8-sig') as f:
    d = json.loads(f.read(), strict=False)

r = d['rep']
r['46'] = 'UE_DEPRECATED(5.6, "Consider using FMemory::Malloc instead of accessing GMalloc directly.")'
r['223'] = '\t\tAr.Logf( TEXT("Allocator Stats for %s: (not implemented)" ), GetDescriptiveName() );'
r['250'] = '\t\treturn TEXT("Unspecified allocator");'

with io.open(p, 'w', encoding='utf-8', newline='') as f:
    f.write(json.dumps(d, ensure_ascii=False, indent=2))
print('ok')