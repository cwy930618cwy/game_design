# 什么该暴露给蓝图/编辑器？为什么 ALyraPawn 反而"不暴露"？

> **定位**：彻底搞懂 UE 里"C++ 可见性"和"蓝图可见性"是两套独立开关，以及——**到底什么东西才应该暴露给蓝图/编辑器**。
>
> **关联**：
> - [01_LyraPawn.h详解](./01_LyraPawn.h详解.md) — `LYRAGAME_API` vs `MinimalAPI`
> - [06_接口Interface与类Class的区别](./06_接口Interface与类Class的区别.md)
>
> **一句话**：`ALyraPawn` 不是"不给蓝图用"，而是"**没必要把它的全部细节塞进反射数据库**"。该暴露的（想在编辑器里配、想在蓝图里调的）一定要暴露；不该暴露的（内部实现、简单基类）就藏着。

---

## 一、先破一个误解：ALyraPawn 到底能不能在蓝图里用？

**能！** `ALyraPawn` 是 `UCLASS`，它**默认就能被蓝图继承、能被 Spawn**。`MinimalAPI` 并没有把它关在 C++ 门外。

那为什么说它"不暴露"？——准确的说法是：

> **`MinimalAPI` 只是不让它把"全部反射细节"都导出，而不是不让蓝图用它。**

要理解这点，得先分清 UE 里**两套完全独立的可见性开关**：

| 开关 | 写法位置 | 控制什么 | 类比 |
|------|---------|---------|------|
| **C++ 链接可见性** | `#define UE_API LYRAGAME_API` | 别的模块能不能 `#include` 并**调用**这个类 | 餐厅大门开不开（别人能不能进厨房） |
| **反射/蓝图可见性** | `UCLASS(XXX)` 括号里的修饰符 | 蓝图和编辑器**能看到多少细节**、能不能序列化 | 菜单和菜谱公开多少 |

**这两套各管各的，互不影响。** `ALyraPawn` 就是：大门开着（C++ 随便用），但菜谱没全贴出来（反射只导出必要的）。

---

## 二、关键：反射数据到底是什么？为什么要"导出"？

UE 有一个工具叫 **UHT（UnrealHeaderTool）**，它会扫描你代码里的 `UCLASS` / `UPROPERTY` / `UFUNCTION` 宏，然后**生成一份"类的说明书"**——这就是**反射数据（Reflection Data）**。

### 反射数据里存了什么？

```cpp
UCLASS()
class AMyCharacter : public ACharacter {
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeed;                    // ← 蓝图能读写、编辑器能显示

    UFUNCTION(BlueprintCallable)
    void DoJump();                      // ← 蓝图能调用

    UPROPERTY(Replicated)
    int32 Health;                        // ← 网络复制要用
};
```

UHT 会为 `AMyCharacter` 生成一份说明书，里面记录：

| 信息 | 用途 |
|------|------|
| 有哪些属性（名字、类型、偏移量） | 编辑器面板显示、存档读档（序列化）、蓝图读写 |
| 有哪些函数（名字、参数） | 蓝图调用、网络 RPC |
| 类的继承关系、默认值 | 创建实例、蓝图节点 |

**这份说明书是有体积的**。每个 `UCLASS` 都会生成一大坨反射代码，打进最终的包体里。

---

## 三、`MinimalAPI` 到底"省"掉了什么？

### 不加 `MinimalAPI`（默认 = `Blueprintable` 那一套完整导出）

- UHT 会导出**完整的反射数据**：所有 `UPROPERTY`、`UFUNCTION` 的元信息全部对外可见。
- 好处：其他模块的蓝图能**完整看到并操作**这个类的每一个细节。
- 代价：**包体变大 + 命名空间污染**（全局多了个 `ALyraPawn` 的完整反射条目）。

### 加 `MinimalAPI`

- UHT **只导出最小必要符号**：让本模块能正常工作的部分，外部不再额外暴露。
- 好处：**包体小、命名空间干净**。
- 代价：外部模块想通过反射访问它的细节会受限（但 C++ 直接调用完全不受影响）。

### 为什么 `ALyraPawn` 适合用 `MinimalAPI`？

因为它是**一个中间基类**：

```
AActor → APawn → AModularPawn → ALyraPawn → ALyraCharacter → 具体的英雄/怪物
```

- 它自己**几乎没逻辑**（构造函数空的，函数都在 .cpp 里调 Super）。
- 它**不是拿来直接在编辑器里配置的终点类**——真正在关卡里放的是它的子类 `ALyraCharacter` 及其蓝图子类。
- 所以"把 `ALyraPawn` 的全部反射细节导出"纯属浪费——**没人会直接拿它做资产**。

