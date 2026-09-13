# LyraActorUtilities 是干嘛的

> 文件：`LyraStarterGame/Source/LyraGame/System/LyraActorUtilities.h`（只有 43 行）+ 同名 `.cpp`（40 行）
> **一句话**：Lyra 自己造的一个"蓝图小工具"，作用只有一个——**让蓝图能判断"当前是单机 / 独立服务器 / 监听服务器 / 客户端"**。

---

## 0. 先讲这个需求是怎么来的

联机游戏里，"我是服务器还是客户端"决定了代码该怎么走：

- 服务器才有资格判定伤害、扣血、生成怪物；
- 客户端只负责显示和输入；
- 单机（Standalone）本质上也是服务器，因为它本地什么都能干。

UE 引擎里表示这件事的枚举叫 `ENetMode`（在 `EngineTypes.h` 里）：

```
 NM_Standalone       单机（同时也是"某种服务器"）
 NM_DedicatedServer  独立服务器（没有本地玩家）
 NM_ListenServer     监听服务器（自己当房主还能玩）
 NM_Client           客户端（连别人的）
```

问题是：**这个 `ENetMode` 是 C++ 枚举，蓝图里用不了**（没有暴露给蓝图）。

于是 Lyra 在 `System/` 里放了这个小文件，做两件事：

1. 自己定义一个**蓝图可见**的同款枚举 `EBlueprintExposedNetMode`；
2. 提供一个**蓝图可调用的函数** `SwitchOnNetMode`，让蓝图像用 `switch` 节点一样分岔。

> 顺便说明：这个文件跟"游戏玩法"无关，纯属**给策划/蓝图同学补工具**，所以它被放在 `System/`（基础工具层）。

---

## 1. 枚举 `EBlueprintExposedNetMode`（第 12～29 行）

```cpp
UENUM()
enum class EBlueprintExposedNetMode : uint8
{
    Standalone,        // 单机：没有网络，本地一个或多个玩家，但具备全部服务器功能
    DedicatedServer,   // 独立服务器：没有本地玩家
    ListenServer,      // 监听服务器：自己开房自己还玩
    Client             // 客户端
};
```

注意几个点：

- **`UENUM()` 但没写 `BlueprintType`**：不过它是作为下面函数的"展开执行引脚"用的，配合 `ExpandEnumAsExecs` 就能在蓝图里显示成多个输出引脚（见下）。
- **顺序是刻意排的**：`Standalone(0) → DedicatedServer(1) → ListenServer(2) → Client(3)`。源码注释把关键用法写在了 `Client` 上（第 24～28 行）：

  > "**每个比 Client 小的值都是某种服务器**，所以用 `NetMode < NM_Client` 判断"是不是服务器"永远成立。"

  这就是老 UE 程序员常用的写法：**`< NM_Client` 一律当服务器处理**。Lyra 的枚举顺序和引擎保持一致，方便换算。
- 枚举值带了很清楚的英文注释（单机/独立服务器/监听服务器/客户端），这是 UE 源码里少见的"新手友好"写法，值得留意。

---

## 2. 函数 `SwitchOnNetMode`（第 41～42 行）

```cpp
UFUNCTION(BlueprintCallable, Category="Lyra", meta=(WorldContext="WorldContextObject", ExpandEnumAsExecs=ReturnValue))
static EBlueprintExposedNetMode SwitchOnNetMode(const UObject* WorldContextObject);
```

逐一拆开看：

| 修饰 | 意思 |
|---|---|
| `UCLASS()` + `: public UBlueprintFunctionLibrary`（第 32～33 行） | 这是**蓝图函数库**：只提供静态函数，不用实例化，蓝图里搜得到 |
| `BlueprintCallable` | 蓝图里可以调用 |
| `Category="Lyra"` | 在蓝图节点菜单的 "Lyra" 分类下 |
| `WorldContext="WorldContextObject"` | 蓝图里不用手填世界，编辑器自动把"当前所在的世界"传进来 |
| **`ExpandEnumAsExecs=ReturnValue`** | ★最关键：把返回值这个枚举**展开成多个执行输出引脚** |
| `static` | 无状态工具函数，不需要对象 |

**`ExpandEnumAsExecs` 到底做了什么？** 这个函数在蓝图里长这样：

