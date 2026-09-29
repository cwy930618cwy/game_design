# P04-1 — Enhanced Input 三件套 + 建 TenkaichiInputConfig（InputAction→GameplayTag 字典）

> **本步定位**：P04 第 1 步。先搞懂 Lyra 用的「新版增强输入（Enhanced Input）」三件套是什么，再建第一个输入类 `TenkaichiInputConfig`——把「按键动作」和「GameplayTag」绑成一本字典。
>
> **玩家此刻**：角色还站桩。要让 WASD 能走、鼠标能转视角，第一步得先给「玩家的操作」铺一条能进游戏的通道。
>
> **本步拆法**：`.h` 和 `.cpp` 分开教、分开建 md（沿用 P01/P02 的 `_h`/`_cpp` 规划）：
> - 本文件 = 概念 + 三件套 + 为什么用 Tag（`.h` 建在哪、是什么）
> - 下一个文件 `P04-1b_建TenkaichiInputConfig_h.md` = 具体写 `.h`
> - 再下一个 `P04-1c_建TenkaichiInputConfig_cpp.md` = 具体写 `.cpp`

---

## 一、这一步要解决什么问题（先讲为什么）

现在角色已经能被 GameMode 生成出来（P02），但它是「死的」——不管按什么键都没反应。

原因：**玩家按下的键，现在没有任何东西去「接住」它，更别说翻译成「往前跑」「转视角」这种动作了。**

所以这一步要做两件事：

1. **认识「键 → 动作」这条链是怎么搭的**（Enhanced Input 三件套）。
2. **建一本「翻译字典」`TenkaichiInputConfig`**——把「一个动作（如 Move）」和「一个 GameplayTag（如 `InputTag.Move`）」对应起来。

> 为什么是「字典」而不是「直接写死按键」？这是 Lyra 的核心设计，下面第三点讲。

---

## 二、Enhanced Input 三件套（先讲清概念）

Lyra 用的是 **新版增强输入（Enhanced Input）**，不是 UE 老版的「轴映射（Axis Mapping）/ 动作映射（Action Mapping）」。

它的核心是三个东西：

| 三件套 | 是什么 | 作用（类比） |
|--------|--------|-------------|
| **InputAction**（输入动作） | 一个「抽象动作」的资产，如 `IA_Move`、`IA_Look` | 一个「动词」，比如「走路」「看」 |
| **InputMappingContext**（输入映射上下文） | 把「具体按键」映射到「InputAction」的资产 | 一本「按键说明书」：W/A/S/D → 走路 |
| **触发事件**（TriggerEvent） | 动作被触发的时机 | 「按下的瞬间」「按住持续」「松开」 |

**一句话**：**按键（MappingContext 里配）→ 触发 InputAction → 代码收到回调**。这条链就是「玩家操作进游戏」的入口。

> 我们这一步**先不碰 MappingContext 的具体按键配置**（那属于资产配置，后面 P04 步 4 才需要）。这一步只建「InputAction → GameplayTag」的字典。

---

## 三、为什么用 GameplayTag 当「翻译官」，而不是直接引用 InputAction？

这是 Lyra 输入体系最关键的一个设计，也是这一步的重点：

- 如果技能/代码里直接写「`IA_Move` 这个资产」，那么代码就和「具体按键」绑死了。
- Lyra 的做法是：**代码只认 GameplayTag（如 `InputTag.Move`），不认具体按键**。
- `InputConfig` 这个「字典」负责把 `InputTag.Move` → `IA_Move`（InputAction）翻译过去。

好处：**解耦**。以后想让「走路」从 WASD 改成手柄摇杆，只改字典/映射配置，代码一行不用动。技能系统（GAS）也只认 Tag，不认按键。

> 类比：代码只喊「我要『前进』这个动作」（Tag），具体「前进」是 W 键还是摇杆，由字典（InputConfig）+ 按键说明（MappingContext）去翻译。代码不关心你的硬件是什么。

---

## 四、这一步的减法（相对 Lyra，我们砍掉了什么）

按铁律「只做减法、说清删了什么」，对照 Lyra 这一步我们**暂时不教**的部分：

| Lyra 有 | 我们这一步 | 为什么先不教 |
|---------|-----------|-------------|
| `FindAbilityInputActionForTag` | 先保留声明，但技能相关逻辑（AbilityInputActions 数组）留到阶段三再讲 | 技能输入是 GAS 那套，现在用不到 |
| `.cpp` 里的 `LogLyra` 日志通道 | 换成项目自己的日志通道（`LogTenkaichi`），或暂时用 `UE_LOG` | 日志通道是 P01 之外的独立话题 |

> 其余（结构体、字典类、NativeInputActions、FindNativeInputActionForTag）**一比一照抄**，只把前缀 `Lyra` 换成 `Tenkaichi`。

---

## 五、下一步

- 先看下一个 md：`P04-1b_建TenkaichiInputConfig_h.md`（写 `.h`）
- 再看：`P04-1c_建TenkaichiInputConfig_cpp.md`（写 `.cpp`）