> **记忆**：越是"底层基类 / 中间层 / 纯 C++ 工具类"，越适合 `MinimalAPI`；越是"要在编辑器里配置、要给策划用的类"，越需要完整导出。

---

## 四、核心问题：到底什么东西"应该"暴露给蓝图/编辑器？

这是本篇的重点。判断标准就一条：**"这个东西是不是要给【非 C++ 的人 / 运行时的蓝图】用？"**

### ✅ 应该暴露给蓝图/编辑器的

| 东西 | 用什么宏 | 为什么 |
|------|---------|--------|
| **想在编辑器面板里配置的变量** | `UPROPERTY(EditAnywhere/EditDefaultsOnly)` | 策划要在编辑器里调数值（如移速、血量） |
| **想让蓝图读写的变量** | `UPROPERTY(BlueprintReadWrite/BlueprintReadOnly)` | 蓝图逻辑要读/改它 |
| **想从蓝图调用的函数** | `UFUNCTION(BlueprintCallable)` | 蓝图事件里要用 |
| **想在蓝图里响应的事件** | `UFUNCTION(BlueprintImplementableEvent)` | 交给蓝图实现（如"播放攻击动画"） |
| **想网络复制的变量** | `UPROPERTY(Replicated / ReplicatedUsing)` | 联机同步必需（如队伍 ID、血量） |
| **想序列化的数据** | `UPROPERTY(SaveGame)` | 存档读档要用 |
| **想被子类替换的组件** | `UPROPERTY(CreateDefaultSubobject, meta=(AllowPrivateAccess))` | 允许蓝图子类换掉默认组件 |

### ❌ 不该暴露给蓝图/编辑器的

| 东西 | 原因 |
|------|------|
| **纯 C++ 内部实现的辅助函数** | 蓝图永远用不到，暴露了也是噪音 |
| **临时变量、循环计数器** | 生命周期只在一次函数调用内，没必要持久化 |
| **底层基类的实现细节** | 没人直接用它做资产，导出纯属浪费（→ `MinimalAPI`） |
| **性能敏感的热点数据** | 反射有开销，高频访问的数据尽量用普通 C++ 成员 |
| **不想被外部看到的私有状态** | 封装边界，避免被误用 |

---

## 五、一张图看懂"该不该暴露"的判断流程

```
              这个东西是什么？
                    │
        ┌───────────┼────────────┐
        ▼           ▼            ▼
   要在编辑器里    要被蓝图      只是 C++ 内部
   配置/显示？     调用/读写？    实现细节？
        │           │            │
        ▼           ▼            ▼
      暴露！       暴露！       藏着！
   EditAnywhere  Blueprint    普通成员 /
   BlueprintRW   Callable     MinimalAPI
        │           │            │
        └───────────┴────────────┘
                    ▼
        判断标准：非 C++ 的人 / 运行时蓝图
        会不会用到它？
          会 → 暴露（加对应 UPROPERTY/UFUNCTION 宏）
          不会 → 藏着（别加宏，或加 MinimalAPI）
```

---

## 六、回到 ALyraPawn：逐条看它该不该暴露

对照 `ALyraPawn` 的真实成员，看看哪些"该暴露"、哪些"藏着就行"：

| 成员 | 暴露了吗 | 该不该 | 原因 |
|------|---------|--------|------|
| 队伍 ID `MyTeamID` | ✅ `UPROPERTY(ReplicatedUsing=...)` | **该** | 网络复制必需，联机同步队友/敌人识别 |
| 队伍变化委托 `OnTeamChangedDelegate` | ✅ `UPROPERTY()` | **该** | 要让蓝图/其他系统监听队伍变化 |
| `SetGenericTeamId` / `GetGenericTeamId` | ✅ `UE_API`（C++ 调用） | **该** | 队伍系统要统一访问，但走的是接口不是反射 |
| 构造函数 / `PreInitializeComponents` / `EndPlay` | ⚠️ 只调 Super | **无所谓** | 生命周期函数，框架自动调，不需要蓝图操心 |
| `DetermineNewTeamAfterPossessionEnds` | 🔒 普通 virtual（无宏） | **该藏** | 内部钩子，留给子类 C++ 重写即可，蓝图用不到 |
| 类的整体反射细节 | 🔒 `MinimalAPI` | **该藏** | 中间基类，没人直接拿它做资产，导出浪费 |

**结论**：`ALyraPawn` 把"**该暴露的**"（队伍 ID、委托——为了网络和监听）都老老实实用 `UPROPERTY` 暴露了；把"**该藏的**"（内部钩子、整体反射细节）用 `MinimalAPI` + 不加宏藏起来了。**这不是"不给蓝图用"，而是"该露的露、该藏的藏"，刚刚好。**

