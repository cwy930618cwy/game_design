# 07 — `LyraEditor.cpp` 复制文件

> 依据：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.cpp`（**9 行 / 393 B，占位版**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.cpp`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。
> ⚠️ 这是 **S00 占位版**，不是 Lyra 正式版；S22 会用完整版（10470 B）覆盖。

---

## 完整代码

```cpp
// 引入模块管理头文件；IMPLEMENT_MODULE 宏与 FDefaultModuleImpl 都在这里
#include "Modules/ModuleManager.h"

// 注册普通模块；把实现类 FDefaultModuleImpl 与模块名 LyraEditor 绑定
IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);
```

> ⚠️ **你现有文件顶部有 5 行「说明：LyraEditor 占位模块的实现文件…」注释块**（第 1-5 行）。
> 按铁律第 4 条「零教学噪声」，**这一整块要删掉**。
> 删完你的文件 = 2 行代码 + 1 行空行，即上表 3 行。

---

## 本段新元素（每行一个，只列首次出现）

- `Modules/ModuleManager.h` —— 模块系统头文件；`IMPLEMENT_MODULE` 宏、`FDefaultModuleImpl` 类都在里面（`Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h`）
- `FDefaultModuleImpl` —— 引擎提供的「什么都不做」的默认模块实现类（`ModuleManager.h:760-761`）
- `IMPLEMENT_MODULE` —— 注册**普通模块**的宏；与 `IMPLEMENT_PRIMARY_GAME_MODULE`（主游戏模块，全工程只能 1 个）不同，每个普通模块各写一个
- `LyraEditor` —— 模块名；必须与 `LyraEditor.Build.cs` 的类名、`Source\LyraEditor\` 目录名三者一致

---

## 逐行讲解

### 第 1 行：`#include "Modules/ModuleManager.h"`

```cpp
// 引入模块管理头文件；IMPLEMENT_MODULE 宏与 FDefaultModuleImpl 都在这里
#include "Modules/ModuleManager.h"
```

**为什么路径不带工程前缀**：

| 写法 | 解析过程 |
|---|---|
| `#include "Modules/ModuleManager.h"` | ① 先看当前目录 `Source\LyraEditor\` → 没有 `Modules\`<br>② 再看依赖模块的 `Public\` 路径 → `LyraEditor.Build.cs` 依赖 `Core` → `Core\Public\` 被加入搜索路径 → 找到 `Core\Public\Modules\ModuleManager.h` ✅ |
| `#include "D:/ue5/.../ModuleManager.h"` | ❌ 绝对路径，换机器就挂，违反一比一 |
| `#include "../../Engine/..."` | ❌ 相对工程路径，同样违反一比一 |

> 📌 详细解释见 `答疑文件\Modules是哪来的.md`

### 第 2 行：`IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);`

```cpp
// 注册普通模块；把实现类 FDefaultModuleImpl 与模块名 LyraEditor 绑定
IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);
```

**两个参数分别是什么**：

| 参数位置 | 值 | 含义 |
|---|---|---|
| 第 1 个 | `FDefaultModuleImpl` | **实现类** —— 引擎提供的空实现（`StartupModule()` / `ShutdownModule()` 都是空函数） |
| 第 2 个 | `LyraEditor` | **模块名** —— 必须与 `Build.cs` 里的 `public class LyraEditor` 和 `Source\LyraEditor\` 目录名完全一致 |

**这个宏展开后干了什么**（简化）：

```cpp
// 大致等价于（实际更复杂，含版本兼容分支）
IModuleInterface* InitializeModule()
{
    return new FDefaultModuleImpl();
}
```

引擎加载模块时（`FModuleManager::LoadModule("LyraEditor")`）就调这个函数，拿到一个模块实例。

**为什么不能写 `IMPLEMENT_PRIMARY_GAME_MODULE`**：

| 宏 | 用在哪 | 一个工程能有几个 |
|---|---|---|
| `IMPLEMENT_PRIMARY_GAME_MODULE` | 主游戏模块（`LyraGameModule.cpp`） | **只能 1 个** |
| `IMPLEMENT_MODULE` | 普通模块（`LyraEditor.cpp`） | 每个模块 1 个，不限总数 |

> ⚠️ 这里写错成 `IMPLEMENT_PRIMARY_GAME_MODULE` → 与 `LyraGameModule.cpp` 重复定义主模块 → 链接报 `duplicate symbol`

---

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraEditor\LyraEditor.cpp`（在 `Source\LyraEditor\` 目录下） |
| include | `#include "Modules/ModuleManager.h"`（**只有这 1 个**） |
| 模块注册宏 | `IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);` |
| 宏名 | 是 `IMPLEMENT_MODULE`，**不是** `IMPLEMENT_PRIMARY_GAME_MODULE` |
| 两个参数顺序 | 第一个是类 `FDefaultModuleImpl`，第二个是模块名 `LyraEditor` |
| 分号 | 语句末尾有 `;` |
| 注释 | 所有 `//` 开头的行**全部删掉** |
| 代码行 | 剔注释后 **2 行**（`#include` + `IMPLEMENT_MODULE`） |

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Source\LyraEditor\LyraEditor.cpp:1-9`（占位版全文，393 B）
- `E:\ue5\LyraStarterGame\Source\LyraEditor\LyraEditor.cpp:1-329`（Lyra 正式版 10470 B，`FLyraEditorModule`，S22 才用）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h:760-761`（`FDefaultModuleImpl`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h:973 / 989 / 1007`（`IMPLEMENT_PRIMARY_GAME_MODULE` 三处分支定义）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h`（`IMPLEMENT_MODULE` 宏定义，与 `IMPLEMENT_PRIMARY_GAME_MODULE` 同文件不同段）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleInterface.h:31 / 55`（`StartupModule` / `ShutdownModule` 虚函数）