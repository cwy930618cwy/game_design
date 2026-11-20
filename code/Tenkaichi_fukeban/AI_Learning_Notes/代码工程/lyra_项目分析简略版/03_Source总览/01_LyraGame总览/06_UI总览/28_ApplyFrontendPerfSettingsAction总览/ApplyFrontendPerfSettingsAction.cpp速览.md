# `ApplyFrontendPerfSettingsAction.cpp` 速览

> 43 行，两个函数 + 一个静态变量。

| 函数 | 干嘛的 |
|---|---|
| `ApplicationCounter`（静态） | 记录当前有几个世界在激活这个 Action |
| `OnGameFeatureActivating()` | `++Counter`，**从 0 变 1 时才**启用前端性能设置 |
| `OnGameFeatureDeactivating()` | `--Counter`，**减到 0 才**关闭（带 `check(Counter >= 0)`） |

## 顶部那段注释解释得很清楚

```
用户设置（以及它们驱动的引擎性能/可扩展性设置）是全局的，
所以多人 PIE 时没必要按世界分别跟踪 —— 只要有任意一个 PIE 世界在菜单里就应用。

但默认情况下，编辑器里不会应用前端性能设置，
除非开发者设置里的 ApplyFrontEndPerformanceOptionsInPIE 被打开。
```

> 💡 **这个静态计数器就是前面 `GameFeatureAction_SplitscreenConfig` 那套"投票制"的翻版** —— 同一个模式在 Lyra 里用了两次，都是为了处理"多世界/多实例对同一个全局设置有意见"的情况。