---

## 七、对比：一个"该完全暴露"的类长什么样？

假设有个 `ABP_Hero`（英雄蓝图基类），策划要在编辑器里配它、蓝图要驱动它：

```cpp
// 这种类就该完整导出，甚至允许蓝图继承
UCLASS(BlueprintType, Blueprintable)   // ← 明确说：蓝图能用、能继承
class ABP_Hero : public ACharacter {
    GENERATED_BODY()

public:
    // 策划要在面板里配的 → 暴露
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    float MoveSpeed = 600.f;

    // 蓝图要调用的技能 → 暴露
    UFUNCTION(BlueprintCallable, Category="Ability")
    void FireProjectile();

    // 交给蓝图实现的动画/特效 → 暴露为事件
    UFUNCTION(BlueprintImplementableEvent)
    void OnDeath();
};
```

对比 `ALyraPawn`：

| | `ALyraPawn` | `ABP_Hero`（假设） |
|---|---|---|
| 定位 | 中间基类，C++ 层用 | 终点类，策划/蓝图用 |
| UCLASS 修饰 | `MinimalAPI`（藏） | `BlueprintType, Blueprintable`（露） |
| 成员宏 | 只有关键数据加 `UPROPERTY` | 大量 `EditAnywhere` / `BlueprintCallable` |
| 谁在用 | 程序（C++ 继承、网络同步） | 策划 + 蓝图 |

---

## 八、常见误区

| 误区 | 正确理解 |
|------|---------|
| "`MinimalAPI` = 不能跨模块用" | ❌ 跨模块 C++ 调用由 `_API` 宏管，`MinimalAPI` 只管反射导出多少 |
| "加了 `MinimalAPI` 蓝图就不能继承它" | ❌ 能不能蓝图继承看 `Blueprintable`，跟 `MinimalAPI` 是两回事 |
| "所有 UCLASS 都该加 `Blueprintable`" | ❌ 只有"要给蓝图继承的终点类"才加；底层基类加了反而污染命名空间 |
| "不想让蓝图用的变量也要加 UPROPERTY" | ❌ 纯 C++ 内部变量就别加宏，加了反而增加反射开销和暴露面 |
| "反射没成本，全都加上无所谓" | ❌ 反射有包体和运行时开销，该藏就藏 |

---

## 九、一张速查表：常用 UCLASS 修饰符

| 修饰符 | 含义 | 什么时候用 |
|--------|------|-----------|
| `BlueprintType` | 这个类可以作为蓝图变量的类型 | 蓝图里要引用它时 |
| `Blueprintable` | 这个类可以被蓝图继承 | 想让策划做蓝图子类时 |
| `NotBlueprintType` | 不能作为蓝图类型 | 纯 C++ 内部类 |
| `MinimalAPI` | 只导出最小反射符号 | 底层基类、减小包体 |
| `Abstract` | 不能直接实例化 | 抽象基类（如 `ALyraCharacter` 常配合使用） |

---

## 十、总结

```
ALyraPawn "不暴露给蓝图" 是个误解，真相是：

1. 它能在蓝图里用（是 UCLASS），MinimalAPI 没把它关在 C++ 门外
2. MinimalAPI 只是"不把全部反射细节导出"，因为它是中间基类，导出浪费

判断"什么该暴露给蓝图/编辑器"的唯一标准：
   👉 这个东西会不会被【非 C++ 的人 / 运行时蓝图】用到？
      会 → 暴露（加 UPROPERTY/UFUNCTION 对应宏）
      不会 → 藏着（不加宏 / MinimalAPI）

该暴露的：编辑器可配项、蓝图读写、蓝图调用、网络复制、序列化
该藏的：  内部辅助函数、临时变量、底层基类细节、性能热点、私有状态
```

**一句话**：暴露给蓝图的不是"整个类"，而是"**一个个具体要被外部使用的成员**"。`ALyraPawn` 把该暴露的队伍 ID、委托都暴露了，把该藏的内部细节藏起来了——这才是正确的暴露姿势。

---

## 十一、下一步

- [01_LyraPawn.h详解](./01_LyraPawn.h详解.md) — 回到源码逐行看
- [06_接口Interface与类Class的区别](./06_接口Interface与类Class的区别.md) — 为什么队伍系统走接口而不是反射
- [04_PawnData何时由谁挂载](./04_PawnData何时由谁挂载.md) — PawnData 是怎么"塞"进角色的（涉及暴露策略）
