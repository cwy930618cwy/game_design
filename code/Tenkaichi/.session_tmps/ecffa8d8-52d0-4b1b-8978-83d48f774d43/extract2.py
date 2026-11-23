import os, json, re, io

UPROJ = r"E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject"
ENGINE = r"D:\ue5\Epic Games\UE_5.4\Engine\Plugins"
PROJ = r"E:\ue5\game_design\code\Tenkaichi\Plugins"
LYRA = r"E:\ue5\LyraStarterGame"
TMP = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43"

def strip(s):
    s = re.sub(r"//[^\n]*", "", s)
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

# 采集 Lyra 引用情况
lyra_files = []
for d, dirs, fs in os.walk(LYRA):
    dirs[:] = [x for x in dirs if x not in ("Intermediate","Binaries","Saved","DerivedDataCache",".git")]
    for f in fs:
        if f.endswith((".cs",".cpp",".h",".ini",".uproject",".uplugin")):
            lyra_files.append(os.path.join(d,f))

lyra_texts = []
for p in lyra_files:
    try:
        lyra_texts.append((p.replace(LYRA+"\\",""), rd(p)))
    except Exception:
        pass

rows = []
for i, p in enumerate(plugins, 1):
    name = p["Name"]
    en = p.get("Enabled")
    path = allp.get(name)
    if not path:
        rows.append(dict(idx=i, name=name, en=en, src="MISSING", cat="", desc="", mods="",
                         ebd="", path="", lyra=[]))
        continue
    try:
        j = json.loads(strip(rd(path)))
    except Exception as e:
        j = {}
    cat = j.get("Category","")
    desc = " ".join(str(j.get("Description","")).split())
    mods = [dict(name=m.get("Name"), type=m.get("Type"), phase=m.get("LoadingPhase")) for m in j.get("Modules",[])]
    # Lyra 引用
    hits = []
    for rel, txt in lyra_texts:
        if name in txt:
            # 精确点：查找 "Name": "X" 或在 Build.cs 中 "X"
            if ('"'+name+'"') in txt or ("'"+name+"'") in txt:
                cnt = txt.count('"'+name+'"')
                hits.append((rel, cnt))
    rows.append(dict(idx=i, name=name, en=en,
                     src="Engine" if path.startswith(ENGINE) else "Project",
                     cat=cat, desc=desc, mods=mods,
                     ebd=j.get("EnabledByDefault",""), path=path, lyra=hits[:12]))

with io.open(os.path.join(TMP,"plugins.json"), "w", encoding="utf-8") as f:
    json.dump(rows, f, ensure_ascii=False, indent=1)
print("ok", len(rows))
for r in rows:
    print(r["idx"], r["name"], r["src"], r["cat"], "| lyra refs:", len(r["lyra"]))
