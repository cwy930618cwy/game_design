# 线A-2 TenkaichiGameEngine —— 教 `.cpp`

> **定位**：阶段三线 A-2 的第 3 步——还原 `TenkaichiGameEngine.cpp`（**.cpp 实现部分**）。
> 源码依据：`E:\ue5\LyraStarterGame5.6\LyraStarterGame\Source\LyraGame\System\LyraGameEngine.cpp`（19 行）
> 命名：`ULyraGameEngine` → **`UTenkaichiGameEngine`**；文件 `LyraGameEngine.cpp` → **`TenkaichiGameEngine.cpp`**（只换前缀）。
> 路径：放到你工程的 `Source\TenkaichiGame\System\TenkaichiGameEngine.cpp`。

---

## 一、`.cpp` 要实现哪些东西（对着 `.h` 的声明）

`.h` 里声明了、需要 `.cpp` 实现的，成对核对（铁律 11）：

| `.h` 声明 | `.cpp` 实现 |
|-----------|------------|
| `UTenkaichiGameEngine(...)` 构造函数 | ✅ |
| `UTenkaichiGameEngine::Init(IEngineLoop*)` | ✅ |

两个都要实现。这个类很精简，`.cpp` 也短。

---

## 二、整份 `.cpp` 一比一还原（照着写）

> **注释中文化**（铁律 21/28）：注释已翻成中文；代码本体（类名/类型/宏）与 Lyra 原样一字不差（只换前缀）。

```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h
#include "TenkaichiGameEngine.h"

// UHT 生成的实现（对应 TenkaichiGameEngine.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameEngine)

class IEngineLoop;


// 构造函数：调用父类初始化
UTenkaichiGameEngine::UTenkaichiGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 引擎初始化钩子：调用父类的 Init，暂时不额外加逻辑（留空壳，为将来插自定义启动代码）
void UTenkaichiGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}
```

---

## 三、逐段讲解（每一段是干嘛的）

### 段 1：头文件包含（1~7 行）
```cpp
#include "TenkaichiGameEngine.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiGameEngine)
class IEngineLoop;
```
- 第 1 行：自己的 `.h`。
- 第 4 行：UHT 生成的 `.cpp` 实现（宏 `UE_INLINE_GENERATED_CPP_BY_NAME`）。
- 第 6 行：`IEngineLoop` 前置声明（`.cpp` 里只用它的指针做参数，不展开）。

### 段 2：构造函数（10~13 行）
```cpp
UTenkaichiGameEngine::UTenkaichiGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
```
- `: Super(ObjectInitializer)`：把初始化器传给父类 `UGameEngine` 的构造函数。
- 函数体空——目前不需要额外初始化。

### 段 3：`Init`（15~18 行）
```cpp
void UTenkaichiGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}
```
- 重写 `UGameEngine::Init`，目前只调用父类实现（`Super::Init`），函数体里没加自定义逻辑。
- 这是"**留钩子**"——以后想在引擎启动时做自定义事，就在 `Super::Init(InEngineLoop);` 前后加代码。

---

## 四、写完后自查（铁律 11/12 两个核对）

1. **缺行核对**：构造函数 + `Init()` 两个实现一个不能少。
2. **声明成对**：`.h` 里声明的 2 个（构造函数 + `Init`），`.cpp` 全部实现。
3. **命名核对**：`UTenkaichiGameEngine`、`TenkaichiGameEngine.cpp`（前缀已换）。

---

## 五、整个类回顾（`.h` + `.cpp` + 配置）

把三部分合起来，`TenkaichiGameEngine` 完整闭环是：
1. **`.h`**：声明 `UTenkaichiGameEngine : public UGameEngine`，重写构造函数 + `Init`。
2. **`.cpp`**：实现两个函数，目前都是调 `Super`，是空壳。
3. **`DefaultEngine.ini`**：`[/Script/Engine.Engine]` 下的 `GameEngine=/Script/TenkaichiGame.TenkaichiGameEngine`，告诉 UE 用这个类当引擎。

三者缺一不可——光有类没配置，引擎不会用它；光有配置没类，编译报错。

---

## 六、确认点

按铁律 29，你写完 `.cpp` 后说"下一步"，**我会先读你工程里的 `TenkaichiGameEngine.cpp` 确认写完、内容对得上**，再进线 A 第 3 项（`TenkaichiGameData`，走同样的"总 md → h → cpp"）。