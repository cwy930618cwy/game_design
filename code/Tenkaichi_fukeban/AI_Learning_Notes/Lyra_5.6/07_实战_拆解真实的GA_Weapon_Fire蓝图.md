# 实战：拆解真实的 `GA_Weapon_Fire` 蓝图

> **定位**：[06 为什么技能是蓝图](./06_为什么技能是蓝图_事件图表在干嘛.md) 讲了"事件图表在干嘛"的**理论**。本文是它的**实战篇**——拿到一份**真实的开火技能蓝图**（从编辑器复制出来的节点文本），逐节点还原"开火这场戏"到底怎么演。
>
> **一句话**：开火 = **激活 → 本地玩家做射线检测 + 播开火动画 → 取出命中结果 → 命中则播特效(表现) + 服务器算伤害(逻辑) → 定时器处理连发**。这张图完美印证了 06 讲的每一条理论。

---

## 一、素材来源（怎么拿到的）

这份素材是把 `Content/Weapons/GA_Weapon_Fire.uasset` 的**事件图表全选 → 复制 → 粘成文本**得来的（方法见 [06 文末](./06_为什么技能是蓝图_事件图表在干嘛.md)）。文本里每个 `Begin Object` = 一个节点，`LinkedTo` = 连线。本文就是把这些机器文本"翻译"成人能看懂的流程图。

---

## 二、先确认它的"身份"（继承关系）

从文本里的 `EventReference` / `MemberParent` 能读出关键信息：

| 线索 | 含义 |
|------|------|
| `EventReference=(...MemberName="K2_ActivateAbility")` | 重写了**激活事件**（蓝图主入口）。 |
| `EventReference=(...MemberParent=".../LyraGameplayAbility'",MemberName="K2_OnAbilityAdded")` | 还重写了**技能授予事件**（来自 C++ 基类的蓝图钩子）。 |
| `StartRangedWeaponTargeting` 的 self 类型 = `LyraGameplayAbility_RangedWeapon` | 它其实继承自 **`ULyraGameplayAbility_RangedWeapon`**（远程武器技能基类）。 |

> **完整继承链**：
> ```
> ULyraGameplayAbility            ← C++ 骨架（05/06 讲过：消耗/互斥/失败/镜头）
>   └─ ULyraGameplayAbility_RangedWeapon   ← 远程武器专用基类（提供 StartRangedWeaponTargeting 等）
>        └─ GA_Weapon_Fire（蓝图）          ← 开火这场戏的具体演法（本文主角）
> ```

---

## 三、这张图里用到的"变量"（= 蓝图面板上能填的格）

文本里的 `VariableReference` 暴露了这个蓝图**面板上可配**的变量。它们正是"数据驱动"的证据——**换枪 = 换这几个值**：

| 变量 | 类型 | 作用 |
|------|------|------|
| `CharacterFireMontage` | `AnimMontage` | **开火动画**（角色全身动作）。 |
| `GE_Damage` | `GameplayEffect`（class） | **伤害效果**（命中后施加，扣血）。 |
| `GameplayCue_Impact` | `GameplayTag` | **命中特效标签**（命中时播的火花/音效）。 |
| `FireDelayTimeSecs` | `float` | **开火延迟**（连发间隔）。 |
| `AutoRate` | `float` | **连发速率**（自动武器每分钟发数）。 |

> 逻辑图一行不用改，只换这几个变量，就能把"步枪"变成"手枪/霰弹枪"。这就是 06 说的"数据驱动"。

---

## 四、逐节点还原：开火这场戏怎么演

把文本里的连线（`LinkedTo`）理成两条主线：**执行流**（什么时候做什么）和**数据流**（命中结果怎么传）。

### 4.1 执行流（主流程）

```
① [Event K2_ActivateAbility]  ← 技能被激活（玩家按开火键）
        │
        ▼
② [Sequence 顺序执行]  ← 同时分两路
   │
   ├─ then_0 ─▶ ③ [Is Locally Controlled? 是否本地玩家]
   │                │ True（本地玩家才做检测）
   │                ▼
   │            ④ [Start Ranged Weapon Targeting]  ← 射线检测/瞄准
   │               【注释原文】"If the Player is locally controlled, do weapon
   │                trace (if not, the target data will get replicated to us)"
   │                （本地玩家做弹道检测；其他玩家的检测结果会网络复制过来）
   │
   └─ then_1 ─▶ ⑦ [Play Montage and Wait 播放开火动画并等待]
                    输入：CharacterFireMontage（开火动画）
                    │
                    ├─ OnCompleted   ─┐
                    ├─ OnInterrupted ─┤  三种结局任一
                    └─ OnCancelled   ─┴─▶ ⑪ [End Ability 结束技能]
                    │
                    └─ then ─▶ ⑧ [Set Timer 定时器]
                                 延迟 FireDelayTimeSecs 秒后触发
                                    │
                                    ▼
                              ⑨ [Custom Event: FireComplete]  ← 连发/点射后处理
```

### 4.2 数据流（命中结果处理，注释框："Process target data once ready"）

