# 02 — GameplayCueNotifies 资源详解（技能表现）

> **定位**：单独讲 Lyra Content 里的 `GameplayCueNotifies/` 目录——存放 **GameplayCue（GC）表现资产**，负责"技能/效果发生时的视听表现"（特效/音效/震屏）。
>
> **关联**：
> - [UE5.6_源码分析/.../01_GAS技能系统](../../UE5.6_源码分析/02_Runtime插件详解/01_GameplayAbilities_GAS技能系统.md) — GAS 四大概念地基
> - [03_GameplayEffects详解](./03_GameplayEffects详解.md) — GE 如何通过 Tag 触发 GC
>
> **一句话**：`GameplayCueNotifies/` 存的是"看起来/听起来怎么样"的资产。它**不影响游戏性**，删掉也不影响玩法，但没了它技能就"没感觉"。靠 **GameplayTag** 与 GE/GA 连接。

---

## 一、GameplayCue 是什么

**GameplayCue（GC）= 技能或效果发生时的"视听表现"**。

| 对比 | GameplayEffect（GE） | GameplayCue（GC） |
|------|---------------------|------------------|
| 管什么 | **数值变化**（伤害/治疗/Buff） | **视听表现**（特效/音效/震屏） |
| 影响游戏性 | ✅ 会改变战局 | ❌ 纯观感，删了不影响玩法 |
| 网络处理 | 服务端权威，精确同步 | 可本地预测/可丢弃，省带宽 |
| 例子 | "扣 30 血""中毒 5 秒" | "爆炸特效""命中音效""受击震屏" |

> ⚠️ **铁律**：GC 绝不能影响游戏性。如果"这个表现"需要改变数值，那应该用 GE，而不是在 GC 里做。

### 为什么要把表现单独拆出来

```
没有分离（糟糕）：
  技能代码里直接写：SpawnEmitter()、PlaySound()、ShakeCamera()
  → 逻辑和表现糊在一起，换皮肤要改一堆代码

Lyra 分离（清晰）：
  技能逻辑只管"施加 GE + 贴 Tag"
  表现（GC）独立资产，绑同一个 Tag
  → 换皮肤只换 GC，逻辑一行不改
```

---

## 二、GameplayCueNotifies/ 目录

### 是什么
存放 **GameplayCue Notify 资产**（`.uasset`）。双击打开是配置界面，设置"触发时播什么特效/音效、绑哪个 Tag"。

### 典型内容
```
GameplayCueNotifies/
├── GC_Explosion              ← 爆炸特效+音效
├── GC_HitImpact              ← 命中反馈
├── GC_CameraShake            ← 震屏
├── GC_MuzzleFlash            ← 枪口火焰
├── GC_Death                  ← 死亡表现
└── ...（各种表现）
```

### 三种形态（重要！）

GameplayCue Notify 有三种基类，对应不同表现需求：

| 形态 | 基类 | 特点 | 典型用途 |
|------|------|------|---------|
| **Static（静态）** | `AGameplayCueNotify_Static` | 一次性爆发，**不是 Actor**，是数据资产 | 命中火花、一次性爆炸 |
| **Burst（爆发）** | `AGameplayCueNotify_Burst` | 继承 Actor，单次爆发表现 | 手雷爆炸、落地冲击 |
| **Looping（循环）** | `AGameplayCueNotify_Looping` | 继承 Actor，持续存在 | 地上的光环、燃烧的火堆 |

> **目录名 `GameplayCueNotifies` 说明这里主要放 Static 这类"通知型"数据资产**。Burst/Looping 因为要放场景里，更多出现在关卡或蓝图里。

### Static vs Burst/Looping 的区别

```
Static（本目录主力）：
  └─ 就是个数据资产，不占场景，用完即弃
  └─ 适合：命中火花、枪口火焰这种"一闪而过"的

Burst / Looping：
  └─ 是 Actor，能放场景里，有生命周期
  └─ 适合：持续燃烧的火炬、地面的毒圈
```

---

## 三、核心机制：GameplayTag 连接逻辑与表现

GameplayCue **靠 Tag 触发**——这是整个机制的灵魂。

### 连接流程

```
1. GE/GA 带一个 Cue Tag：GameplayCue.Fire.Explosion
2. GC_Explosion 资产里绑定同一个 Tag
3. 当 GE 施加 / GA 激活 → 系统自动找同 Tag 的 GC → 播放表现
```

