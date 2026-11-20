# P05 动画表现 · 总概念 — AnimInstance 与动画蓝图的分工

> **玩家此刻**：P04 角色已经能 WASD 移动、鼠标转视角了，但那是"平移"——脚不动，像块滑板在飘。这一课让角色**移动时播走路/跑步动画、停下恢复待机**，看起来"真的在走"。
>
> **本 md 定位**：只讲概念和全景，不写代码。你看完说"懂了/下一步"，我再拆 `.h` 和 `.cpp` 分开教。

---

## 一、先解决一个最大的误解：动画切换"不写在 C++ 里"

很多人（包括以前的我）会以为：走路/跑步切换，是写一段 C++——`if (速度 > X) 播跑步动画`。

**Lyra 不是这么干的。**

走路/跑步的切换，**几乎全在"动画蓝图"（Animation Blueprint）里，用图形化的"状态机 + Blend Space"完成**。C++ 只负责一件极简的事：**每帧把"速度"这类数字算好，送进动画蓝图**。

这就是为什么 `ULyraAnimInstance` 这个 C++ 类**总共才 65 行**——它根本不含任何"切动画"的逻辑。

---

## 二、用类比讲清"两层分工"

想象一个**舞台剧**：

| 角色 | 对应 | 干什么 |
|------|------|--------|
| **幕后报数员** | C++ 的 `AnimInstance` | 每帧拿传感器测一遍"演员现在速度是多少、离地多高"，然后把数字写在黑板上 |
| **舞台导演** | 动画蓝图的状态机 | 盯着黑板上的数字，决定"现在该让演员走路、还是跑步、还是站着" |
| **道具师** | Blend Space | 让"走"和"跑"两个动作之间平滑过渡，不突兀 |

**关键点**：
- 报数员（C++）**不做决策**，只提供数据。
- 导演（蓝图状态机）**不做测量**，只根据数据决策。
- 两者通过"黑板"（暴露给蓝图的变量）对接。

---

## 三、Lyra 的 C++ 类 `ULyraAnimInstance` 到底做了什么（65 行拆解）

源码路径：`Source/LyraGame/Animation/LyraAnimInstance.h`（46 行）+ `.cpp`（65 行）。

它只做两件事：

### 1. 每帧算一个数字：`GroundDistance`（离地距离）

```cpp
// LyraAnimInstance.cpp 第 51-64 行（核心）
void ULyraAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    const ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwningActor());
    if (!Character) { return; }

    ULyraCharacterMovementComponent* CharMoveComp =
        CastChecked<ULyraCharacterMovementComponent>(Character->GetCharacterMovement());
    const FLyraCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
    GroundDistance = GroundInfo.GroundDistance;   // ← 把"离地距离"存进蓝图可读的变量
}
```

`GroundDistance` 声明在 `.h` 里，带 `UPROPERTY(BlueprintReadOnly)`——**这就是那块"黑板"**：

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
float GroundDistance = -1.0f;
```

### 2. 把 GameplayTag 自动映射成蓝图变量（`GameplayTagPropertyMap`）

```cpp
UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
```

这个东西的作用：**当角色身上的 GameplayTag 发生变化时，自动同步到动画蓝图里对应的布尔/枚举变量**。比如角色被打死挂上 `Status.Death` 这个 Tag，动画蓝图里的 `IsDead` 变量自动变 true，就能切死亡动画。

> ⚠️ 这一块（GameplayTag 映射）依赖 GAS 和 Lyra 的 CharacterMovementComponent，**P05 先做减法跳过**，只保留"送数据"这件事。减法说明我在教 `.cpp` 时详细讲。

---

## 四、动画蓝图那边（P05 步 3 才动，这里只讲概念）

动画蓝图（Lyra 里叫 `ABP_Mannequin_Base`）里有一个**状态机（State Machine）**，大致长这样：

```
        ┌─────────┐
  速度=0 │  Idle   │ 待机
        └────┬────┘
   速度>0 走  │  速度>0 跑
        ┌────▼────┐
        │  Locomotion │ 移动（里面套 Blend Space）
        └─────────┘
             │
      Blend Space：横轴 = 速度
      速度小 → 走路动画   速度大 → 跑步动画
```

**Blend Space（混合空间）**：一个二维坐标图，横轴通常是"速度"。把"走路动画"放在速度 150 的位置、"跑步动画"放在速度 600 的位置，中间速度会**自动在两个动画间按比例混合**，所以从走变跑是平滑的，不是突然跳帧。

---

## 五、P05 三步全景（先看清楚，再动手）

| 步 | 干什么 | 是 C++ 还是蓝图 | 对应 Lyra |
|----|--------|----------------|-----------|
| 步 1 | 认识分工（就是本 md） | 概念 | `LyraAnimInstance` / `ABP_Mannequin_Base` |
| 步 2 | 建 `TenkaichiAnimInstance`（送数据） | **C++**（`.h` + `.cpp`） | `LyraAnimInstance` |
| 步 3 | 动画蓝图搭状态机 + Blend Space | **编辑器（蓝图）** | `ABP_Mannequin_Base` |

---

## 六、一个必须先说清的现实（避免你后面踩坑）

`ULyraAnimInstance` 依赖两个 Lyra 自带、**你现在还没有**的类：

1. `ALyraCharacter`（Lyra 的角色类，带一堆自定义）
2. `ULyraCharacterMovementComponent`（Lyra 自定义的移动组件，里面有 `GetGroundInfo()` 算离地距离）

**P05 的减法策略**（铁律 16：只做减法）：
- 我们**不做**"离地距离"（`GroundDistance`）这块——因为那需要整套 Lyra 的 CharacterMovementComponent，太重。
- 我们**只保留核心思路**：C++ 的 `AnimInstance` 每帧算好**速度**（用引擎自带的 `GetVelocity().Size2D()`，不需要 Lyra 自定义组件），暴露成 `BlueprintReadOnly` 变量，送进动画蓝图。
- 动画蓝图那边，用"速度"这个数据去驱动 Idle/Walk/Run 状态机 + Blend Space。

> 这样"一比一还原"的是**分工思想**（C++ 送数据、蓝图切动画），而不是照抄 Lyra 那套依赖链——Lyra 那套依赖你现在够不着，照抄会卡死。

---

## 七、下一步

看完这个总概念，如果你确认：
1. 理解"动画切换不在 C++、在动画蓝图"这个点；
2. 接受"先做减法、只送速度、跳过 GroundDistance/GameplayTag 映射"这个策略；

回我"**懂了/下一步**"，我就开始教 **步 2 的 `.h`**（建 `TenkaichiAnimInstance` 的类声明），`.h` 和 `.cpp` 分开教、分开建 md。