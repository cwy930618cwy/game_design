# `LyraGameEngine.h` 速览

> 一个**空的扩展点**，全类只有一个 override。

| 成员 | 干嘛的 |
|---|---|
| `UCLASS()` 无 `Config` | 不像 GameInstance/Session 那样读 ini |
| `: UGameEngine` | 在 `ULyraGameInstance` 的**更下一层**（引擎级） |
| `ULyraGameEngine()` | 构造函数声明 |
| `Init(IEngineLoop*)` | 唯一 override，引擎初始化时调用 |

**说明**：Lyra 保留了这个类但没往里写东西，纯粹是为了将来需要改引擎初始化流程时有个地方落脚。
