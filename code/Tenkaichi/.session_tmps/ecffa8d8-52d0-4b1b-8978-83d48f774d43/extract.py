import os, json, re, io

UPROJ = r"E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject"
ENGINE = r"D:\ue5\Epic Games\UE_5.4\Engine\Plugins"
PROJ = r"E:\ue5\game_design\code\Tenkaichi\Plugins"
OUT = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43\plugins.tsv"

def strip(s):
    s = re.sub(r"//[^\n]*", "", s)
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    s = re.sub(r",(\s*[}\]])", r"\1", s)
    return s

raw = io.open(UPROJ, encoding="utf-8").read()
data = json.loads(strip(raw))
plugins = data["Plugins"]

allp = {}
for root in (ENGINE, PROJ):
    for d, _, fs in os.walk(root):
        for f in fs:
            if f.endswith(".uplugin"):
                allp.setdefault(f[:-8], os.path.join(d, f))

rows = []
for i, p in enumerate(plugins, 1):
    name = p["Name"]
    en = p.get("Enabled")
    path = allp.get(name)
    if not path:
        rows.append([i, name, en, "MISSING", "", "", "", ""])
        continue
    try:
        j = json.loads(strip(io.open(path, encoding="utf-8").read()))
    except Exception as e:
        rows.append([i, name, en, "PARSEFAIL", "", "", str(e)[:40], path])
        continue
    cat = j.get("Category", "")
    desc = " ".join(str(j.get("Description", "")).split())
    mods = ", ".join("%s:%s:%s" % (m.get("Name"), m.get("Type"), m.get("LoadingPhase")) for m in j.get("Modules", []))
    ebd = j.get("EnabledByDefault", "")
    rows.append([i, name, en, "Engine" if path.startswith(ENGINE) else "Project", cat, desc, mods, str(ebd)])

with io.open(OUT, "w", encoding="utf-8") as f:
    for r in rows:
        f.write("|".join("" if x is None else str(x) for x in r) + "\n")
print("rows:", len(rows))
