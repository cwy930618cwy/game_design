# 17 — 生命周期函数：PreInitializeComponents 与 EndPlay 详解

> **定位**：解释 LyraPawn 重写的 `PreInitializeComponents` 和 `EndPlay`——这两个是 **AActor 的生命周期函数**。
>
> **一句话**：`PreInitializeComponents` = 组件初始化**前**调用；`EndPlay` = Actor 结束播放（销毁前）调用。LyraPawn 重写它们但实现为空，**主要是留出"扩展点"**，让子类/组件能挂钩。
>
> **代码**：`LyraPawn.cpp:27-35`

---

## 一、这两个函数是什么（生命周期函数）

它们是 **AActor 的生命周期回调**，在 Actor 的特定阶段被引擎自动调用：

```cpp
// LyraPawn.h 第 28-31 行
//~AActor interface
virtual void PreInitializeComponents() override;   // 组件初始化前
virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;  // 结束播放
//~End of AActor interface
```

| 函数 | 什么时候调用 | 顺序 |
|------|------------|:---:|
| `PreInitializeComponents()` | 组件初始化**前** | 最早 |
| `PostInitializeComponents()` | 组件初始化**后** | 之后 |
| `BeginPlay()` | 游戏开始 | 更后 |
| `EndPlay()` | Actor 结束播放（销毁前） | 最后 |

---

## 二、看 LyraPawn 的实现（真实代码，是空的）

看 LyraPawn.cpp 的实现（我之前读的）：

```cpp
// LyraPawn.cpp 第 27-35 行
void ALyraPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();   // 调父类（必须）
}

void ALyraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);   // 调父类（必须）
}
```

**注意**：两个函数**只调了 `Super`**，没有额外逻辑——**实现是空的**。

---

## 三、那"空的为什么还要重写"？（核心）

**这是关键**——Lyra 重写这两个空函数，**不是为了现在做什么，而是为了"留出扩展点"**。

### 理由 1：让子类/继承链能挂钩

```
LyraPawn 重写 = 在继承链上"留了个位置"
  → 子类（如 ALyraCharacter）可以在这里加自己的逻辑
  → 如果 LyraPawn 不重写，子类就没法在这里"override 加东西"
```

**重写空的 = 留出"钩子"**，让更下层的类能在这里扩展。

### 理由 2：`Super` 保证链式传递

```cpp
void ALyraPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();   // 传父类（AModularPawn → ... → AActor）
}
```

**`Super` 保证**：即使 ALyraPawn 自己没逻辑，也要**调用父类的实现**（父类可能有逻辑）。**不调 Super，父类的初始化就断了。**

---

## 四、那这两个函数"真正用来做什么"？（它们的用途）

虽然 LyraPawn 里是空的，但这两个函数**本来是用来做重要的事**的：

### PreInitializeComponents —— 组件初始化前（最早）

**用途**：在所有组件初始化**前**，做最早的设置。**顺序最早**，适合做"基础初始化"。

```cpp
// 用途：最早初始化（例：设置默认值）
void AMyActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	// 设置初始状态（组件还没初始化）
	bIsReady = false;
}
```

### EndPlay —— 结束播放（销毁前）

**用途**：Actor 要销毁/结束播放时，做**清理工作**（释放资源、解绑委托）。

```cpp
// 用途：清理（例：解绑委托、释放资源）
void AMyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理工作
	MyDelegate.Unbind();
	Super::EndPlay(EndPlayReason);   // 最后调父类
}
```

---

## 五、为什么 Lyra 实现是空的？（配场景）

Lyra 把这两个函数**重写成空的**（只调 Super），是因为：

```
LyraPawn 是"基类"
  → 它自己不需要在这两个时机做特殊事
  → 但为了"让子类能扩展"，它重写留出钩子
  → 子类（ALyraCharacter 等）如果需要，就在这里 override 加逻辑
```

**类比**：
```
LyraPawn = 大楼的地基
  → 地基自己不需要装电梯（空）
  → 但留了"电梯井"（重写函数 = 留钩子）
  → 子类（楼层）需要时装电梯（override 加逻辑）
```

---

## 六、总结速查

```
生命周期函数：
  PreInitializeComponents() = 组件初始化前（最早）
  EndPlay() = 结束播放（销毁前）

LyraPawn 重写这两个：
  实现是空的（只调 Super）
  目的 = 留出"扩展点"（钩子）
    → 子类能在这里 override 加逻辑
    → Super 保证父类实现被调用

真正的用途：
  PreInitializeComponents → 最早初始化
  EndPlay → 清理（解绑/释放）
```

**一句话**：`PreInitializeComponents`（组件初始化前）和 `EndPlay`（销毁前）是 **AActor 生命周期函数**。LyraPawn 重写它们但**实现是空的**——**目的是留出"扩展点"**，让子类能在这里加逻辑，同时 `Super` 保证父类实现被调用。

---

## 七、下一步

理解了这两个生命周期函数，下一步可以看 **ALyraPawn 的其他生命周期函数**（PossessedBy/UnPossessed）或 **完整的 Actor 生命周期**。
