# -*- coding: utf-8 -*-
import io, json

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\llm_data.json'
with io.open(p, 'r', encoding='utf-8-sig') as f:
    d = json.loads(f.read(), strict=False)

# 930 / 997 是代码行（bool bEnabled = false;），rep 必须保留代码
d['rep']['930'] = '\tbool bEnabled = false;  // 必须是 uint8 而不是位域，以减少 ALU 操作'
d['rep']['997'] = '\tbool bEnabled = false; // 必须是 uint8 而不是位域，以减少 ALU 操作'

with io.open(p, 'w', encoding='utf-8', newline='') as f:
    f.write(json.dumps(d, ensure_ascii=False, indent=2))

print('fixed 930 / 997')