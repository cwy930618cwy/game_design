# Q14 — `GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode` 这串路径怎么对应到我的类

> **问题**：`Config/DefaultEngine.ini` 里写 `GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode`，看晕了——这串路径怎么对应上我 `TenkaichiGameMode.h` 里的 `class ATenkaichiGameMode`？
>
> **一句话**：这不是文件路径，是 UE 的**"类引用路径"**。`/Script/` 表示"来自 C++ 代码"，中间是**模块名**，点后是**类名（去掉 `A` 前缀）**。引擎靠这串字符串在运行时找到你的 `UCLASS`。

---

## 一、问题是什么

你在 `DefaultEngine.ini` 第 5 行写了：

```ini
GlobalDefaultGameMode=/Script/Tenkaichi.TenkaichiGameMode
```

而你的头文件 `TenkaichiGameMode.h` 第 10 行是：

```cpp
class ATenkaichiGameMode : public AGameMode
```

困惑点：ini 里是 `/Script/Tenkaichi.TenkaichiGameMode`，.h 里是 `ATenkaichiGameMode`，两者长得完全不像，怎么就连上了？

---

## 二、根本原因：这个字段存的是"类路径字符串"，不是文件路径

先查引擎源码，看 `GlobalDefaultGameMode` 到底是什么类型：

```211:213:d:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\EngineSettings\Classes\GameMapsSettings.h
	/** GameMode to use if not specified in any other way. (e.g. per-map DefaultGameMode or on the URL). */
	UPROPERTY(config, noclear, EditAnywhere, Category=DefaultModes, meta=(MetaClass="/Script/Engine.GameModeBase", DisplayName="Default GameMode"))
	FSoftClassPath GlobalDefaultGameMode;
```

关键两点：
1. 类型是 **`FSoftClassPath`**——一个"软类引用路径"，本质就是一串文本，指向某个 `UClass`。
2. `meta=(MetaClass="/Script/Engine.GameModeBase")`——限定这串路径只能指向 `AGameModeBase` 的子类（所以你能选的都是 GameMode 类）。

**所以 ini 里那串不是"文件在哪"的路径，而是"这是哪个类"的名字。** 引擎启动时读这串字符串，用它去加载对应的 `UClass`。

---

## 三、这串路径怎么拆（一一对应到你的类）

`/Script/Tenkaichi.TenkaichiGameMode` 拆成三段：

| 段 | 值 | 含义 | 对应你工程里的什么 |
|----|----|----|------------------|
| 前缀 | `/Script/` | "这是一个 **C++ 类**"（不是蓝图资产） | 你的类是纯 C++ 写的（`UCLASS()` 在 .h 里） |
| 模块名 | `Tenkaichi` | 类所在的**模块名** | `Source/Tenkaichi/` 目录 + `Tenkaichi.Build.cs` 里的模块名 |
| 类名 | `TenkaichiGameMode` | 类名**去掉 `A` 前缀** | `class ATenkaichiGameMode` → 去掉 `A` = `TenkaichiGameMode` |

拼起来：`/Script/` + `Tenkaichi`（模块）+ `.` + `TenkaichiGameMode`（类名去 A）= `/Script/Tenkaichi.TenkaichiGameMode`。

**对应关系一句话**：
> ini 里的 `Tenkaichi.TenkaichiGameMode` = 你 .h 里 `ATenkaichiGameMode` 的"模块名.类名（去 A）"。

---

## 四、为什么 C++ 类用 `/Script/`、蓝图用 `/Game/`（还带 `_C`）

这是 UE 的两套虚拟路径根：

| 类型 | 路径根 | 结尾 | 例子 |
|------|--------|------|------|
| **C++ 类** | `/Script/模块名` | 类名（**无 `_C`**） | `/Script/Tenkaichi.TenkaichiGameMode` |
| **蓝图资产** | `/Game/...` | 资产名 + `_C` | `/Game/B_LyraGameMode.B_LyraGameMode_C` |

**为什么蓝图带 `_C`**：蓝图是"资产"，UE 给蓝图生成的类对象名习惯加 `_C` 后缀（`B_LyraGameMode` 是资产名，`.B_LyraGameMode_C` 是它生成的类）。C++ 类没有这层，直接是类名。

**Lyra 真实证据**（它是蓝图 GameMode，所以带 `_C`）：

```66:66:e:\ue5\LyraStarterGame5.6\LyraStarterGame\Config\DefaultEngine.ini
GlobalDefaultGameMode=/Game/B_LyraGameMode.B_LyraGameMode_C
```

**我们和 Lyra 的差异**：
- Lyra 的 GameMode 是**蓝图** `B_LyraGameMode` → `/Game/B_LyraGameMode.B_LyraGameMode_C`
- 我们的 GameMode 是**纯 C++** `ATenkaichiGameMode` → `/Script/Tenkaichi.TenkaichiGameMode`

> 这就是为什么你照抄 Lyra 的写法会出错——它是蓝图带 `_C`，我们是 C++ 不带。**类型不同，路径格式就不同**（这正是 19 号铁律里那张对照表的由来）。

---

## 五、引擎怎么用这串字符串（可选了解）

引擎启动 → `UGameMapsSettings` 读 `GlobalDefaultGameMode` 这串 → 用 `LoadClass` / `StaticLoadClass` 把它解析成 `UClass*` → 得到 `ATenkaichiGameMode` 这个类 → 玩家加入时按这个类造 GameMode 实例。

> 源码入口：`GameMapsSettings.h` 第 121 行 `GetGlobalDefaultGameMode()`、第 149 行 `SetGlobalDefaultGameMode(const FString&)`——注意参数就是 `FString`，印证了"它存的是字符串路径"。

---

## 六、一句话结论

**`/Script/Tenkaichi.TenkaichiGameMode` 是"类引用路径"不是文件路径**：`/Script/`=C++ 类、`Tenkaichi`=模块名（`Source/Tenkaichi`）、`TenkaichiGameMode`=类名去掉 `A`（对应 `ATenkaichiGameMode`）。引擎靠这串字符串在运行时加载你的 `UCLASS`。C++ 类用 `/Script/模块.类名去A`（无 `_C`），蓝图用 `/Game/.../名.名_C`（有 `_C`）——Lyra 是蓝图所以带 `_C`，我们是 C++ 所以不带。
