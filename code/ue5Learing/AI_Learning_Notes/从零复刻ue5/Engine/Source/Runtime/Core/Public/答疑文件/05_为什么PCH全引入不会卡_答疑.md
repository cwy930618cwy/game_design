# 05 — 为什么 `CoreSharedPCH.h` 一次 include 252 个头不会卡？（PCH 答疑）

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`（263 行 / 252 include）
> 用户问题：「一次性引入这么多 不会卡吗？我理解应该按需引入呀」
> **✅ 本节可独立编译**——纯概念答疑。

---

## 一、你的理解对了一半

**「按需引入」是对的** —— 那是**普通头文件**的规则（IWYU：Include What You Use）。

**但 `CoreSharedPCH.h` 不是普通头文件** —— 它是**预编译头（PCH）**，规则完全相反：**故意一次全引入，然后编译成二进制快照**。

| 项 | 普通头文件（如 `CoreMinimal.h`） | 预编译头（`CoreSharedPCH.h`） |
|---|---|---|
| 规则 | **按需引入**（IWYU） | **故意全引入** |
| 谁 include 它 | 你的 `.h` / `.cpp` | **编译器自动**（`/FI` 强制包含） |
| 解析次数 | 每个 `.cpp` 各解析一次 | **只解析一次**，存成 `.pch` |
| 目的 | 让代码能编过 | **让编译变快** |

> 🔑 **一句话**：**普通头是「按需点菜」，PCH 是「提前把整本菜单做成预制菜」——点菜慢，预制菜快。**

---

## 二、为什么「一次全引入」反而更快（核心）

### 2.1 不用 PCH 的情况

```
685 个 .cpp，每个都 #include "CoreMinimal.h"（148 个头）
  ↓
编译器对每个 .cpp 都要：
  ① 打开 148 个头文件
  ② 逐行解析（宏展开 / 模板实例化 / 语法分析）
  ③ 生成 AST（抽象语法树）
  ↓
重复 685 次 → 685 × 148 = 101,380 次头文件解析
  ↓
编译时间：几小时
```

### 2.2 用 PCH 的情况

```
第 1 步：编译 CoreSharedPCH.h（252 个头）
  ① 打开 252 个头文件
  ② 逐行解析
  ③ 生成 AST
  ④ **把 AST 序列化成二进制 .pch 文件**（几百 MB）
  ↓ 只做 1 次

第 2 步：编译 685 个 .cpp
  ① 编译器读 .pch 文件（**直接反序列化 AST，跳过解析**）
  ② 只解析 .cpp 自己的代码
  ↓
重复 685 次，但每次只解析「.cpp 自己那几十行」
  ↓
编译时间：几分钟
```

### 2.3 关键：`.pch` 里存的是「解析结果」，不是「源码」

| 项 | 内容 |
|---|---|
| `.h` 文件 | **源码**（文本，要解析） |
| `.pch` 文件 | **AST 的二进制序列化**（已解析完，直接读） |

> 类比：
> - 不用 PCH = 每次做菜都从「洗菜切菜」开始
> - 用 PCH = 提前把菜洗好切好放冰箱（`.pch`），做菜时直接下锅

---

## 三、编译器怎么「自动」用 PCH（源码证据）

### 3.1 创建 PCH（`/Yc`）

```csharp
// VCToolChain.cs:301~302
Arguments.Add($"/Yc\"{PchThroughHeaderFilePath}\"");
Arguments.Add($"/Fp\"{CreatePchFilePath}\"");
```

| 参数 | 含义 |
|---|---|
| `/Yc"CoreSharedPCH.h"` | **创建** PCH，以这个头为「截止点」 |
| `/Fp"xxx.pch"` | PCH 输出到哪个文件 |

### 3.2 使用 PCH（`/Yu`）

```csharp
// VCToolChain.cs:318~319
Arguments.Add($"/Yu\"{PchThroughHeaderFilePath}\"");
Arguments.Add($"/Fp\"{UsingPchFilePath}\"");
```

| 参数 | 含义 |
|---|---|
| `/Yu"CoreSharedPCH.h"` | **使用** PCH，跳过这个头之前的所有解析 |
| `/Fp"xxx.pch"` | 从哪个文件读 PCH |

### 3.3 强制包含（`/FI`）

```csharp
// VCToolChain.cs:285
Arguments.Add($"/FI\"{ForceIncludeFileString}\"");
```

| 参数 | 含义 |
|---|---|
| `/FI"CoreSharedPCH.h"` | **强制**在每个 `.cpp` 最前面 include 这个头（不用你手写） |

> 🔑 **关键**：`/FI` 让编译器**自动**在每个 `.cpp` 开头插入 `#include "CoreSharedPCH.h"`，你根本不用写。

### 3.4 完整流程

```
① UBT 编译 CoreSharedPCH.h → 生成 CoreSharedPCH.pch（几百 MB）
        ↓
② UBT 编译每个 .cpp 时，命令行加：
   cl.exe /Yu"CoreSharedPCH.h" /Fp"CoreSharedPCH.pch" /FI"CoreSharedPCH.h" MyFile.cpp
        ↓
③ 编译器：
   - 读 .pch（反序列化 AST，跳过 252 个头的解析）
   - 只解析 MyFile.cpp 自己的代码
        ↓
④ 输出 MyFile.obj
```

---

## 四、那「按需引入」什么时候用？

| 场景 | 用什么 | 为什么 |
|---|---|---|
| **写普通 `.h`**（如 `MyActor.h`） | `#include "CoreMinimal.h"` + 按需 include 其他 | 遵循 IWYU，减少耦合 |
| **写 `.cpp`** | 只 include 自己需要的头 | 遵循 IWYU |
| **PCH 头**（`CoreSharedPCH.h`） | **故意全引入** | 预编译一次，全项目复用 |
| **模块的 `Public/` 头** | 按需 include | 对外接口要干净 |

