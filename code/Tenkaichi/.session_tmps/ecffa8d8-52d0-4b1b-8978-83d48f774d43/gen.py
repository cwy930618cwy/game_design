import io, json, os

TMP = r"E:\ue5\game_design\code\Tenkaichi\.session_tmps\ecffa8d8-52d0-4b1b-8978-83d48f774d43"
OUTDIR = r"E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra澶嶅埢\S00宸ョ▼楠ㄦ灦\绛旂枒鏂囦欢\鎻掍欢璇﹁В"
META = os.path.join(TMP, "plugins.json")
DATA = os.path.join(TMP, "data.json")

meta = {r["idx"]: r for r in json.load(io.open(META, encoding="utf-8-sig"))}
data = json.load(io.open(DATA, encoding="utf-8-sig"))

os.makedirs(OUTDIR, exist_ok=True)

INDEX = []

for k in sorted(data, key=lambda x: int(x)):
    i = int(k)
    d = data[k]
    m = meta[i]

    name = m["name"]
    en = "true" if m["en"] is True else ("false" if m["en"] is False else str(m["en"]))
    src = m["src"]
    cat = m["cat"] or "锛堟棤锛?
    ebd = m["ebd"]
    mods = m["mods"]
    desc = m["desc"] or "锛堝畼鏂规棤鎻忚堪锛?

    if src == "MISSING":
        srcline = "**鏈満娌℃湁姝ゆ彃浠?*锛堝紩鎿庝笌宸ョ▼ `Plugins\\` 鍧囨棤锛?
    elif src == "Engine":
        srcline = "寮曟搸鎻掍欢 `D:\\ue5\\Epic Games\\UE_5.4\\Engine\\Plugins\\...`"
    else:
        srcline = "宸ョ▼鎻掍欢 `E:\\ue5\\game_design\\code\\Tenkaichi\\Plugins\\%s\\`" % name

    modline = "銆?.join("`%s`锛?s锛? % (x["name"], x["type"]) for x in mods) if mods else "锛堟棤妯″潡 / 绌猴級"

    lyra_lines = []
    for rel, cnt in m["lyra"]:
        lyra_lines.append("- `%s`锛坸%d锛? % (rel, cnt))
    lyra_txt = "\n".join(lyra_lines) if lyra_lines else "- 锛堟棤锛?

    md = []
    md.append("# %02d 鈥?`%s` 璇﹁В" % (i, name))
    md.append("")
    md.append("> 浣嶇疆锛歚.uproject` 鐨?`Plugins` 鏁扮粍绗?**%d** 椤? % i)
    md.append("> 鏉ユ簮锛?s" % srcline)
    md.append("> 褰撳墠鐘舵€侊細`\"Enabled\": %s`%s" % (en, "" if ebd in ("", None) else "锛宍EnabledByDefault` = `%s`" % ebd))
    md.append("> 鍒嗙被锛歚%s`銆€瀹樻柟鎻忚堪锛?s" % (cat, desc))
    md.append("> 妯″潡锛?s" % modline)
    md.append("")
    md.append("---")
    md.append("")
    md.append("## 涓€銆佷竴鍙ヨ瘽")
    md.append("")
    md.append(d["one"])
    md.append("")
    md.append("## 浜屻€佸畠鍒板簳鍋氫粈涔?)
    md.append("")
    md.append(d["detail"])
    md.append("")
    md.append("## 涓夈€佹湁瀹?vs 娌″畠")
    md.append("")
    md.append(d["diff"])
    md.append("")
    md.append("## 鍥涖€丩yra 鎬庝箞鐢ㄥ畠")
    md.append("")
    md.append(d["lyra"])
    md.append("")
    md.append("**Lyra 婧愮爜閲岀殑寮曠敤浣嶇疆**锛?)
    md.append("")
    md.append(lyra_txt)
    md.append("")
    md.append("## 浜斻€佹湰澶嶅埢绾夸粈涔堟椂鍊欑瀹?)
    md.append("")
    md.append(d["when"])
    md.append("")
    md.append("## 鍏€佹敞鎰?)
    md.append("")
    md.append(d["note"])
    md.append("")
    md.append("---")
    md.append("")
    md.append("## 婧愮爜渚濇嵁")
    md.append("")
    if m.get("path"):
        md.append("- `%s`锛坄.uplugin` 鍘熸枃锛? % m["path"])
    md.append("- `E:\\ue5\\game_design\\code\\Tenkaichi\\Tenkaichi.uproject` 绗?%d 椤? % i)
    md.append("- `E:\\ue5\\LyraStarterGame\\LyraStarterGame.uproject`锛圠yra 鍘熺増 76 椤瑰鐓э級")
    md.append("")

    fn = "%02d_%s.md" % (i, name)
    io.open(os.path.join(OUTDIR, fn), "w", encoding="utf-8").write("\n".join(md))

    INDEX.append((i, name, en, src, cat, d["layer"], d["one"], fn))

# 绱㈠紩
idx = []
idx.append("# 鎻掍欢璇﹁В 鈥?绱㈠紩锛?0 椤癸級")
idx.append("")
idx.append("> 瀵瑰簲 `Tenkaichi.uproject` 鐨?`Plugins` 鏁扮粍锛岄€愰」涓€浠借瑙ｃ€?)
idx.append("> 渚濇嵁锛歚D:\\ue5\\Epic Games\\UE_5.4\\Engine\\Plugins`锛堝疄鏌?620 涓?`.uplugin`锛? 鍚勬彃浠?`.uplugin` 鍘熸枃 + `E:\\ue5\\LyraStarterGame`锛堝紩鐢ㄤ綅缃疄鏌ワ級")
idx.append("")
idx.append("## 涓€銆佹寜鍚敤鐘舵€佸垎缁?)
idx.append("")
idx.append("| 鐘舵€?| 鏁伴噺 | 璇存槑 |")
idx.append("|---|---|---|")
en_true = [x for x in INDEX if x[2] == "true"]
en_false = [x for x in INDEX if x[2] == "false"]
idx.append("| `Enabled: true` | **%d** | 鍙備笌缂栬瘧 / 鍔犺浇 |" % len(en_true))
idx.append("| `Enabled: false` | **%d** | 寮曟搸鍘嬫牴涓嶆煡锛屾湰鏈虹己澶变篃涓嶆姤閿?|" % len(en_false))
idx.append("")
idx.append("## 浜屻€佹寜鏉ユ簮鍒嗙粍")
idx.append("")
idx.append("| 鏉ユ簮 | 鏁伴噺 |")
idx.append("|---|---|")
idx.append("| 寮曟搸鎻掍欢锛堟湰鏈哄瓨鍦級 | **%d** |" % len([x for x in INDEX if x[3] == "Engine"]))
idx.append("| 宸ョ▼鎻掍欢锛坄Tenkaichi\\Plugins\\`锛?| **%d** |" % len([x for x in INDEX if x[3] == "Project"]))
idx.append("| 鏈満缂哄け锛堥潬璞佸厤锛?| **%d** |" % len([x for x in INDEX if x[3] == "MISSING"]))
idx.append("")
idx.append("## 涓夈€佸叏琛紙70 椤癸級")
idx.append("")
idx.append("| # | 鎻掍欢 | En | 鏉ユ簮 | 鍒嗙被 | 灞傜骇 | 涓€鍙ヨ瘽 |")
idx.append("|---|---|---|---|---|---|---|")
for i, name, en, src, cat, layer, one, fn in INDEX:
    one_short = one.replace("|", "\\|")
    idx.append("| %d | [`%s`](%s) | %s | %s | %s | %s | %s |" % (
        i, name, fn.replace(" ", "%20"), en, src, cat, layer, one_short))
idx.append("")
idx.append("## 鍥涖€佹寜灞傜骇鍒嗙粍")
idx.append("")
by_layer = {}
for i, name, en, src, cat, layer, one, fn in INDEX:
    by_layer.setdefault(layer, []).append((i, name, fn))
for layer in sorted(by_layer, key=lambda x: -len(by_layer[x])):
    idx.append("### %s锛?d 椤癸級" % (layer, len(by_layer[layer])))
    idx.append("")
    for i, name, fn in by_layer[layer]:
        idx.append("- [%02d %s](%s)" % (i, name, fn.replace(" ", "%20")))
    idx.append("")
idx.append("---")
idx.append("")
idx.append("## 浜斻€? 涓€孍nabled:false 涓旀湰鏈虹己澶便€嶇殑鎻掍欢锛堝埆鏀规垚 true锛?)
idx.append("")
idx.append("| # | 鎻掍欢 | 涓轰粈涔堟湰鏈烘病鏈?|")
idx.append("|---|---|---|")
idx.append("| 21 | `MagicLeap` | MagicLeap AR 骞冲彴锛孶E5.4 璧蜂粠寮曟搸绉婚櫎 |")
idx.append("| 22 | `MagicLeapMedia` | 鍚屼笂锛堝獟浣撴挱鏀撅級 |")
idx.append("| 23 | `MagicLeapPassableWorld` | 鍚屼笂锛堝彲閫氳涓栫晫锛?|")
idx.append("| 26 | `OpenXRHMD` | UE5.4 宸插苟鍏?`OpenXR` 涓绘彃浠?|")
idx.append("| 27 | `SteamVR` | 5.4 鏀逛负鍙€夊垎鍙戯紙Launcher 鐗堜笉甯︼級 |")
idx.append("| 28 | `GearVR` | 涓夋槦 GearVR 宸插簾寮?|")
idx.append("| 29 | `LuminPlatformFeatures` | MagicLeap Lumin 骞冲彴宸插簾寮?|")
idx.append("| 30 | `MLSDK` | MagicLeap SDK锛屽凡绉婚櫎 |")
idx.append("| 62 | `RuntimePhysXCooking` | UE5 榛樿 Chaos锛孭hysX 宸茬Щ闄?|")
idx.append("")
idx.append("## 鍏€? 涓€孍nabled:true 浣嗘湰鏈虹己澶便€嶇殑璞佸厤椤?)
idx.append("")
idx.append("| # | 鎻掍欢 | 璞佸厤鏈哄埗 |")
idx.append("|---|---|---|")
idx.append("| 6 | `D3DExternalGPUStatistics` | `\"Optional\": true` 鈫?UBT 璁?LogDebug 鍚庤烦杩?|")
idx.append("| 32 | `PlayFabParty` | `PlatformAllowList: [XB1, XSX]` 鈫?Win64 涓嬪紩鎿庡垽瀹氫笉鏀寔锛岃烦杩?|")
idx.append("")
idx.append("---")
idx.append("")
idx.append("## 婧愮爜渚濇嵁")
idx.append("")
idx.append("- `E:\\ue5\\game_design\\code\\Tenkaichi\\Tenkaichi.uproject`锛?0 椤规彃浠讹紝瀹炴煡锛?)
idx.append("- `D:\\ue5\\Epic Games\\UE_5.4\\Engine\\Plugins`锛堥€掑綊 620 涓?`.uplugin`锛岄€愪釜璇诲彇 `Category` / `Description` / `Modules` / `EnabledByDefault`锛?)
idx.append("- `E:\\ue5\\game_design\\code\\Tenkaichi\\Plugins`锛? 涓伐绋嬫彃浠?`.uplugin`锛?)
idx.append("- `E:\\ue5\\LyraStarterGame`锛堝叏鐩綍鎵弿 `.uproject/.uplugin/.Build.cs/.cpp/.h/.ini`锛岀粺璁℃瘡涓彃浠跺悕鐨勫紩鐢ㄤ綅缃級")
idx.append("")

io.open(os.path.join(OUTDIR, "00_鎻掍欢璇﹁В_绱㈠紩.md"), "w", encoding="utf-8").write("\n".join(idx))
print("generated:", len(INDEX), "files + index")
