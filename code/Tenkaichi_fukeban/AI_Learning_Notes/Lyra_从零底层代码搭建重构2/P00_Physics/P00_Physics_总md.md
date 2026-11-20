# P00 Physics（物理）—— 总 md

> **目录**：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\Physics\`
> **对应工程**：`E:\ue5\game_design\code\Tenkaichi\Source\TenkaichiGame\Physics\`
> **版本**：重构版2 v1（2026-10-20）｜ **P00 是"文件夹层"的最底层，谁都不依赖**。注意：它排在**第 0 步（工程骨架 + 底层插件）之后**——先搭好工程配置，再进文件夹，P00 是第一个要教的文件夹。

---

## 一、这个文件夹解决什么问题（为什么）

游戏里要判断"子弹打中了什么"、"角色和谁互动"，靠的就是**碰撞检测**。而碰撞检测有个前提：你得先定义"有哪些碰撞通道（Channel）"、"物体是什么材质"。

Lyra 的 `Physics/` 文件夹就干这两件事：

1. **`LyraCollisionChannels.h`** —— 定义游戏专属的**碰撞通道**（武器命中、交互检测用哪条射线通道）。
2. **`PhysicalMaterialWithTags`** —— 给物理材质**挂 GameplayTag**，让"这块地面 / 这面墙 / 这堵盾"能被游戏逻辑按标签识别（比如"这是金属，子弹打上去该弹开还是穿透"）。

> 🔑 **一句话**：这个文件夹 = 游戏世界最底层的"**碰撞规矩 + 材质身份证**"。它不依赖工程里任何其他类，所以最先教。

---

## 二、全景：这个文件夹里有什么（3 个文件）

| 文件 | 大小 | 类型 | 干嘛的 |
|------|------|------|--------|
| `LyraCollisionChannels.h` | 24 行 | **纯头文件（宏）** | 定义 4 条游戏专属射线碰撞通道（Interaction / Weapon / Weapon_Capsule / Weapon_Multi） |
| `PhysicalMaterialWithTags.h` | 28 行 | 类声明 | 声明 `UPhysicalMaterialWithTags`：继承引擎物理材质，加一个 `FGameplayTagContainer Tags` |
| `PhysicalMaterialWithTags.cpp` | 11 行 | 类实现 | 构造函数，啥都不做（空实现） |

**依赖树**（被依赖在下）：

```
PhysicalMaterialWithTags.h / .cpp
   └── 引擎 UPhysicalMaterial（物理材质基类）＋ GameplayTagContainer（标签容器）
LyraCollisionChannels.h
   └── 引擎 ECC_GameTraceChannel1~4（引擎预留的游戏通道）★ 无任何工程内依赖
```

**叶子/节点判断**：三个文件**全部是叶子**（只依赖引擎），不依赖工程内任何类，**可独立编译，无需注释挂起**。✅

---

## 三、关键概念：碰撞通道（Trace Channel）是什么

UE 的碰撞分两类：

| 类型 | 英文 | 用途 |
|------|------|------|
| **物体通道**（Object Channel） | ObjectChannel | "我是谁"：玩家、敌人、墙壁…… |
| **射线通道**（Trace Channel） | TraceChannel | "我要查什么"：射线打出去看碰到啥 |

- **物体通道**是"我是哪种东西"（角色、静态物、可拾取物），物体之间能不能碰靠它。
- **射线通道**是"我这条射线要查哪类东西"（射击、交互、视线），**一次只查一类**，查谁由通道决定。

引擎预留了 **`ECC_GameTraceChannel1` ~ `ECC_GameTraceChannel18`** 给游戏自己用。Lyra 在 `LyraCollisionChannels.h` 里把它们**起了别名**：

| 宏名 | 底层通道 | 用途 |
|------|---------|------|
| `Lyra_TraceChannel_Interaction` | `ECC_GameTraceChannel1` | 交互检测（准星指着物体） |
| `Lyra_TraceChannel_Weapon` | `ECC_GameTraceChannel2` | 武器射线（打物理资产，不打胶囊） |
| `Lyra_TraceChannel_Weapon_Capsule` | `ECC_GameTraceChannel3` | 武器射线（打胶囊，不打物理资产） |
| `Lyra_TraceChannel_Weapon_Multi` | `ECC_GameTraceChannel4` | 武器射线（穿透多个敌人，不停在第一个） |

> 💡 注意：这里**只是给通道起名字**（宏别名），真正"哪条通道查谁"的规则在 `Config/DefaultEngine.ini` 的 `[/Script/Engine.CollisionProfile]` 里配置。Lyra 头文件顶部注释也提醒了这一点——**改这里的通道名，必须同步改 ini**。

---

## 四、关键概念：PhysicalMaterialWithTags 为什么需要 Tag

UE 自带的 `UPhysicalMaterial`（物理材质）决定"这块材质的物理属性"（摩擦、弹性、声效），但**无法表达"游戏逻辑标签"**。

Lyra 的 `UPhysicalMaterialWithTags` 在引擎物理材质基础上**加了一个 `FGameplayTagContainer Tags`**：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
FGameplayTagContainer Tags;
```

于是美术在材质上挂几个 GameplayTag（比如 `Material.Metal` / `Material.Flesh`），游戏代码就能这样判断：

> "这个物体是金属材质" → 子弹穿透 / 打火花
> "这个是肉体材质" → 子弹停下 / 飘血

**好处**：不用写死枚举，新增一种材质行为只要加个 Tag，不用改 C++。

---

## 五、怎么教（推进节奏）

按铁律 35/20：**先建本总 md（已建）→ 停，等确认 → 教 `.h` → 停 → 教 `.cpp`**。

P00 内容极简，拆两个教学步骤：

| 步骤 | 内容 | 状态 |
|------|------|------|
| P00.1 | `LyraCollisionChannels.h`（纯头文件，4 条通道宏） | ✅ 已教（`P00.1_LyraCollisionChannels_h.md`） |
| P00.2 | `PhysicalMaterialWithTags.h` + `.cpp`（材质挂 Tag 类） | ✅ 已教完（`P00.2_总md` + `P00.2-2_h` + `P00.2-3_cpp`） |

> 依赖都就位（只依赖引擎），**无需注释挂起**，可一次性写完编译通过。
> **✅ P00 Physics 全部教完**（3 文件）：纯叶子文件夹，只依赖引擎，可独立编译，无挂起。

---

## 六、依赖分析总结（铁律 30/31 核查）

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `LyraCollisionChannels.h` | 无 | ✅ 能 | 否 |
| `PhysicalMaterialWithTags.h` | 无 | ✅ 能 | 否 |
| `PhysicalMaterialWithTags.cpp` | 无 | ✅ 能 | 否 |

**结论**：P00 是纯叶子文件夹，可完整教完并编译，无死结、无断点。

---

**P00 Physics 已全部教完**（3 文件：`LyraCollisionChannels.h` + `PhysicalMaterialWithTags.h/.cpp`）。下一步进入 **P01 Messages（消息总线，9 文件，依赖 GameplayMessageRouter 插件）**。