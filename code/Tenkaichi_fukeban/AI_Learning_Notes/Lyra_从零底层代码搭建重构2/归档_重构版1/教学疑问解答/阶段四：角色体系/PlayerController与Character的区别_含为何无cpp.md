# PlayerController 与 Character 的区别（含"为什么 CameraAssistInterface 没有 .cpp"）

> 阶段四：角色体系 · 4.7（Character）→ 4.8（PlayerController）
> 疑问来源：教 4.8.1 PlayerController 时，把"角色 Pawn"和"玩家控制器"搞混了，并疑问为什么 `CameraAssistInterface` 不教 `.cpp`。

---

## 一、一句话答案

**Character（角色）= 这辆车本身（能跑、能跳、会受伤、长什么样）。**
**PlayerController（玩家控制器）= 开车的司机（按什么键、看哪里、属于哪个队、怎么操作这辆车）。**

两者是**两套完全不同的东西**，在 Lyra 里各管各的，靠 **`Possess`（占据）** 关系连起来。

---

## 二、逐项对比

| 维度 | `TenkaichiCharacter`（4.7） | `TenkaichiPlayerController`（4.8） |
|------|------------------------------|-------------------------------------|
| **本质** | 一个 **Pawn**（`ACharacter` 子类），是**场景里的实体** | 一个 **Controller**（`APlayerController` 子类），**不是场景实体** |
| **能不能看见** | ✅ 玩家能看到它（有 Mesh、有碰撞、有动画） | ❌ 看不到（没有 Mesh、没有碰撞，是一双"看不见的手"） |
| **管的职责** | 移动（CharacterMovement）、动画、外观、受伤 | 输入、相机、队伍、回放、作弊、自动奔跑 |
| **复制** | 它自己是会同步的（位置/旋转/动画） | 控制器本身**不复制**，只有"操作意图"通过命令发给服务器 |
| **谁拥有它** | 被一个 Controller `Possess`（占据） | 本地玩家专属（司机只有一个） |
| **生命周期** | 死了/重生，Pawn 会换 | 控制器**一直存在**，贯穿整个对局 |
| **和玩家状态** | 通过 `GetController()` 找"谁是司机" | 通过 `GetPawn()` / `GetPlayerState()` 找"车"和"行驶证" |

> **记忆口诀**：
> - 司机（PlayerController）**不换**，车（Character）**可以换**（死了换一辆、重生换一辆）。
> - 一辆车同时只有一个司机（`Possess`），一个司机同时只开一辆车（`Possess` 的 Pawn）。
> - 司机开不了车就只是"空手操作"（`PlayerController` 不一定有 Pawn），车没人开就停在原地（Pawn 不一定有 Controller）。

---

## 三、为什么之前先教 Character，现在才教 PlayerController

阶段四的依赖顺序是 **Character（4.7）在前，PlayerController（4.8）在后**，这不是随便排的：

```
Character（4.7）
   │  继承自 Pawn / CharacterMovement
   │  （车的本体先造好）
   ▼
PlayerController（4.8）
     司机要"开"一辆车，前提是车已经存在
     司机靠 Possess 找到车、调用车的接口
```

但注意一个**绕回来**的点：Character 内部会反过来 `GetController()` 拿 PlayerController 去查询（比如取 ASC、取队伍）。所以这俩**互相引用**，是典型的"死结"（铁律 35：代码照写 + 注释挂起，不纠结）。

---

## 四、为什么 CameraAssistInterface 不教 `.cpp`（先回答用户疑问）

> ⚠️ **这不是我省略，是 Lyra 本来就没有这个 `.cpp`。**

我核实过 Lyra 源码（`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Camera\` 目录）：

```
LyraCameraAssistInterface.h   ← 只有这个 .h，没有任何 .cpp
```

**原因**：这是一个**纯接口（interface）**，3 个方法全是**内联空实现**：

```cpp
virtual void GetIgnoredActorsForCameraPentration(...) const { }  // 空实现
virtual TOptional<AActor*> GetCameraPreventPenetrationTarget() const { return ...; }  // 空实现
virtual void OnCameraPenetratingTarget() { }  // 空实现
```

- **纯接口** = 只定义"口子"，不自己实现逻辑，逻辑全在**实现它的类**里。
- 这 3 个方法**都在 `.h` 里用 `{ }` 直接写了空实现**，函数体一行都没有独立到 `.cpp` 的必要。
- 所以 Lyra **没有** `LyraCameraAssistInterface.cpp`，工程也就**不建**（铁律 16 禁止加法，Lyra 没有的我不发明）。

**对比**：`Character` / `PlayerController` 这种**有真实逻辑**的类，逻辑写在 `.cpp` 里，所以有 `.cpp`。而 `CameraAssistInterface` 纯接口无逻辑，就**只有 `.h`**。

> **判断标准**：Lyra 源码里**有 `.cpp` 的文件才教 `.cpp`**；Lyra 源码里**没有 `.cpp` 的文件（纯接口/纯头文件）就不教**，只教 `.h`。这是"一比一还原"，不是我省步骤。

---

## 五、CameraAssistInterface 的接口到底给谁实现？

它虽然自己没有 `.cpp`，但**接口要被别的类实现**。在 Lyra 里，实现它的是：

| 实现者 | 实现了哪个方法 |
|--------|----------------|
| `LyraPlayerController`（4.8.1） | `OnCameraPenetratingTarget()` —— 相机穿透时隐藏 view target |
| `LyraCameraMode_ThirdPerson`（第三人称相机，阶段六） | 查询"允许穿透名单 / 禁止穿透目标"来驱动相机 |

所以：
- `CameraAssistInterface.h` = 纯接口（0 依赖，无 `.cpp`）→ **只教 `.h`**。
- 真正实现逻辑在 `PlayerController.cpp` 和 `CameraMode_ThirdPerson.cpp` 里 → 那才是教 `.cpp` 的地方。

---

## 六、对照表（什么类有 `.cpp`，什么类没有）

| 类型 | 例子 | 有没有 `.cpp` | 为什么 |
|------|------|---------------|--------|
| **有真实逻辑的类** | Character / PlayerController / PlayerState | ✅ 有 | 逻辑写在 `.cpp` |
| **纯接口（内联空实现）** | CameraAssistInterface / TeamAgentInterface | ❌ 没有 | 3 个方法全 `{ }` 空实现，无需 `.cpp` |

> **通用规律**：**纯接口类（`I` 开头、方法全空实现）在 Lyra 里通常没有 `.cpp`**。判断依据永远回到 Lyra 源码——**源码里没有的文件，工程就不建**。

---

## 待确认

1. "Character=车，PlayerController=司机，靠 Possess 连接"这个类比懂了吗？
2. "CameraAssistInterface 没有 `.cpp`，是因为 Lyra 本来就没有（纯接口内联空实现），不是我省略"——清楚了吗？
3. 确认后我继续教 **4.8.1-2 `TenkaichiPlayerController.h`**。