import json, io
p = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43\plugins.json"
rows = json.load(io.open(p, encoding="utf-8"))
for r in rows[:40]:
    if r["lyra"]:
        print("%2d %s -> %s" % (r["idx"], r["name"], ", ".join("%s x%d" % (a,b) for a,b in r["lyra"][:6])))
