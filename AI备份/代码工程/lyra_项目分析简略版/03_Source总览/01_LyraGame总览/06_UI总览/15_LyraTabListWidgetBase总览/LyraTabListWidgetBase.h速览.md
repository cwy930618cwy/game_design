# `LyraTabListWidgetBase.h` 速览

> Tab 列表。设置界面的分类页就是它。**比引擎原生的 TabList 多了"预注册"和"延迟创建"**。

| 成员 | 干嘛的 |
|---|---|
| `FLyraTabDescriptor` | ⭐ 一个 Tab 的完整描述：Id / 文字 / 图标 / 是否隐藏 / 按钮类 / 内容类 |
| `ULyraTabButtonInterface` / `ILyraTabButtonInterface` | Tab 按钮要实现的接口（`SetTabLabelInfo`） |
| `: UCommonTabListWidgetBase` | 标准继承 |
| `GetPreregisteredTabInfo()` / `GetAllPreregisteredTabInfos()` | 查预注册信息 |
| `SetTabHiddenState()` | ⭐ 隐藏某个 Tab（**只能在关联 Switcher 之前调**） |
| `RegisterDynamicTab()` | 运行时动态注册一个 Tab |
| `IsFirstTabActive()` / `IsLastTabActive()` | 判断首尾（用于手柄的"左右循环"） |
| `IsTabVisible()` / `GetVisibleTabCount()` | 可见性查询 |
| `OnTabContentCreated` / `OnTabContentCreatedNative` | ⭐ Tab 内容创建后的回调（蓝图版 + C++ 版） |
| `SetupTabs()`（private） | ⭐ 真正创建内容并注册 |
| `PreregisteredTabInfoArray` | 配置好的 Tab 列表 |
| `PendingTabLabelInfoMap` | 运行时注册但还没创建的 Tab |

> ⚠️ 源码里留了个 TODO：`TabContentType` 应该改成 `TSoftClassPtr`，因为底层 CommonTabList 还不支持延迟构造内容。

**优先级**：`SetupTabs` → `RegisterDynamicTab`
