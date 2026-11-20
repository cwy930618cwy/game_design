# -*- coding: utf-8 -*-
"""P07 反向对账：T 有而 Lyra 无的符号（铁律16 加法检查）+ 铁律32 方法注释检查"""
import os, re, io

LYRA = r"E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Teams"
TENK = r"E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\Teams"

def lyra2tenk(fn):
    return "Tenkaichi" + fn[4:] if fn.startswith("Lyra") else fn

def read(p):
    for enc in ("utf-8-sig", "utf-8", "gbk", "latin-1"):
        try:
            with io.open(p, "r", encoding=enc) as f:
                return f.read()
        except Exception:
            continue
    return ""

def norm(txt):
    txt = re.sub(r"/\*.*?\*/", " ", txt, flags=re.S)
    txt = re.sub(r"//[^\n]*", "", txt)
    txt = re.sub(r'"[^"\n]*"', '""', txt)
    txt = re.sub(r"^\s*#.*$", "", txt, flags=re.M)
    return txt

def symbols(txt):
    t = norm(txt)
    out = set()
    for m in re.finditer(r"\b(?:class|struct|enum(?:\s+class)?)\s+([AUFSEI][A-Za-z0-9_]*)", t):
        out.add("TYPE:" + m.group(1))
    for m in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_:]*)\s*\(", t):
        n = m.group(1)
        if n in ("if","for","while","switch","return","sizeof","check","ensure","static_assert"):
            continue
        out.add("FN:" + n)
    return out

print("=" * 70)
print("一、反向：T 有 / Lyra 无（铁律 16 加法检查）")
print("=" * 70)
found = False
for f in sorted(os.listdir(LYRA)):
    tf = lyra2tenk(f)
    lt, tt = read(os.path.join(LYRA, f)), read(os.path.join(TENK, tf))
    tt_norm = tt.replace("Tenkaichi", "Lyra").replace("TENKAICHI", "LYRA")
    sl, st = symbols(lt), symbols(tt_norm)
    only_t = sorted(st - sl)
    if only_t:
        print("  [%s] %s" % (tf, only_t)); found = True
if not found:
    print("  ✅ 无擅自添加")

print()
print("=" * 70)
print("二、铁律 32：方法上方是否有中文注释")
print("=" * 70)
CN = re.compile(r"[\u4e00-\u9fff]")
for t in sorted(os.listdir(TENK)):
    txt = read(os.path.join(TENK, t))
    lines = txt.splitlines()
    miss = []
    for i, line in enumerate(lines):
        s = line.strip()
        # 类外的函数定义：Type Class::Func(  或 文件级函数
        m = re.match(r"^(?:[A-Za-z_][A-Za-z0-9_:<>\*&\s]*\s+)?([A-Za-z_][A-Za-z0-9_]*::)([~A-Za-z_][A-Za-z0-9_]*)\s*\(", s)
        if not m:
            continue
        # 往上找 1~4 行注释
        ok = False
        for j in range(i-1, max(-1, i-5), -1):
            p = lines[j].strip()
            if p.startswith("//") or p.startswith("*") or p.startswith("/*"):
                if CN.search(p):
                    ok = True
                break
            if p == "":
                continue
            break
        if not ok:
            miss.append((i+1, s[:70]))
    if miss:
        print("  [%s]" % t)
        for ln, s in miss:
            print("     :%d  %s" % (ln, s))