```cpp
// GC 资产里设置绑定的 Tag
GameplayTags = (GameplayCue.Fire.Explosion)

// GE 里添加触发 Tag（Granted Tags 或 Trigger Tags）
Trigger Tags = (GameplayCue.Fire.Explosion)
```

### Tag 解耦的意义

```
程序 A 写 GE（只管数值 + 贴 Tag）
美术 B 做 GC（只管特效 + 绑同 Tag）
        ↓
两人完全并行，互不阻塞
        ↓
策划想让"火球"换个新特效？
  → 换一个绑同 Tag 的 GC 即可，GE 一行不用动
```

> **这就是 Lyra 能让美术/程序并行开发、让皮肤替换变得简单的关键设计。**

---

## 四、一个完整例子（火球爆炸）

```
玩家施放火球
   │
   ▼
GA_Fireball 激活
   │
   ├─► 施加 GE_FireballDamage（改数值：目标 -30 血）
   │      └─ GE 带 Tag：GameplayCue.Fire.Explosion
   │             │
   │             ▼
   │      系统匹配到 GC_Explosion（本目录里）
   │             │
   │             ▼
   │      播放：爆炸粒子 + 爆炸音效 + 轻微震屏
   │
   └─► 结果：目标掉血（游戏性）+ 看到爆炸（观感）
```

注意：**GC 的播放是由 Tag 自动触发的，不需要在 GA/GE 里手写 SpawnEmitter**。

---

## 五、美术/策划在哪配

| 想改什么 | 去哪 |
|---------|------|
| 改技能特效外观 | `GameplayCueNotifies/` 里对应 GC，换粒子系统 |
| 改命中音效 | 对应 GC 里换 SoundCue / MetaSound |
| 新增一个表现 | 复制 GC 模板改名，绑对应 Tag |
| 让某效果带新表现 | 给 GE/GA 加一个 Cue Tag 即可 |
| 调整震屏强度 | GC_CameraShake 里改参数 |

> ⚠️ GC 是纯表现资产，修改一般**不影响平衡性**，可以放心调。引用了新特效资源时需重新保存。

---

## 六、常见误区

| 误区 | 正确理解 |
|------|---------|
| "GC 也能改数值" | ❌ GC 只管表现，**绝不影响游戏性**；改数值用 GE |
| "GC 必须和 GE 同名" | 靠 **Tag** 关联，名字随便 |
| "删掉 GC 技能就不灵了" | ❌ 删 GC 只影响观感，技能逻辑照常 |
| "GC 都是 Actor" | 本目录主要是 **Static 数据资产**，不是 Actor |
| "要在 GA 里手写播特效" | ❌ 贴 Tag 即可，系统自动触发 GC |

---

## 七、总结速查

```
GameplayCueNotifies/ = 技能表现的资产库（特效/音效/震屏）

三种形态：
  Static   = 一次性数据资产（本目录主力，如命中火花）
  Burst    = 爆发型 Actor（如手雷爆炸）
  Looping  = 循环型 Actor（如燃烧火堆）

核心机制：GameplayTag 连接
  GE/GA 贴 Cue Tag → 自动触发同 Tag 的 GC → 播放表现

关键特性：
  ├─ 纯观感，不影响游戏性
  ├─ 可本地播放、可丢弃，省网络带宽
  └─ 美术程序并行开发，换皮肤只换 GC

分工对比：
  GE（GameplayEffects/）= 打多少血（游戏性）
  GC（GameplayCueNotifies/）= 看起来怎样（观感）
```

**一句话**：`GameplayCueNotifies/` 管"技能看起来/听起来怎么样"。它通过 **GameplayTag** 与 GE/GA 连接，实现**逻辑与表现分离**——这是 Lyra 技能系统的核心设计，也是换皮肤、换特效而不改逻辑的关键。

---

## 八、下一步

- [UE5.6_源码分析/.../01_GAS技能系统](../../UE5.6_源码分析/02_Runtime插件详解/01_GameplayAbilities_GAS技能系统.md) — GAS 四大概念地基
- [03_GameplayEffects详解](./03_GameplayEffects详解.md) — GE 如何触发 GC
- [04_角色与动画资源](./04_角色与动画资源.md) — 角色如何配合播放技能动画
