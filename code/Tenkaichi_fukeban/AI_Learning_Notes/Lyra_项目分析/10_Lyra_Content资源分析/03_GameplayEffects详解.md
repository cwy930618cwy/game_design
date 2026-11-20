# 03 — GameplayEffects 资源详解（数值效果）

> **定位**：单独讲 Lyra Content 里的 `GameplayEffects/` 目录——存放 **GameplayEffect（GE）数据资产**，负责"改变数值"的效果（伤害/治疗/Buff/Debuff）。
>
> **关联**：
> - [UE5.6_源码分析/.../01_GAS技能系统](../../UE5.6_源码分析/02_Runtime插件详解/01_GameplayAbilities_GAS技能系统.md) — GAS 四大概念地基
> - [02_GameplayCueNotifies详解](./02_GameplayCueNotifies详解.md) — GE 如何通过 Tag 触发 GC 表现
>
> **一句话**：`GameplayEffects/` 存的是"改数值"的效果资产。它**影响游戏性**（会改变战局），走服务端权威、精确同步。通过 Modifier 改属性，通过 Tag 触发对应的 GameplayCue 表现。

---

## 一、GameplayEffect 是什么

**GameplayEffect（GE）= 一次对数值的修改操作**。

| 对比 | GameplayEffect（GE） | GameplayCue（GC） |
|------|---------------------|------------------|
| 管什么 | **数值变化**（伤害/治疗/Buff/Debuff） | **视听表现**（特效/音效/震屏） |
| 影响游戏性 | ✅ 会改变战局 | ❌ 纯观感 |
| 网络处理 | 服务端权威，精确同步 | 可本地/可丢弃 |
| Content 目录 | `GameplayEffects/` | `GameplayCueNotifies/` |
| 例子 | "扣 30 血""中毒 5 秒""攻击 +10" | "爆炸特效""命中音效" |

> ⚠️ **铁律**：GE 影响游戏性，所以必须走**服务端权威**。客户端不能自己算伤害，必须由服务端施加 GE 并同步结果。

### GE 和 GA 的关系

```
GA（GameplayAbility）= 技能"动作"（玩家按下按键释放的那个行为）
GE（GameplayEffect） = 技能"效果"（动作造成的数值变化）

一个 GA 可以施加多个 GE：
  GA_火球
    ├─ GE_伤害（目标 -30 血）
    └─ GE_灼烧（每秒 -5 血，持续 3 秒）
```

---

## 二、GameplayEffects/ 目录

### 是什么
存放各种 **GameplayEffect 数据资产**（`.uasset`）。双击打开是**填表界面**，配置"改什么数值、持续多久、怎么叠加"。

### 典型内容
```
GameplayEffects/
├── GE_Damage_Health          ← 扣血效果
├── GE_Heal                   ← 治疗效果
├── GE_Buff_AttackPower       ← 加攻击力 Buff
├── GE_Debuff_Poison          ← 中毒 Debuff
├── GE_Init_Health            ← 初始化生命值
└── ...（各种即时/持续效果）
```

### GE 的关键配置项（双击打开能看到）

| 配置 | 含义 | 例子 |
|------|------|------|
| **Duration Policy** | 持续策略 | Instant（即时）/ Duration（持续 N 秒）/ Infinite（永久） |
| **Modifiers** | 改哪些属性 | Attribute = Health, Modifier = Add -30（扣 30 血） |
| **Stacking** | 叠加规则 | 中毒可叠 5 层，每层每秒掉血 |
| **Tags** | 标签 | 标记效果类型（用于驱散/免疫判断） |
| **Granted Tags** | 授予的 Tag | 施加后给目标打上状态标记（如"中毒中"） |

### 三种持续策略

| 策略 | 含义 | 典型用途 |
|------|------|---------|
| **Instant（即时）** | 立刻生效，一次性 | 开枪伤害、治疗药水 |
| **Duration（持续）** | 持续一段时间后消失 | 中毒 5 秒、Buff 持续 10 秒 |
| **Infinite（无限）** | 一直存在直到手动移除 | 装备提供的属性加成 |

---

## 三、怎么被引用 / 施加

GE 有两种使用方式：

### 方式 1：随 AbilitySet 授予（常驻类效果）
由 AbilitySet 在角色初始化时批量授予（见 [01_数据资产类详解](./01_数据资产类详解.md)）：

```cpp
// AbilitySet 初始化时授予（如初始生命值上限 GE）
AbilitySet->GiveToAbilitySystem(ASC, Level, Context);
```

### 方式 2：GA 运行时动态施加（一次性效果）
由 GA 在技能释放时实时施加：

