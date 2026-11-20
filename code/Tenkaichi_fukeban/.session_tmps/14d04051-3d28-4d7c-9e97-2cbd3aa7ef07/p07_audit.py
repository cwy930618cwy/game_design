# -*- coding: utf-8 -*-
"""P07 Teams 符号级对账：Lyra Teams vs Tenkaichi Teams"""
import os, re, io, sys

LYRA = r"E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Teams"
TENK = r"E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\Teams"

# 文件名映射（只换前缀）
def lyra2tenk(fn):
    if fn.startswith("Lyra"):
        return "Tenkaichi" + fn[4:]
    return fn

def read(p):
    for enc in ("utf-8-sig", "utf-8", "gbk", "latin-1"):
        try:
            with io.open(p, "r", encoding=enc) as f:
                return f.read()
        except Exception:
            continue
    return ""

lyra_files = sorted(os.listdir(LYRA))
tenk_files = sorted(os.listdir(TENK))

print("=" * 70)
print("一、文件齐平（%d vs %d）" % (len(lyra_files), len(tenk_files)))
print("=" * 70)
missing, extra = [], []
for f in lyra_files:
    t = lyra2tenk(f)
    if t not in tenk_files:
        missing.append((f, t))
for t in tenk_files:
    exp = set(lyra2tenk(f) for f in lyra_files)
    if t not in exp:
        extra.append(t)
print("Lyra 有 %d，Tenkaichi 有 %d" % (len(lyra_files), len(tenk_files)))
print("缺失（Lyra 有 / T 没有）: %s" % (missing or "无"))
print("多余（T 有 / Lyra 无对应）: %s" % (extra or "无"))

print()
print("=" * 70)
print("二、残留 Lyra 前缀 / U 前缀类名错误")
print("=" * 70)
bad = 0
for t in tenk_files:
    p = os.path.join(TENK, t)
    txt = read(p)
    for i, line in enumerate(txt.splitlines(), 1):
        s = line.strip()
        if s.startswith("//") or s.startswith("*") or s.startswith("/*"):
            continue
        hits = re.findall(r"\b(?:[UALFSIE][A-Za-z0-9_]*)?Lyra[A-Za-z0-9_]*\b", line)
        if hits:
            print("  %s:%d  %s" % (t, i, s))
            bad += 1
if not bad:
    print("  无")

print()
print("=" * 70)
print("三、注释英文残留（代码块里 // 或 /* 后接英文）")
print("=" * 70)
eng = 0
for t in sorted(tenk_files):
    p = os.path.join(TENK, t)
    txt = read(p)
    for i, line in enumerate(txt.splitlines(), 1):
        m = re.search(r"(//|/\*)\s*(.*)$", line)
        if not m:
            continue
        rest = m.group(2).strip()
        if not rest:
            continue
        # 允许：@TODO、代码标识符、纯符号、UE_/TEXT 等
        if re.search(r"[\u4e00-\u9fff]", rest):
            continue
        if rest.startswith("@TODO"):
            continue
        if re.match(r"^[\s\-=~#*]*$", rest):
            continue
        print("  %s:%d  %s" % (t, i, line.strip()))
        eng += 1
if not eng:
    print("  无")
