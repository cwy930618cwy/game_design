# `LyraInputConfig.cpp` 速览

> 49 行，三个函数，**两个 Find 几乎一模一样**。

| 函数 | 干嘛的 |
|---|---|
| 构造函数 | 空 |
| `FindNativeInputActionForTag()` | 线性遍历 `NativeInputActions`，找到就返回；**找不到按 `bLogNotFound` 打 Error** |
| `FindAbilityInputActionForTag()` | 同上，遍历 `AbilityInputActions` |

> 💡 两个函数都是 O(n) 线性查找。输入配置一般不会太大，所以没问题；但**同一个 Tag 配两次的话只会返回第一个**，这是个容易踩的小坑。

**优先级**：两个 Find 函数