```
⑤ [目标数据就绪事件]  ← ④ 的射线检测出结果后触发
        │
        ▼
⑥ [Get Hit Result From Target Data]  ← 从目标数据里取出命中结果
        │
        ▼
⑦ [Break Hit Result 拆解命中]  ← 拆出一堆字段：
        │   bBlockingHit(是否命中) / Location / Normal / PhysMat(材质)
        │   HitActor / HitComponent / HitBoneName(命中骨骼) ...
        │
        ├─ bBlockingHit
        ▼
⑧ [Branch 分支判断]
   ├─ True（命中了）─▶ ⑨ [Execute Gameplay Cue 播命中特效]
   │                     参数：GameplayCue_Impact(特效标签)
   │                           + MakeGameplayCueParametersFromHitResult(命中参数)
   │                    │
   │                    └─ then ─▶ ⑩ [Has Authority? 是否有服务器权限]
   │                                    │ True（只有服务器能算伤害，防外挂）
   │                                    ▼
   │                               ⑪ [Apply Gameplay Effect to Target 施加伤害]
   │                                  施加 GE_Damage 给目标 ★扣血发生在这里
   │
   └─ False（打空）─▶ （不处理）
```

---

## 五、这张图印证了 06 的 4 条理论

| 06 文档讲的理论 | 这张真图里的证据 |
|----------------|-----------------|
| **蓝图重写 `ActivateAbility` 是主体** | ① `K2_ActivateAbility` 就是入口。 |
| **用 `Ability Task` 节点做异步** | ⑦ 节点文本 `ProxyClass=...AbilityTask_PlayMontageAndWait`——就是"播动画并等待"这个 Ability Task。 |
| **命中 → 施加伤害 GE** | ⑥⑦⑧⑪：取命中 → 拆解 → 判断 → `Apply Gameplay Effect`（`GE_Damage`）。 |
| **网络同步：服务器才算伤害** | ⑩ `Has Authority` 判断 + ④ 的注释"target data 会复制给我们"。 |

---

## 六、最妙的细节：表现（Cue）与逻辑（Effect）分离

注意 ⑨ 和 ⑪ 是**两个不同节点**，这是 Lyra 的核心设计：

| 节点 | 干什么 | 谁能执行 | 目的 |
|------|--------|---------|------|
| `Execute Gameplay Cue`（⑨） | **纯表现**：命中火花、音效、镜头震动 | **所有客户端**都能播 | 让玩家"看得见" |
| `Apply Gameplay Effect`（⑪） | **真实逻辑**：扣血 | **只有服务器**（`Has Authority`） | 防外挂，权威结算 |

> **一句话**：特效人人能放，伤害只在服务器算。这样即使客户端被篡改，也改不了"到底扣多少血"。

---

## 七、节点速查表（这张图里的关键节点）

| 节点 | 类型 | 作用 |
|------|------|------|
| `K2_ActivateAbility` | Event | 技能激活入口。 |
| `Sequence` | 流程控制 | 同时做两件事（检测 + 播动画）。 |
| `Is Locally Controlled` | 判断 | 只有本地玩家做弹道检测。 |
| `Start Ranged Weapon Targeting` | 远程武器基类函数 | 做射线检测，产出目标数据。 |
| `Play Montage and Wait` | **Ability Task** | 播开火动画并等待结束（异步）。 |
| `Set Timer (by Delegate)` | 定时器 | 延迟 `FireDelayTimeSecs` 触发 `FireComplete`。 |
| `FireComplete` | Custom Event | 连发/点射的后处理回调。 |
| `Get Hit Result From Target Data` | 工具函数 | 取出命中结果。 |
| `Break Hit Result` | 工具函数 | 拆解命中的各字段。 |
| `Execute Gameplay Cue` | GAS 函数 | 播命中特效（表现）。 |
| `Has Authority` | 判断 | 是否服务器权限。 |
| `Apply Gameplay Effect to Target` | GAS 函数 | 施加伤害 GE（逻辑，扣血）。 |
| `End Ability` | GAS 函数 | 结束技能。 |

---

## 八、一句话总结这张图

```
开火 = ①激活 → ②同时[③本地玩家做射线检测 + ⑦播开火动画]
              → ⑤检测出结果 → ⑥⑦拆解命中
              → ⑧命中？→ 是 → ⑨播命中特效(表现) + ⑩⑪服务器算伤害(逻辑)
                        → 否 → 结束
              → ⑧定时器 → ⑨FireComplete(处理连发)
```

> **核心记忆**：一张技能蓝图 = **"几个 Ability Task（异步等动画/等数据）+ 几个判断（命中？有权限？）+ 两个关键动作（播特效 Cue、算伤害 Effect）"**。看懂这个套路，任何 Lyra 技能蓝图都能拆。

---

> **下一步建议**：
> - 想看**跳跃 `GA_Hero_Jump`** 的图 → 对比"瞬发/被动技能"和"持续/武器技能"的图表差异（跳跃没有射线检测，可能直接加跳跃速度）。
> - 想看 **`GameplayCue` 特效蓝图**长啥样 → 命中火花到底在哪个蓝图里、怎么播。
> - 想看 **`GE_Damage` 伤害效果**里填了什么 → 伤害数值、SetByCaller、距离衰减怎么配。
