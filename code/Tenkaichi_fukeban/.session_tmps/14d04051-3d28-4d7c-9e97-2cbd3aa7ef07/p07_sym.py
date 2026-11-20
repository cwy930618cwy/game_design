# -*- coding: utf-8 -*-
"""P07 符号级对账：逐类比对 Lyra vs Tenkaichi 的类名/函数/属性"""
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
    """去掉注释、字符串、预处理，再把 Lyra 前缀归一"""
    txt = re.sub(r"/\*.*?\*/", " ", txt, flags=re.S)
    txt = re.sub(r"//[^\n]*", "", txt)
    txt = re.sub(r'"[^"\n]*"', '""', txt)
    txt = re.sub(r"^\s*#.*$", "", txt, flags=re.M)
    return txt

def symbols(txt):
    """提取：类名、成员函数签名、UPROPERTY/UFUNCTION 名"""
    t = norm(txt)
    out = set()
    # 类/结构体声明
    for m in re.finditer(r"\b(?:class|struct|enum(?:\s+class)?)\s+([AUFSEI][A-Za-z0-9_]*)", t):
        out.add("TYPE:" + m.group(1))
    # 函数定义 (CppName::Func 或 函数声明)
    for m in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_:]*)\s*\(", t):
        n = m.group(1)
        if n in ("if", "for", "while", "switch", "return", "sizeof", "check", "ensure", "static_assert"):
            continue
        out.add("FN:" + n)
    # UPROPERTY / UFUNCTION 后的成员名（取声明里的标识符）
    for m in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:=[^;,)]*)?[;,)]", t):
        out.add("VAR:" + m.group(1))
    return out

lyra_files = sorted(os.listdir(LYRA))
total_l = total_t = 0
problems = []

for f in lyra_files:
    tf = lyra2tenk(f)
    lp, tp = os.path.join(LYRA, f), os.path.join(TENK, tf)
    lt, tt = read(lp), read(tp)
    if not tt:
        problems.append("[缺失] " + tf); continue

    # 行数
    ln, tn = len(lt.splitlines()), len(tt.splitlines())
    total_l += ln; total_t += tn

    # 把 Tenkaichi 文本反换成 Lyra，再比符号
    tt_norm = tt.replace("Tenkaichi", "Lyra").replace("TENKAICHI", "LYRA").replace("TenkaichiGame", "LyraGame")
    sl, st = symbols(lt), symbols(tt_norm)
    # Tenkaichi 侧因 #if 0 挂起可能少符号
    only_l = sorted(x for x in sl - st if not x.startswith("VAR:"))
    only_l_v = sorted(x for x in sl - st if x.startswith("VAR:"))
    if only_l:
        problems.append("[%s] Lyra 有 T 无: %s" % (tf, only_l))
    if only_l_v:
        problems.append("[%s]   变量级差异: %s" % (tf, only_l_v))

print("=" * 70)
print("P07 符号级对账")
print("=" * 70)
print("Lyra 总行数 %d / Tenkaichi 总行数 %d" % (total_l, total_t))
print()
if problems:
    for p in problems:
        print(p)
else:
    print("✅ 符号零缺口")
