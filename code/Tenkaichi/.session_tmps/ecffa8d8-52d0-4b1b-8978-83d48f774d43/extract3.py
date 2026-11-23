import os, json, re, io

UPROJ = r"E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject"
ENGINE = r"D:\ue5\Epic Games\UE_5.4\Engine\Plugins"
PROJ = r"E:\ue5\game_design\code\Tenkaichi\Plugins"
LYRA = r"E:\ue5\LyraStarterGame"
TMP = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43"

def strip(s):
    out = []
    for line in s.splitlines():
        t = line.strip()
        if t.startswith("//"):
            continue
        out.append(line)
    s = "\n".join(out)
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    s = re.sub(r",(\s*[}\]])", r"\1", s)
    return s

def rd(p):
    return io.open(p, encoding="utf-8-sig", errors="replace").read()

data = json.loads(strip(rd(UPROJ)))
plugins = data["Plugins"]

allp = {}
for root in (ENGINE, PROJ):
    for d, _, fs in os.walk(root):
        for f in fs:
            if f.endswith(".uplugin"):
                allp.setdefault(f[:-8], os.path.join(d, f))

lyra_texts = []
for d, dirs, fs in os.walk(LYRA):
    dirs[:] = [x for x in dirs if x not in ("Intermediate","Binaries","Saved","DerivedDataCache",".git")]
    for f in fs:
        if f.endswith((".cs",".cpp",".h",".ini",".uproject",".uplugin")):
            p = os.path.join(d,f)
            try:
                lyra_texts.append((p.replace(LYRA+"\\",""), rd(p)))
            except Exception:
                pass

rows = []
for i, p in enumerate(plugins, 1):
    name = p["Name"]; en = p.get("Enabled")
    path = allp.get(name)
    d = dict(idx=i, name=name, en=en, src="MISSING", cat="", desc="", mods=[], ebd="", path="", lyra=[])
    if path:
        d["src"] = "Engine" if path.startswith(ENGINE) else "Project"
        d["path"] = path
        try:
            j = json.loads(strip(rd(path)))
            d["cat"] = j.get("Category","") or ""
            d["desc"] = " ".join(str(j.get("Description","")).split())
            d["mods"] = [dict(name=m.get("Name"), type=m.get("Type"), phase=m.get("LoadingPhase")) for m in j.get("Modules",[])]
            d["ebd"] = j.get("EnabledByDefault","")
        except Exception as e:
            d["cat"] = "PARSEFAIL:"+str(e)[:40]
    hits = []
    for rel, txt in lyra_texts:
        if ('"'+name+'"') in txt:
            hits.append((rel, txt.count('"'+name+'"')))
    d["lyra"] = hits[:10]
    rows.append(d)

with io.open(os.path.join(TMP,"plugins.json"), "w", encoding="utf-8") as f:
    json.dump(rows, f, ensure_ascii=False, indent=1)

for r in rows:
    mods = "; ".join("%s(%s)" % (m["name"], m["type"]) for m in r["mods"])
    print("%2d|%s|%s|%s|%s|%s|%s" % (r["idx"], r["name"], r["src"], r["cat"], r["ebd"], mods[:70], r["desc"][:90]))
