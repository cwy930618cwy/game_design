# `GameFeatureAction_AddGameplayCuePath.cpp` 速览

> 39 行，**只有构造函数和编辑器校验**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | ⭐ 默认塞一个 `/GameplayCues` 目录（注释：这是最常用的约定路径） |
| `IsDataValid()`（编辑器） | 逐个检查路径是否为空 |

> 💡 **这个文件最大的信息量在于它"没写什么"** —— 没有 Activating / Deactivating。
> 真正把目录加进 `ULyraGameplayCueManager` 的是 `LyraGameFeaturePolicy.cpp` 中的 `ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering`。
>
> 所以排查"GameplayCue 找不到"时，要看的是 **Policy 那个观察者**，不是这里。
