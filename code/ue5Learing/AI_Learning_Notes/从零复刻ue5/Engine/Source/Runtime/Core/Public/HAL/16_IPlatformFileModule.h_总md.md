# 16 — `HAL/IPlatformFileModule.h` —— 总 md

> 依据：`D:\ue5\Epic Games\UE_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileModule.h`（**23 行**）
> 你新建文件：`ue_5.6\Engine\Source\Runtime\Core\Public\HAL\IPlatformFileModule.h`
> **⚠️ 本节不能独立编译**——依赖 `IModuleInterface`。

---

## 一、这个文件解决什么问题（为什么）

UE 的模块系统允许**运行时加载 DLL**。有些平台文件实现（比如 `AndroidFile` / `IoStoreFile`）是独立模块，需要能被动态加载。`IPlatformFileModule.h` 就是这些模块的**统一接口**：模块加载后，引擎通过 `GetPlatformFile()` 拿到它提供的 `IPlatformFile` 实例。

**它只有一个类、一个方法**：

```cpp
class IPlatformFileModule : public IModuleInterface
{
public:
    virtual IPlatformFile* GetPlatformFile() = 0;
};
```

**说个场景**：

```cpp
// 引擎加载 AndroidFile 模块
IPlatformFileModule* Module = FModuleManager::LoadModulePtr<IPlatformFileModule>("AndroidFile");
if (Module)
{
    IPlatformFile* File = Module->GetPlatformFile();   // 拿到平台文件实例
    // 挂到文件系统链上
}
```

**为什么继承 `IModuleInterface`**：`IModuleInterface` 是所有 UE 模块的基类，提供 `StartupModule()` / `ShutdownModule()` 生命周期钩子。继承它之后，这个模块就能被 `FModuleManager` 正常加载/卸载。

> 🔑 **一句话**：`IPlatformFileModule.h` = **平台文件模块的「取货窗口」**，模块加载后从这里拿 `IPlatformFile` 实例。

---

## 二、内部结构全景

```
IPlatformFileModule.h（23 行）
├─ 第 1~7 行     版权头 + #pragma once + 3 个 include
├─ 第 9~11 行    类说明注释
├─ 第 12~14 行   ① class IPlatformFileModule : public IModuleInterface
├─ 第 15 行      公有区
├─ 第 17~21 行   ② GetPlatformFile() 说明注释
└─ 第 22~23 行   ③ virtual IPlatformFile* GetPlatformFile() = 0
```

---

## 三、依赖分析

| 文件 | 依赖工程内类？ | 能否独立编译 | 需挂起？ |
|------|--------------|-------------|---------|
| `HAL/IPlatformFileModule.h` | **是**（`IModuleInterface`） | ❌ 不能 | 否 |

**关键依赖**：

| 文件 | 作用 |
|---|---|
| `CoreTypes.h` | `TCHAR` 等基础类型 |
| `CoreFwd.h` | `IPlatformFile` 等前置声明 |
| `Modules/ModuleInterface.h` | **基类 `IModuleInterface`** |

**注意**：`IPlatformFile` 只通过 `CoreFwd.h` 前置声明，本文件只用到指针（照抄不改）。

---

## 四、怎么教（推进节奏）

**一轮三件套**（见经验 `08`）：总 md（本文件）+ `答疑文件\29_IPlatformFileModule.h逐行注释版.md` + 还原文件，同一轮产出并验收。

---

**下一步**：按 `HAL\` 文件名首字母顺序，下一个是 `IPlatformFileOpenLogWrapper.h`（9465 字节）。