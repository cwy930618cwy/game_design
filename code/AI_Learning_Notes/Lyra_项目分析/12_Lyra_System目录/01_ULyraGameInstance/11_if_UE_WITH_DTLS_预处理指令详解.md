# `#if UE_WITH_DTLS` 什么时候生效？—— 预处理指令详解

> **定位**：解释 `LyraGameInstance.cpp` 第 32 行的 `#if UE_WITH_DTLS`（及第 48 行嵌套的 `#if !UE_BUILD_SHIPPING`）。
>
> **原代码**：
> ```cpp
> namespace Lyra
> {
>     static bool bTestEncryption = false;           // 总是编译
>     static FAutoConsoleVariableRef CVarLyraTestEncryption(...);
>
> #if UE_WITH_DTLS                                // ← 本文主角
>     static bool bUseDTLSEncryption = false;
>     ...（DTLS 相关 CVar）...
>
>     #if !UE_BUILD_SHIPPING                      // ← 嵌套的预处理
>         static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(...);
>     #endif // UE_BUILD_SHIPPING
> #endif // UE_WITH_DTLS
> };
> ```
>
> **一句话**：`#if UE_WITH_DTLS` 是**预处理指令**，在**编译之前**就决定"这段代码要不要"——如果 `UE_WITH_DTLS` 这个宏**被定义了**，中间代码就保留；**没定义**，中间代码被**整段删掉**（就像从没写过）。它跟 namespace 完全不同：namespace 是编译期结构，`#if` 是**预处理期**的"代码开关"。

---

## 一、先破题：你的疑问

| 疑问 | 答案 |
|------|------|
| `#if` 什么时候生效？ | **编译之前**（预处理阶段），比 namespace 更早 |
| `UE_WITH_DTLS` 是啥？ | 一个**宏**（编译开关），定义了=1，没定义=0 |
| 跟 namespace 啥关系？ | 完全不同层次——`#if` 管"这段代码存不存在"，namespace 管"代码怎么归类" |

> **核心**：`#if` 是"**编译期的代码开关**"，决定某段代码**要不要参与编译**。被 `#if` 排除的代码，编译器**根本看不到**，就像被橡皮擦掉了。

---

## 二、`#if` 是什么？—— 预处理指令

### 2.1 编译的两个阶段

C++ 编译分两大步：

```
【第 1 步：预处理（Preprocess）】  ← #if 在这里生效！
  处理所有以 # 开头的指令（#if、#include、#define）
  → 展开宏、决定哪些代码保留/删除
  → 产出"干净的"C++ 代码

【第 2 步：编译（Compile）】  ← namespace 在这里处理
  把"干净的"C++ 代码翻译成机器码
  → 处理 namespace、class、函数等
```

> **关键**：`#if` 在**第 1 步（预处理）**就处理完了，比 namespace（第 2 步）更早。预处理阶段，`#if` 决定"哪些代码留下"，留下的代码才轮到 namespace 去归类。

### 2.2 `#if` 的语法

```cpp
#if 条件          // 条件为真 → 保留中间代码
    // 这些代码只在条件为真时存在
#endif            // 结束
```

- `#` 开头 = 预处理指令（编译前处理）。
- `#if` 后面跟一个**表达式**，非 0 为真，0 为假。
- 必须配 `#endif` 结束。

### 2.3 常见配套指令

| 指令 | 作用 |
|------|------|
| `#if` | 条件为真才保留 |
| `#ifdef` | 宏**被定义**才保留 |
| `#ifndef` | 宏**没被定义**才保留 |
| `#else` | 否则（另一种情况） |
| `#elif` | 否则如果 |
| `#endif` | 结束 |
| `#define` | 定义宏 |

---

## 三、`UE_WITH_DTLS` 是什么？—— 一个编译开关宏

### 3.1 它是宏，值是 0 或 1

`UE_WITH_DTLS` 是 UE 定义的一个**宏**（编译期常量），表示"这个构建**是否启用 DTLS 加密**"：

```cpp
// 在某个 UE 头文件里（简化示意）
#define UE_WITH_DTLS 1    // 启用了 DTLS → 值为 1
// 或
#define UE_WITH_DTLS 0    // 没启用 DTLS → 值为 0
```

> DTLS = Datagram TLS，一种针对 UDP 的加密协议（对应 TCP 的 TLS）。UE 用它给网络包加密。

### 3.2 它怎么决定代码去留？

```cpp
#if UE_WITH_DTLS          // 如果 UE_WITH_DTLS == 1
    // DTLS 相关的 CVar、命令
    static bool bUseDTLSEncryption = false;
    ...
#endif
```

**两种情况**：

**情况 A：启用了 DTLS（`UE_WITH_DTLS` = 1）**
→ 预处理后，中间代码**保留**：
```cpp
namespace Lyra
{
    static bool bTestEncryption = false;
    static FAutoConsoleVariableRef CVarLyraTestEncryption(...);

    static bool bUseDTLSEncryption = false;        // ← 保留了
    static FAutoConsoleVariableRef CVarLyraUseDTLSEncryption(...);
    ...
};
```

