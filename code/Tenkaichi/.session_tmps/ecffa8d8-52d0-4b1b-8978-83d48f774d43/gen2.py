import io, json, os

TMP = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43"
OUTDIR = json.loads(io.open(os.path.join(TMP, "paths.json"), encoding="utf-8-sig").read())["outdir"]

def rd(p):
    return io.open(p, encoding="utf-8-sig").read()

meta = {r["idx"]: r for r in json.loads(rd(os.path.join(TMP, "plugins.json")))}
data = json.loads(rd(os.path.join(TMP, "data.json")))
L = json.loads(rd(os.path.join(TMP, "labels.json")))

os.makedirs(OUTDIR, exist_ok=True)

INDEX = []

for k in sorted(data, key=lambda x: int(x)):
    i = int(k)
    d = data[k]
    m = meta[i]
    name = m["name"]
    en = "true" if m["en"] is True else ("false" if m["en"] is False else str(m["en"]))
    src = m["src"]
    cat = m["cat"] or L["none_val"]
    ebd = m["ebd"]
    mods = m["mods"]
    desc = m["desc"] or L["none_val"]

    if src == "MISSING":
        srcline = L["src_missing"]
    elif src == "Engine":
        srcline = L["src_engine"]
    else:
        srcline = L["src_project"].format(name=name)

    if mods:
        modline = ", ".join("`%s`(%s)" % (x["name"], x["type"]) for x in mods)
    else:
        modline = L["none_mod"]

    lyra_lines = ["- `%s` (x%d)" % (rel, cnt) for rel, cnt in m["lyra"]]
    lyra_txt = "\n".join(lyra_lines) if lyra_lines else L["none_lyra"]

    b = []
    b.append(L["title"].format(i="%02d" % i, name=name))
    b.append("")
    b.append(L["meta_pos"].format(i=i))
    b.append(L["meta_src"].format(src=srcline))
    if ebd in ("", None):
        b.append(L["meta_state"].format(en=en))
    else:
        b.append(L["meta_state2"].format(en=en, ebd=ebd))
    b.append(L["meta_cat"].format(cat=cat))
    b.append(L["meta_desc"].format(desc=desc))
    b.append(L["meta_mods"].format(mods=modline))
    b.append("")
    b.append("---")
    b.append("")
    b.append(L["h1"])
    b.append("")
    b.append(d["one"])
    b.append("")
    b.append(L["h2"])
    b.append("")
    b.append(d["detail"])
    b.append("")
    b.append(L["h3"])
    b.append("")
    b.append(d["diff"])
    b.append("")
    b.append(L["h4"])
    b.append("")
    b.append(d["lyra"])
    b.append("")
    b.append(L["h4_sub"])
    b.append("")
    b.append(lyra_txt)
    b.append("")
    b.append(L["h5"])
    b.append("")
    b.append(d["when"])
    b.append("")
    b.append(L["h6"])
    b.append("")
    b.append(d["note"])
    b.append("")
    b.append("---")
    b.append("")
    b.append(L["hsrc"])
    b.append("")
    if m.get("path"):
        b.append(L["ref_uplugin"].format(path=m["path"]))
    b.append(L["ref_uproj"].format(i=i))
    b.append(L["ref_lyra"])
    b.append("")

    fn = "%02d_%s.md" % (i, name)
    io.open(os.path.join(OUTDIR, fn), "w", encoding="utf-8").write("\n".join(b))
    INDEX.append((i, name, en, src, cat, d["layer"], d["one"].replace("|", "\\|"), fn))

