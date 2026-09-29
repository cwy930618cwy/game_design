# `LyraGameSettingRegistry.h` 速览

> 设置页的**组装图纸**：把各个设置项拼成一棵树，交给设置界面显示。

| 成员 | 干嘛的 |
|---|---|
| `: UGameSettingRegistry` | 来自 GameSettings 插件 |
| `GET_SHARED_SETTINGS_FUNCTION_PATH(属性名)` | ⭐ 宏：生成一条"读/写 Shared 设置某字段"的动态数据源 |
| `GET_LOCAL_SETTINGS_FUNCTION_PATH(属性名)` | ⭐ 同上，针对 Local 设置 |
| `Get()` | 按 LocalPlayer 取（没有就新建并 Initialize） |
| `SaveChanges()` | 保存 |
| `OnInitialize()` | ⭐ 依次组装 5 大类并 RegisterSetting |
| `IsFinishedInitializing()` | ⭐ **Shared 设置还没加载完就返回 false** |
| 5 个 `InitializeXXXSettings()` | 分别对应 5 个分类的 cpp 文件 |
| `InitializeVideoSettings_FrameRates()` | 帧率相关的四个选项 |
| `AddPerformanceStatPage()` | 性能统计子页 |
| 5 个 `UGameSettingCollection*` 成员 | Video / Audio / Gameplay / MouseAndKeyboard / Gamepad |

> 💡 **那两个宏是理解全目录的钥匙**：它们展开后是 `FGameSettingDataSourceDynamic`，用**字符串路径**（`GetLocalSettings` → `GetXXX`）在运行时反射调用函数 —— 所以设置项不用硬引用具体字段。

**优先级**：两个宏 → `OnInitialize`