**情况 B：没启用 DTLS（`UE_WITH_DTLS` = 0）**
→ 预处理后，中间代码**被删掉**：
```cpp
namespace Lyra
{
    static bool bTestEncryption = false;
    static FAutoConsoleVariableRef CVarLyraTestEncryption(...);

    // ← DTLS 那段整个消失了，就像从没写过
};
```

> **重点**：情况 B 里，`bUseDTLSEncryption` 这些变量**根本不存在**于编译后的程序里——它们被预处理阶段"擦除"了。这就是为什么叫"编译开关"。

---

## 四、嵌套的 `#if !UE_BUILD_SHIPPING`（第 48 行）

代码里还有一层嵌套：

```cpp
#if UE_WITH_DTLS                    // 外层：启用了 DTLS？
    ...
    #if !UE_BUILD_SHIPPING          // 内层：不是 Shipping 发布版？
        static FAutoConsoleCommandWithWorldAndArgs CmdGenerateDTLSCertificate(...);
    #endif // UE_BUILD_SHIPPING
#endif // UE_WITH_DTLS
```

### 4.1 `UE_BUILD_SHIPPING` 是什么？

UE 的**构建配置**宏，表示"这是不是正式发布版（Shipping）"：

| 构建配置 | `UE_BUILD_SHIPPING` | 说明 |
|---------|---------------------|------|
| Debug / Development | 0 | 开发期，带调试工具 |
| **Shipping** | **1** | 正式发布，删掉所有调试/测试代码 |

### 4.2 `!UE_BUILD_SHIPPING` 的意思

```cpp
#if !UE_BUILD_SHIPPING    // 如果不是 Shipping 版（即开发/调试版）
    // 这个生成证书的调试命令才存在
#endif
```

- `!` = 非（取反）。
- `!UE_BUILD_SHIPPING` = "不是 Shipping 版"。
- 意思：**只有开发/调试版才编译这个生成证书的命令行工具**，正式发布版（Shipping）里删掉——因为正式玩家不需要生成测试证书。

### 4.3 嵌套的真值表

| 场景 | `UE_WITH_DTLS` | `UE_BUILD_SHIPPING` | 生成证书命令存在吗？ |
|------|----------------|---------------------|---------------------|
| 开发版 + 启用 DTLS | 1 | 0 | ✅ 存在 |
| 开发版 + 没启用 DTLS | 0 | 0 | ❌ 不存在（外层就删了） |
| 发布版 + 启用 DTLS | 1 | 1 | ❌ 不存在（内层删了） |
| 发布版 + 没启用 DTLS | 0 | 1 | ❌ 不存在（外层就删了） |

> 只有"**开发版 + 启用 DTLS**"时，那个生成证书的调试命令才会出现在最终程序里。

---

## 五、`#if` vs `namespace` vs `if`（三者对比）★

你连续问了 namespace 和 `#if`，再加个运行时 `if`，三者彻底分清：

| | `#if`（预处理） | `namespace`（编译期结构） | `if`（运行时语句） |
|---|---|---|---|
| **阶段** | 预处理期（最早） | 编译期 | 运行时 |
| **本质** | 代码开关（决定代码存不存在） | 命名空间（代码归类） | 条件判断（程序跑时决定走哪条路） |
| **何时处理** | 编译之前 | 编译时 | 程序运行时 |
| **运行时存在吗** | 处理后消失 | 运行时消失 | 运行时真实执行 |
| **例子** | `#if UE_WITH_DTLS ... #endif` | `namespace Lyra { ... }` | `if (bTestEncryption) { ... }` |
| **JS 对照** | 没有真等价（构建工具如 webpack define） | 对象命名前缀 | `if (...) {}` |

### 时间线（三者谁先谁后）

```
【预处理期】  #if UE_WITH_DTLS 生效
  → 决定 DTLS 那段代码要不要留下
  → 被排除的代码"蒸发"

【编译期】  namespace Lyra 生效
  → 把留下的代码归类到 Lyra 空间
  → 处理符号、作用域

【程序启动】  static 变量初始化
  → bTestEncryption = false 等被赋值

【运行时】  if (bTestEncryption) 生效
  → 程序跑起来，根据变量值决定走哪条分支
```

> **一句话**：`#if` 决定"代码在不在"（最早，预处理）；namespace 决定"代码归哪类"（编译期）；`if` 决定"运行时走哪条路"（最晚，运行时）。三者层层递进。

---

## 六、为什么用 `#if` 而不是 `if`？

既然 `if (UE_WITH_DTLS)` 也能判断，为什么非要用 `#if`？

### 6.1 `#if`（预处理）：代码根本不编译

```cpp
#if UE_WITH_DTLS
    FDTLSCertificate 相关代码    // 没启用 DTLS 时，这段"不存在"
#endif
```
- 没启用 DTLS 时，这段代码**被删掉**，编译器看不到，**不会产生任何机器码**。
- 适合"整个功能模块开关"。

### 6.2 `if`（运行时）：代码编译了，只是不执行

