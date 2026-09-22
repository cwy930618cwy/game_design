# `LyraExperienceDefinition.cpp` 速览

> 83 行，**几乎全是 `#if WITH_EDITOR` 里的** —— 打包后基本不做任何事。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `IsDataValid()`（编辑器） | ① 逐个校验 `Actions`，有 null 项就报 "Null entry at index N" ② ⭐ **禁止"蓝图的蓝图"继承**（往下看） |
| `UpdateAssetBundleData()`（编辑器） | 遍历 Actions，让每个 Action 通过 `AddAdditionalAssetBundleData` 把自己需要的资源写进 Bundle —— **这样加载 Experience 就知道要顺带加载哪些资源** |

## 那条禁止继承的校验说了什么

```
如果 Experience 是蓝图类，它的"第一个原生父类"必须就是它的直接父类。
也就是说：允许一层蓝图继承，不允许蓝图的蓝图再继承。
报错信息里明确建议 —— 想组合请用 ActionSets。
```

> 💡 这是 Lyra 明确在推的**组合优于继承**原则：想复用就拆 ActionSet，不要搞继承树。

**优先级**：`UpdateAssetBundleData`