**UE 的 IWYU 规则**（`ModuleRules.cs:70~98` 的 `IWYUSupport` 枚举）：

| 值 | 含义 |
|---|---|
| `None` | 代码甚至编不过，IWYU 完全跳过 |
| `KeepAsIs` | 能改但保持原样（**Core 用的就是这个**） |
| `KeepAsIsForNow` | 解析处理但文件不改 |
| `KeepPublicAsIsForNow` | 同上，但允许改私有头和 cpp |
| `Full` | 完整 IWYU 支持 |

`Core.Build.cs` 第 304 行：`IWYUSupport = IWYUSupport.KeepAsIs;` —— Core 保持原样，不强制 IWYU。

---

## 五、为什么「全引入」不会让最终程序变大

**关键**：`#include` 只是「文本粘贴」，**不产生任何代码**。

| 项 | 会不会进最终二进制 |
|---|---|
| `#include "Math/Vector.h"` | ❌ 不会（只是让编译器知道 `FVector` 长什么样） |
| 实际用到 `FVector` 的代码 | ✅ 会（生成机器码） |
| 没用到 `FVector` | ❌ 不会（编译器优化掉） |

**举例**：

```cpp
#include "CoreSharedPCH.h"   // 252 个头全进来

void MyFunc()
{
    int32 X = 1;   // 只用了 int32
}
```

编译后：`MyFunc` 里只有 `int32 X = 1` 的机器码，**252 个头一个字节都不进二进制**。

> 类比：`#include` 是「把字典搬到你桌上」——你只查了一个词，字典本身不会变成你的文章。

---

## 六、那「卡」在哪？（真实代价）

| 代价 | 说明 |
|---|---|
| **磁盘** | `.pch` 文件几百 MB（Core 的约 300~500 MB） |
| **首次编译** | 要先编 PCH（几分钟），之后才快 |
| **改 PCH 头** | 改 `CoreSharedPCH.h` 里任何一行 → **整个 PCH 重编 + 所有 `.cpp` 重编**（几小时） |
| **内存** | 编译时编译器要装下整个 AST（几 GB） |

**所以 UE 的规则是**：

| 规则 | 原因 |
|---|---|
| PCH 头**很少改** | 改了要全项目重编 |
| PCH 头**只放稳定头** | 不稳定的头放进去会频繁触发重编 |
| 游戏模块**默认不用共享 PCH** | 游戏代码改动频繁，用 PCH 反而慢（`PCHUsageMode.Default` 注释：*Engine modules use shared PCHs, game modules do not*） |

---

## 七、`CoreSharedPCH.h` vs `CoreMinimal.h` 到底谁快

| 场景 | 用 `CoreMinimal.h`（148 头） | 用 `CoreSharedPCH.h`（252 头） |
|---|---|---|
| 写普通 `.h` | ✅ 快（少解析 104 个头） | ❌ 慢（多解析 104 个头） |
| 685 个 `.cpp` 编译 | ❌ 慢（每个都解析 148 个头） | ✅ 快（PCH 只解析 1 次） |

**结论**：
- **写代码时** → 用 `CoreMinimal.h`（按需）
- **编译加速时** → 用 `CoreSharedPCH.h`（PCH）

> 这就是为什么 UE 两个都有：`CoreMinimal.h` 给**人**用，`CoreSharedPCH.h` 给**编译器**用。

---

## 八、速查表 · 口诀

| 问题 | 答案 |
|---|---|
| 252 个头全引入不会卡吗？ | **不会**，因为 PCH 只解析 1 次，存成 `.pch` 二进制快照 |
| 应该按需引入吗？ | **普通头要**，**PCH 头不要**（故意全引入） |
| 最终程序会变大吗？ | **不会**，`#include` 不产生代码 |
| 代价是什么？ | 磁盘（几百 MB）+ 改 PCH 头要全项目重编 |
| 编译器怎么自动用？ | `/Yc` 创建 + `/Yu` 使用 + `/FI` 强制包含 |

**口诀**：**普通头按需点菜（IWYU），PCH 头提前做预制菜（`/Yc` 做一次、`/Yu` 吃 685 次）；`#include` 只是文本粘贴，不进二进制；代价是磁盘和「改 PCH 要全重编」。**

---

## 九、一句话总结

> **`CoreSharedPCH.h` 一次 include 252 个头，不是「每次都解析 252 个头」，而是「解析一次存成 `.pch`，之后 685 个 `.cpp` 直接读快照」。所以不但不卡，反而比按需引入快几十倍。**
>
> **「按需引入」是给普通头文件的规则；PCH 头的规则正好相反——故意全引入，才能一次编译、全项目复用。**

---

## 待确认

1. 这个解释清楚了吗？还有哪里不明白告诉我。
2. 看完说「下一步」，我教 `Core.h`（319 行）。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\CoreSharedPCH.h`（263 行 / 252 include）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（330 行，第 15 行 `SharedPCHHeaderFile`、第 304 行 `IWYUSupport`）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Configuration\ModuleRules.cs`（`IWYUSupport` 70~98、`PCHUsageMode` 193~219、`PrivatePCHHeaderFile` 793、`SharedPCHHeaderFile` 799）
- `D:\ue5\Epic Games\UE_5.6\Engine\Source\Programs\UnrealBuildTool\Platform\Windows\VCToolChain.cs`（`/FI` 285、`/Yc` 301、`/Fp` 302、`/Yu` 318）