idx = []
idx.append(L["idx_title"])
idx.append("")
idx.append(L["idx_desc1"])
idx.append(L["idx_desc2"])
idx.append("")
idx.append(L["idx_s1"])
idx.append("")
idx.append(L["idx_s1_head"])
idx.append("|---|---|---|")
idx.append(L["idx_true"].format(n=len([x for x in INDEX if x[2] == "true"])))
idx.append(L["idx_false"].format(n=len([x for x in INDEX if x[2] == "false"])))
idx.append("")
idx.append(L["idx_s2"])
idx.append("")
idx.append(L["idx_s2_head"])
idx.append("|---|---|")
idx.append(L["idx_engine"].format(n=len([x for x in INDEX if x[3] == "Engine"])))
idx.append(L["idx_project"].format(n=len([x for x in INDEX if x[3] == "Project"])))
idx.append(L["idx_missing"].format(n=len([x for x in INDEX if x[3] == "MISSING"])))
idx.append("")
idx.append(L["idx_s3"])
idx.append("")
idx.append(L["idx_s3_head"])
idx.append("|---|---|---|---|---|---|---|")
for r in INDEX:
    idx.append(L["idx_row"].format(i=r[0], name=r[1], fn=r[7], en=r[2], src=r[3], cat=r[4], layer=r[5], one=r[6]))
idx.append("")
idx.append(L["idx_s4"])
idx.append("")
byl = {}
for r in INDEX:
    byl.setdefault(r[5], []).append(r)
for layer in sorted(byl, key=lambda x: -len(byl[x])):
    idx.append(L["idx_layer"].format(layer=layer, n=len(byl[layer])))
    idx.append("")
    for r in byl[layer]:
        idx.append(L["idx_layer_item"].format(i=r[0], name=r[1], fn=r[7]))
    idx.append("")
idx.append("---")
idx.append("")
idx.append(L["idx_s5"])
idx.append("")
idx.append(L["idx_s5_head"])
idx.append("|---|---|---|")
S5 = [
    (21, "MagicLeap", "MagicLeap AR 平台，UE5.4 起从引擎移除"),
    (22, "MagicLeapMedia", "同上（媒体播放）"),
    (23, "MagicLeapPassableWorld", "同上（可通行世界）"),
    (26, "OpenXRHMD", "UE5.4 已并入 `OpenXR` 主插件"),
    (27, "SteamVR", "5.4 改为可选分发（Launcher 版不带）"),
    (28, "GearVR", "三星 GearVR 已废弃"),
    (29, "LuminPlatformFeatures", "MagicLeap Lumin 平台已废弃"),
    (30, "MLSDK", "MagicLeap SDK，已移除"),
    (62, "RuntimePhysXCooking", "UE5 默认 Chaos，PhysX 已移除"),
]
for a, b2, c2 in S5:
    idx.append("| %d | `%s` | %s |" % (a, b2, c2))
idx.append("")
idx.append(L["idx_s6"])
idx.append("")
idx.append(L["idx_s6_head"])
idx.append("|---|---|---|")
idx.append("| 6 | `D3DExternalGPUStatistics` | `\"Optional\": true` → UBT 记 LogDebug 后跳过 |")
idx.append("| 32 | `PlayFabParty` | `PlatformAllowList: [XB1, XSX]` → Win64 下引擎判定不支持，跳过 |")
idx.append("")
idx.append("---")
idx.append("")
idx.append(L["idx_src_head"])
idx.append("")
for line in [
    "`E:\\ue5\\game_design\\code\\Tenkaichi\\Tenkaichi.uproject`（70 项插件，实查）",
    "`D:\\ue5\\Epic Games\\UE_5.4\\Engine\\Plugins`（递归 620 个 `.uplugin`，逐个读取 `Category` / `Description` / `Modules` / `EnabledByDefault`）",
    "`E:\\ue5\\game_design\\code\\Tenkaichi\\Plugins`（9 个工程插件 `.uplugin`）",
    "`E:\\ue5\\LyraStarterGame`（全目录扫描 `.uproject/.uplugin/.Build.cs/.cpp/.h/.ini`，统计每个插件名的引用位置）",
]:
    idx.append(L["idx_src"].format(line=line))
idx.append("")

io.open(os.path.join(OUTDIR, "00_\u63d2\u4ef6\u8be6\u89e3_\u7d22\u5f15.md"), "w", encoding="utf-8").write("\n".join(idx))
print("generated:", len(INDEX), "files + index")