```cpp
if (UE_WITH_DTLS)    // 假设这是个运行时变量
    FDTLSCertificate 相关代码    // 永远会编译进程序，只是运行时可能跳过
```
- 代码**始终被编译**进程序（占体积），运行时才判断。
- 无法"删除"代码。

### 6.3 对比

| | `#if`（预处理） | `if`（运行时） |
|---|---|---|
| 代码被删掉吗 | ✅ 是，彻底消失 | ❌ 否，始终编译进去 |
| 占最终体积吗 | 不占（被删的） | 占 |
| 适合场景 | 整个功能开关（如 DTLS） | 运行时逻辑分支 |

> **结论**：DTLS 是"整个功能模块"，不启用时希望它**彻底消失**（省体积、避免依赖缺失报错），所以用 `#if` 而不是 `if`。

---

## 七、JS 对照（帮你理解）

JS 没有真正的预处理 `#if`，但构建工具（如 webpack 的 `DefinePlugin`）能模拟：

```js
// 构建时定义一个常量
// webpack DefinePlugin: { 'UE_WITH_DTLS': 'true' }

if (UE_WITH_DTLS) {
    // 如果构建工具把它替换成 true，且开启 tree-shaking
    // 这段才可能被保留；否则可能被摇掉
}
```

```cpp
// C++ 的 #if 是真·预处理
#if UE_WITH_DTLS
    // 没定义就直接不存在，比 JS 的构建替换更彻底、更早
#endif
```

| 概念 | JS（构建工具模拟） | C++（`#if`） |
|------|-------------------|--------------|
| 处理时机 | 构建/打包时 | 预处理期（编译前） |
| 本质 | 字符串替换 + tree-shaking | 语言内置预处理指令 |
| 彻底程度 | 依赖打包器优化 | 编译器强制删除 |

> **关键差异**：C++ 的 `#if` 是**语言内置**的预处理，编译器保证执行；JS 靠构建工具"模拟"，效果类似但机制不同。

---

## 八、常见疑问速答

| 疑问 | 答案 |
|------|------|
| `#if` 什么时候生效？ | 编译之前（预处理阶段），最早 |
| `UE_WITH_DTLS` 是变量吗？ | 不是，是宏（编译期常量），值 0 或 1 |
| 没启用 DTLS 时，那段代码去哪了？ | 被预处理删掉了，编译器根本看不到 |
| `#if` 和 `if` 区别？ | `#if` 删代码（预处理），`if` 只是运行时跳过（代码还在） |
| `!UE_BUILD_SHIPPING` 啥意思？ | "不是正式发布版"，即开发/调试版 |
| 为什么嵌套两层 `#if`？ | 外层管"有无 DTLS"，内层管"有无调试工具"，组合控制 |
| `#if` 处理后还占体积吗？ | 被删的代码不占，留下的才占 |
| `#endif` 必须写吗？ | 必须，和 `#if` 配对 |
| 预处理和编译啥区别？ | 预处理先（处理 `#` 指令），编译后（翻译成机器码） |

---

## 九、总结

```
#if UE_WITH_DTLS = 预处理指令（编译前的"代码开关"）

什么时候生效：
  ★ 编译之前（预处理阶段），比 namespace 更早
  决定中间代码"要不要参与编译"

UE_WITH_DTLS 是啥：
  一个宏（编译期常量），1=启用DTLS，0=没启用
  没启用时，中间 DTLS 代码被整段删掉（像从没写过）

嵌套的 #if !UE_BUILD_SHIPPING：
  只有"非发布版（开发/调试版）"才编译生成证书的调试命令
  正式发布版（Shipping）删掉这些测试工具

#if vs namespace vs if：
  #if       → 预处理期，决定"代码在不在"（最早）
  namespace → 编译期，决定"代码归哪类"
  if        → 运行时，决定"走哪条路"（最晚）

为什么用 #if 不用 if：
  #if 彻底删代码（省体积、避免依赖报错）
  if 只是运行时跳过（代码还在，占体积）
```

**一句话**：`#if UE_WITH_DTLS` 是**预处理指令**，在**编译之前**（比 namespace 更早）就决定中间那段 DTLS 代码**要不要参与编译**——`UE_WITH_DTLS` 是个宏开关，没启用时那段代码被**整段删掉**（编译器根本看不到，像从没写过）；嵌套的 `#if !UE_BUILD_SHIPPING` 进一步限制"生成证书的调试命令"只在**开发版**存在，正式发布版删掉。它和 namespace（编译期归类）、运行时 `if`（运行时分支）是三个完全不同层次的机制。

---

## 十、下一步

- 找 `UE_WITH_DTLS` 在哪个头文件定义、由什么构建配置决定。
- 看 UE 其他常见 `#if` 宏：`WITH_EDITOR`、`UE_BUILD_DEBUG`、`PLATFORM_WINDOWS` 等。
- 理解预处理 `#include`（头文件包含也是预处理指令）。
- 回顾 DTLS 加密在 Lyra 网络里的完整流程（见 `05`~`09` 篇）。
