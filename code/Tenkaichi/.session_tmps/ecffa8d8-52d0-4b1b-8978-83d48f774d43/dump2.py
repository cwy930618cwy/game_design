import json, io
p = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43\plugins.json"
rows = json.load(io.open(p, encoding="utf-8"))
for r in rows[40:]:
    mods = "; ".join("%s(%s)" % (m["name"], m["type"]) for m in r["mods"])
    print("%2d|%s|%s|%s|%s|%s" % (r["idx"], r["name"], r["src"], r["cat"], mods[:60], r["desc"][:100]))
    if r["lyra"]:
        print("      lyra:", ", ".join("%s x%d" % (a,b) for a,b in r["lyra"][:4]))
