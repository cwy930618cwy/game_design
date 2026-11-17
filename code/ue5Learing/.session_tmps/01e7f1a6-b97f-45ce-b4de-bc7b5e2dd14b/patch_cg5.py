# -*- coding: utf-8 -*-
import io

p = r'E:\ue5\game_design\code\ue5Learing\.session_tmps\01e7f1a6-b97f-45ce-b4de-bc7b5e2dd14b\gen_cg2.py'
s = io.open(p, encoding='utf-8').read()

fixes = [
    # 217-219 引擎 2 行块（217 起、219 收尾）
    ("T[217] = '/** 表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。'\nT[218] = 'UI 泵消息循环自然返回前，游戏线程做不了任何事。 */'\nT[219] = ''",
     "T[217] = '/** 表示游戏线程当前正卡在深层调用栈里，同时编辑器 UI 的一部分在泵消息。'\nT[218] = 'UI 泵消息循环自然返回前，游戏线程做不了任何事。'\nT[219] = '*/'"),
    # 702-706 引擎 4 行块
    ("T[702] = '\\t/**'\nT[703] = '\\t * 恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数'\nT[704] = '\\t * 在检查线程状态（如 IsInGameThread()）时能正常工作。'\nT[705] = '\\t */'",
     "T[702] = '\\t/**'\nT[703] = '\\t * 恢复 EStaticInit 标签，让全局（或局部静态）C++ 对象的析构函数'\nT[704] = '\\t * 在检查线程状态（如 IsInGameThread()）时能正常工作。'\nT[705] = '\\t * （原文：state (from functions like IsInGameThread()).）'\nT[706] = '\\t */'"),
    # 737-743 引擎 6 行块
    ("T[737] = '/**'\nT[738] = ' * 确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。'\nT[739] = ' *'\nT[740] = ' * @param CtorSignature 当前正在跑的构造函数签名，'\nT[741] = ' *\\t\\t用来拼出正确的错误信息。'\nT[742] = ' */'",
     "T[737] = '/**'\nT[738] = ' * 确保当前线程正处于「从 UClass 取 vtable 指针」的过程中。'\nT[739] = ' *'\nT[740] = ' * @param CtorSignature 当前正在跑的构造函数签名，'\nT[741] = ' *\\t\\t用来拼出正确的错误信息。'\nT[742] = ' * （原文：construct proper error message.）'\nT[743] = ' */'"),
    # 849-852 引擎 3 行块
    ("T[849] = '/* 用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。'\nT[850] = ' * 目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。'\nT[851] = ' */'",
     "T[849] = '/* 用来包住对 this 的空指针检查；在不用 -fno-delete-null-pointer-checks 时会被优化掉。'\nT[850] = ' * 目的是给调用方机会去修掉 this 为空的潜在原因，不再依赖未定义行为。'\nT[851] = ' * （原文：on undefined behavior.）'\nT[852] = ' */'"),
]

for old, new in fixes:
    assert old in s, old[:40]
    s = s.replace(old, new)

io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patched')