```cpp
// GA 运行时施加伤害 GE
UAbilitySystemBlueprintLibrary::ApplyGameplayEffectToTarget(
    DamageEffectClass, SourceASC, TargetASC, Level, Context);
```

> ⚠️ **关键区别**：
> - **GA 是"授予后常驻"**（学会了就一直会）
> - **GE 是"运行时动态创建实例"**（每次开枪 new 一个 GE 实例施加伤害，用完即弃）

---

## 四、Modifier 怎么改数值（核心）

GE 真正干活的地方是 **Modifiers 数组**。每个 Modifier 描述"改哪个属性、怎么改"：

```
Modifier[0]:
  Attribute: Health          ← 改哪个属性
  Modifier Op: Add           ← 操作类型（Add/Multiply/Override）
  Value: -30                 ← 改多少
```

### Modifier Op 三种操作

| 操作 | 含义 | 例子 |
|------|------|------|
| **Add** | 加减 | 血量 -30 |
| **Multiply** | 乘除（百分比） | 攻击力 ×1.5（+50%） |
| **Override** | 直接覆盖 | 强制速度 = 0（定身） |

> Modifier 支持从曲线/其他属性取值，实现"伤害 = 攻击力 × 系数"这类公式。

---

## 五、GE 如何触发表现（连接 GC）

GE 本身只管数值，但它可以通过 **Tag** 触发 GameplayCue 播放表现：

```
GE_FireballDamage
  ├─ Modifier: Health -30（改数值）
  └─ Trigger Tags: GameplayCue.Fire.Explosion
                          │
                          ▼
              系统匹配 GC_Explosion → 播放爆炸特效
```

> 详见 [02_GameplayCueNotifies详解](./02_GameplayCueNotifies详解.md)。**这就是"逻辑（GE）和表现（GC）分离"的连接点。**

---

## 六、策划在哪配

| 想改什么 | 去哪 |
|---------|------|
| 改伤害数值 | `GameplayEffects/` 里对应 GE，改 Modifier 的 Value |
| 改 Buff 持续时间 | 改 Duration Policy + Duration Magnitude |
| 改叠加规则 | 改 Stacking 配置（层数/刷新策略） |
| 新增一个效果 | 复制 GE 模板改名，配 Modifier |
| 让效果带新表现 | 给 GE 加 Trigger Tag 即可，不用改代码 |

> ⚠️ GE 是纯数据资产，改数值**不用编译 C++**。但因为是服务端权威，多人游戏里要确保改动在服务端生效。

---

## 七、常见误区

| 误区 | 正确理解 |
|------|---------|
| "GE 放场景里" | GE 是数据资产，不在场景里；运行时动态创建实例 |
| "GE 是常驻的" | 除了随 AbilitySet 授予的，多数是运行时施加、用完即弃 |
| "客户端也能施加 GE" | ❌ 伤害类 GE 必须服务端权威 |
| "GE 里能播特效" | GE 只管数值；表现靠 Tag 触发 GC |
| "一个 GA 只能一个 GE" | 一个 GA 可以施加多个 GE |

---

## 八、总结速查

```
GameplayEffects/ = 数值效果资产库（改血量/攻击/Buff/Debuff）

核心配置：
  Duration Policy = 持续策略（Instant/Duration/Infinite）
  Modifiers       = 改哪个属性、怎么改（Add/Multiply/Override）
  Stacking        = 叠加规则
  Tags            = 标记 + 触发 GC 表现

使用方式：
  ├─ 随 AbilitySet 授予（常驻，如初始属性）
  └─ GA 运行时施加（一次性，如开枪伤害，用完即弃）

关键特性：
  ├─ 影响游戏性，走服务端权威
  ├─ 通过 Modifier 改数值
  └─ 通过 Tag 触发 GameplayCue 表现

分工对比：
  GE（GameplayEffects/）      = 打多少血（游戏性）
  GC（GameplayCueNotifies/）  = 看起来怎样（观感）
```

**一句话**：`GameplayEffects/` 管"打多少血、加多少属性"（游戏性），是 GAS 里真正改变战局的部分。它通过 **Modifier** 改数值、通过 **Tag** 触发表现，并且**必须走服务端权威**——这是它和纯表现的 GameplayCue 最根本的区别。

---

## 九、下一步

- [UE5.6_源码分析/.../01_GAS技能系统](../../UE5.6_源码分析/02_Runtime插件详解/01_GameplayAbilities_GAS技能系统.md) — GAS 四大概念地基
- [02_GameplayCueNotifies详解](./02_GameplayCueNotifies详解.md) — GE 触发的表现
- [01_数据资产类详解](./01_数据资产类详解.md) — AbilitySet 如何授予 GE
- [04_角色与动画资源](./04_角色与动画资源.md) — 角色技能动画