```
       ┌─ Standalone      （走这条线 = 当前是单机）
  ──▶ SwitchOn ─┬─ DedicatedServer （走这条线 = 独立服务器）
   Net Mode      ├─ ListenServer    （走这条线 = 监听服务器）
                 └─ Client          （走这条线 = 客户端）
```

也就是说，蓝图里**不需要手写 4 个 `==` 判断 + 分支**，直接从这个节点拉 4 条执行线，各接各的逻辑。
`SwitchOnXxx` 这种命名约定，就是"给蓝图用的 switch 节点"的意思。

---

## 3. 实现在 `.cpp` 里干了什么（第 9～40 行）

```cpp
EBlueprintExposedNetMode ULyraActorUtilities::SwitchOnNetMode(const UObject* WorldContextObject)
{
    ENetMode NetMode = NM_Standalone;

    for (const UObject* TestObject = WorldContextObject; TestObject != nullptr; TestObject = TestObject->GetOuter())
    {
        if (const UActorComponent* Component = Cast<const UActorComponent>(WorldContextObject))
        {
            NetMode = Component->GetNetMode();
            break;
        }
        else if (const AActor* Actor = Cast<const AActor>(WorldContextObject))
        {
            NetMode = Actor->GetNetMode();
            break;
        }
    }

    switch (NetMode) { ... 把引擎枚举翻译成蓝图枚举 ... }
}
```

思路分两步：

1. **拿到引擎的 `ENetMode`**：如果传进来的是 `UActorComponent`，就问组件；如果是 `AActor`，就问 Actor（`GetNetMode()` 是它们的成员函数）。
2. **翻译**：把 `NM_Client / NM_Standalone / NM_DedicatedServer / NM_ListenServer` 一一对应成 `EBlueprintExposedNetMode` 的四个值；翻译不到的（理论上不会发生）走 `default`，用 `ensure(false)` 在开发期报警，然后兜底返回 `Standalone`。

**⚠️ 值得留心的一个实现问题**：
这个 `for` 循环的字面意图是"从传入对象开始，沿 `Outer` 链往上找，直到找到 Actor/Component"（因为 `TestObject = TestObject->GetOuter()` 确实在往上走）。
但**循环体里检查的始终是 `WorldContextObject`（入参本身），不是 `TestObject`**。所以：

- 入参本身是 Actor/Component 时：正常（第一次迭代就 `break`）；
- 入参是个"挂在 Actor 下面的普通 UObject"时：不会去外层找，循环空转几圈后 `TestObject` 变 `nullptr` 退出，`NetMode` 保持默认的 `NM_Standalone`，于是蓝图会误判成"单机"。

换句话说，**上溯 `Outer` 的逻辑没真正生效**。实际使用时为了保证结果正确，最好直接把 Actor 或 Component 传进去（蓝图里这个 `WorldContext` 一般是自动填的，多半就是 Actor/Component，所以平时不容易踩到）。这是读源码时可以记一笔的细节。

---

## 4. 谁在用它

我搜了整个 Lyra 的 `Source/`（C++ 侧）：**只有这个文件自己**。也就是说——

> **它是纯粹给蓝图（`Content/` 里的蓝图资产）准备的工具**，C++ 代码不需要它。

这也是它存在的意义：C++ 程序员本来就随时能调 `GetNetMode()`，但蓝图同学没法直接拿到，所以专门包一个。

---

## 5. 蓝图里典型的使用场景

- UI：只在客户端显示某些提示（比如"你死了，等复活"），服务器不用显示；
- 关卡/玩法蓝图：只有服务器执行生成逻辑；
- 调试：单机时给开发者开一些作弊入口；
- 表现：监听服务器和独立服务器在处理上可以走不同分支。

---

## 6. 一句话总结

`ULyraActorUtilities` 是一个**蓝图函数库**：

- 自己定义了一个**蓝图能看见的网络模式枚举** `EBlueprintExposedNetMode`（值顺序和引擎的 `ENetMode` 一致，方便用 `< Client` 判断服务器）；
- 提供了一个 `SwitchOnNetMode` 节点，靠 `ExpandEnumAsExecs` **在蓝图里长成 4 条执行分支**，一条对应一种网络模式；
- 实现上就是"拿 Actor/Component 的 `GetNetMode()`"再翻译成蓝图枚举。

它属于 Lyra 的**基础设施工具**（所以放在 `System/`），本身不含任何玩法逻辑——**只为让蓝图也能优雅地做"服务器/客户端"分支判断**。
