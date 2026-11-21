# 05 — `LyraGameModule.cpp` 复制文件

> 依据：`E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp`（**20 行**）
> 你新建文件：`E:\ue5\game_design\code\Tenkaichi\Source\LyraGame\LyraGameModule.cpp`
> **用法**：照着下面代码逐行敲。**注释在每行上方，敲的时候整行 `//` 注释都不要抄**。
> ⚠️ 源文件第 6-8 行是块注释 `/** FLyraGameModule */`，按铁律**压成一行中文注释**，见下方。

---

```cpp
// 版权声明：Epic Games 保留所有权利

// 引入模块管理头文件；FDefaultGameModuleImpl 和主模块宏都在这里
#include "Modules/ModuleManager.h"


// 游戏主模块类；空实现即可，生命周期函数按需重写
class FLyraGameModule : public FDefaultGameModuleImpl
// 类体开始
{
	// 模块启动时由引擎调用；可在这里注册委托 / 初始化子系统
	virtual void StartupModule() override
	// 启动函数体开始
	{
	// 启动函数体结束（当前无需初始化）
	}

	// 模块关闭时由引擎调用；与启动成对，做清理
	virtual void ShutdownModule() override
	// 关闭函数体开始
	{
	// 关闭函数体结束（当前无需清理）
	}
// 类体结束
};

// 注册主游戏模块；把实现类 FLyraGameModule 与模块名 LyraGame 绑定
IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");
```

> 说明：源文件第 6-8 行的块注释已压成上面一行中文注释；每行代码上方的 `//` 注释敲的时候整行不抄。

---

**本段新元素**（每行一个，只列首次出现）：

- `FLyraGameModule` —— 游戏主模块类，本文件定义（`LyraGameModule.cpp:9`）
- `FDefaultGameModuleImpl` —— 引擎提供的默认游戏模块基类（`ModuleManager.h:767`）
- `StartupModule()` —— 模块加载时的生命周期回调（`ModuleInterface.h:31`）
- `ShutdownModule()` —— 模块卸载时的生命周期回调（`ModuleInterface.h:55`）
- `IMPLEMENT_PRIMARY_GAME_MODULE` —— 注册主游戏模块的宏（`ModuleManager.h:973`）

## 敲完自查

| 检查项 | 通过标准 |
|---|---|
| 文件路径 | `Source\LyraGame\LyraGameModule.cpp` |
| include | `#include "Modules/ModuleManager.h"` |
| 类 | `class FLyraGameModule : public FDefaultGameModuleImpl` |
| 两个函数 | `virtual void StartupModule() override` / `virtual void ShutdownModule() override`，**空实现** |
| 主模块宏 | `IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");` |
| 注释 | 全部删掉 `//` 开头的行；块注释已压成一行，不逐行保留 |
| 代码行 | 剔注释后 **11 行**，与源逐字一致 |

## 源码依据

- `E:\ue5\game_design\code\LyraStarterGame\Source\LyraGame\LyraGameModule.cpp:1-20`（源全文）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h:767`（`FDefaultGameModuleImpl`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleManager.h:973 / 1007`（`IMPLEMENT_PRIMARY_GAME_MODULE` 宏）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Core\Public\Modules\ModuleInterface.h:31 / 55`（两个生命周期虚函数）
