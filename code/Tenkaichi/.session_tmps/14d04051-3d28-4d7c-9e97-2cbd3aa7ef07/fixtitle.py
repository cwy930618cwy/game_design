# -*- coding: utf-8 -*-
import os

D = r'E:\ue5\game_design\code\Tenkaichi\AI_Learning_Notes\Lyra_从零底层代码搭建重构2\P06_Character'

# 文件名 -> 期望首行标题
WANT = {
    'P06.1_TenkaichiPawnData_h.md':               '# P06.1 UTenkaichiPawnData —— `.h` 声明层',
    'P06.1_TenkaichiPawnData_cpp.md':             '# P06.1 UTenkaichiPawnData —— `.cpp` 实现层',
    'P06.1_TenkaichiPawnData_总md.md':            '# P06.1 UTenkaichiPawnData —— 总 md（为什么 + 全景 + 依赖分析）',
    'P06.2_TenkaichiPawnExtensionComponent_h.md': '# P06.2 UTenkaichiPawnExtensionComponent —— `.h` 声明层',
    'P06.2_TenkaichiPawnExtensionComponent_cpp.md': '# P06.2 UTenkaichiPawnExtensionComponent —— `.cpp` 实现层',
    'P06.2_TenkaichiPawnExtensionComponent_总md.md': '# P06.2 UTenkaichiPawnExtensionComponent —— 总 md（Pawn 的"初始化总调度器"）',
    'P06.3_TenkaichiHeroComponent_h.md':          '# P06.3 UTenkaichiHeroComponent —— `.h` 声明层',
    'P06.3_TenkaichiHeroComponent_cpp.md':        '# P06.3 UTenkaichiHeroComponent —— `.cpp` 实现层',
    'P06.3_TenkaichiHeroComponent_总md.md':       '# P06.3 UTenkaichiHeroComponent —— 总 md（玩家操控的"英雄中枢"）',
    'P06.4-7_CharacterPawn_总md.md':              '# P06.4~P06.7 Character / Pawn —— 节总览',
    'P06.4_TenkaichiPawn_h.md':                   '# P06.4 ATenkaichiPawn —— `.h` 声明层',
    'P06.4_TenkaichiPawn_cpp.md':                 '# P06.4 ATenkaichiPawn —— `.cpp` 实现层',
    'P06.4_TenkaichiPawn_总md.md':                '# P06.4 ATenkaichiPawn —— 总 md（队伍感知的 Pawn 基类）',
    'P06.5_TenkaichiCharacter_h.md':              '# P06.5 ATenkaichiCharacter —— `.h` 声明层',
    'P06.5_TenkaichiCharacter_cpp.md':            '# P06.5 ATenkaichiCharacter —— `.cpp` 实现层',
    'P06.5_TenkaichiCharacter_总md.md':           '# P06.5 ATenkaichiCharacter —— 总 md（先看全景，再教 .h / .cpp）',
    'P06.6_TenkaichiCharacterWithAbilities_h.md': '# P06.6 ATenkaichiCharacterWithAbilities —— `.h` 声明层',
    'P06.6_TenkaichiCharacterWithAbilities_cpp.md': '# P06.6 ATenkaichiCharacterWithAbilities —— `.cpp` 实现层',
    'P06.6_TenkaichiCharacterWithAbilities_总md.md': '# P06.6 ATenkaichiCharacterWithAbilities —— 总 md（先看全景，再教 .h / .cpp）',
    'P06.7_TenkaichiCharacterMovementComponent_h.md': '# P06.7 UTenkaichiCharacterMovementComponent —— `.h` 声明层',
    'P06.7_TenkaichiCharacterMovementComponent_cpp.md': '# P06.7 UTenkaichiCharacterMovementComponent —— `.cpp` 实现层',
    'P06.7_TenkaichiCharacterMovementComponent_总md.md': '# P06.7 UTenkaichiCharacterMovementComponent —— 总 md（自定义角色移动组件）',
    'P06.8_TenkaichiHealthComponent_h.md':        '# P06.8 UTenkaichiHealthComponent —— `.h` 声明层',
    'P06.8_TenkaichiHealthComponent_cpp.md':      '# P06.8 UTenkaichiHealthComponent —— `.cpp` 实现层',
    'P06.8_TenkaichiHealthComponent_总md.md':     '# P06.8 UTenkaichiHealthComponent —— 总 md（为什么 + 全景 + 依赖分析）',
}

for name, want in WANT.items():
    p = os.path.join(D, name)
    if not os.path.exists(p):
        print('MISS', name)
        continue
    with open(p, encoding='utf-8') as f:
        lines = f.read().split('\n')
    # 去掉首个非空行之前的行（如归档说明不属于这里），定位第一个 # 开头的行
    idx = next((i for i, l in enumerate(lines) if l.startswith('# ')), None)
    if idx is None:
        print('NO-TITLE', name)
        continue
    old = lines[idx]
    if old == want:
        continue
    lines[idx] = want
    with open(p, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines))
    print('FIX', name)
    print('   old:', old)
    print('   new:', want)
