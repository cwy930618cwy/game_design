# P05 步 3 — 动画蓝图搭状态机 + Blend Space（速度 → 待机/走/跑）

> **本 md**：教最后一步——在**编辑器（动画蓝图）**里搭状态机 + Blend Space，让角色根据 `Speed` 播走路/跑步动画。
>
> **注意**：这一步**不是写 C++**，而是动编辑器（蓝图）。没有对应的 C++ 代码方式（铁律 19 的例外——动画状态机只能靠蓝图）。

---

## 一、先讲为什么（这一步在干嘛）

回顾分工：C++ 的 `UTenkaichiAnimInstance` 已经每帧把 `Speed` 算好送进蓝图了（步 2 完成）。

现在要做的是**"导演"那一层**——动画蓝图读 `Speed`，决定放哪个动画：

```
Speed = 0        → 待机（Idle）
Speed 小（走路） → 走路动画
Speed 大（跑步） → 跑步动画
```

这三者的切换，用**状态机（State Machine）+ Blend Space（混合空间）**实现。

---

## 二、减法说明（Lyra 的动画蓝图有多复杂，我们砍掉什么）

Lyra 的 `ABP_Mannequin_Base` 是 **1.7MB** 的巨型蓝图，包含：

| Lyra 有的 | 我们 P05 做吗 | 为什么砍 |
|-----------|--------------|----------|
| 多武器动画分层（Linked Layers + AnimLayers） | ❌ 不做 | 那是多武器射击游戏才需要的，P05 只有徒手走/跑 |
| 按武器分 `ABP_PistolAnimLayers` / `ABP_RifleAnimLayers` 等 | ❌ 不做 | 同上 |
| FootFX 脚步特效、大量 AnimNotifier | ❌ 不做 | 高级表现，教学用不到 |
| 蹲走、冲刺、倾斜（Lean） | ❌ 不做 | 先只做最基本的 待机/走/跑 |

**我们只保留最核心的**：一个动画蓝图 + 一个状态机（Idle/Locomotion）+ 一个 Blend Space（走↔跑混合）。这是 Lyra 思路的"最小子集"。

---

## 三、准备动画资源（先做这个，否则状态机没素材）

搭状态机之前，你得有动画序列（AnimSequence）：
- **待机动画**（Idle）
- **走路动画**（Walk）
- **跑步动画**（Run）

**从哪里拿？**
- 引擎自带：UE5 模板工程里的 Mannequin 有 `Manny` 角色的基础动画（待机/走/跑）。
- 或从 Lyra 项目复制：`Content/Characters/Heroes/Mannequin/Animations/` 下有现成的走跑动画。

> ⚠️ 这一步需要你在引擎里确认有这三个动画序列可用。如果没有，先到"第三人称模板"或 Lyra 里迁移几个进来。

---

## 四、操作步骤（在编辑器里点）

### 第 1 步：建 Blend Space（1D 混合空间）

1. 在内容浏览器，右键 → **动画（Animation）→ Blend Space**。
2. 弹出的类型选 **Blend Space 1D**（一维，因为只有一个参数：速度）。
3. 命名 `BS_Locomotion`（对应 Lyra 的 `BS_MM_Unarmed_Jog_Walk`）。
4. 打开它，在底部**横轴（Axis）**设为 `Speed`（就是步 2 里 C++ 暴露的那个变量），范围填 `0 ~ 600`（单位 cm/s）。
5. 把动画拖进坐标轴：
   - **Walk** 放在 `Speed = 150` 的位置
   - **Run** 放在 `Speed = 600` 的位置
   - （可选）中间加几个过渡点，让混合更平滑

> Blend Space 的作用：角色速度在 150~600 之间时，会自动按比例混合走路和跑步动画，所以"从走变跑"是平滑的，不是突然跳帧。

### 第 2 步：建动画蓝图（Animation Blueprint）

1. 右键 → **动画 → 动画蓝图（Animation Blueprint）**。
2. **父类（Parent Class）选 `TenkaichiAnimInstance`**（这就是步 2 建的那个 C++ 类，关键！）。
3. 目标骨骼（Skeleton）选 Mannequin 的骨骼。
4. 命名 `ABP_Tenkaichi`（对应 Lyra 的 `ABP_Mannequin_Base`，只换前缀）。

### 第 3 步：搭状态机

在动画蓝图里（AnimGraph 标签页）：

1. 删掉默认的"最终动画姿势"节点，右键拖出一个 **状态机（State Machine）** 节点，连到输出。
2. 双击进入状态机，建两个状态：
   - **Idle**（待机）
   - **Locomotion**（移动）
3. 建两条转换规则（Transition）：
   - `Idle → Locomotion`：条件 = `Speed > 0`
   - `Locomotion → Idle`：条件 = `Speed == 0`（或 `Speed < 某阈值`）

### 第 4 步：在 Locomotion 状态里放 Blend Space

1. 双击进入 **Locomotion** 状态。
2. 拖入第 1 步建的 **Blend Space** 节点（`BS_Locomotion`）。
3. Blend Space 的 `Speed` 参数，**直接连到 `Try Get Pawn Owner` → 速度**，或者更简单——动画蓝图里已经能直接读到步 2 暴露的 `Speed` 变量，直接用它驱动 Blend Space 的横轴。

### 第 5 步：把动画蓝图挂到角色

1. 打开角色的骨骼网格组件（Skeletal Mesh Component）详情面板。
2. 找到 **动画（Animation）→ 动画模式（Anim Class）**。
3. 设为 `ABP_Tenkaichi`。

---

## 五、验收标准

进 PIE：
- 角色静止 → 播 Idle 待机动画
- 按 WASD 移动 → 播走路动画
- 跑起来（速度大）→ 平滑过渡到跑步动画
- 停下 → 恢复待机

---

## 六、常见坑

| 坑 | 原因 | 解决 |
|----|------|------|
| 动画蓝图读不到 `Speed` | 父类没选对 `TenkaichiAnimInstance` | 重建动画蓝图，父类选 C++ 那个类 |
| Blend Space 不混合，直接跳 | 没把动画放到正确的 `Speed` 位置，或轴范围不对 | 检查横轴范围和动画摆放位置 |
| 状态机不切换 | Transition 条件写错（如用了 `==` 浮点比较） | 用 `>` 或 `<` 阈值，别用 `==`（浮点相等难命中） |
| 角色还是滑行 | 动画蓝图没挂到骨骼网格组件 | 检查 Anim Class 是否设为 `ABP_Tenkaichi` |

---

## 七、下一步

这一步做完，P05 就完整了：**角色移动时播走路/跑步动画，停下恢复待机**——从"滑行"变成"活人"。

P05 验收通过后，阶段二还剩 **P06（组件化重构）** 和 **P07（第三人称相机）**。回我「**P05 完成**」或告诉我验收结果，我们再推进。