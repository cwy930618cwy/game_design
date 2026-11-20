# `System/` 目录速览

> 13 个类，27 个文件，**没有子目录**。逐个文件后面单独拆。

| 类 | 干嘛的 |
|---|---|
| `LyraGameInstance` | 进程级入口，游戏从这里 Init / Shutdown |
| `LyraGameEngine` | 引擎层定制 |
| `LyraGameSession` | 网络会话：建服、加入、登录 |
| `LyraGameData` | 全局数据资产入口 |
| `LyraAssetManager` | 资源加载总控 |
| `LyraAssetManagerStartupJob` | 启动时的一个"加载工位" |
| `LyraReplicationGraph` | 网络同步优化：只同步附近的 |
| `LyraReplicationGraphSettings` | ReplicationGraph 的配置 |
| `LyraReplicationGraphTypes` | ReplicationGraph 用到的类型定义 |
| `LyraSignificanceManager` | 按重要性给远处对象降级 |
| `GameplayTagStack` | 让 GameplayTag 能带数量（如 弹药:30） |
| `LyraSystemStatics` | 通用静态工具函数 |
| `LyraDevelopmentStatics` | 开发期专用的静态工具 |
| `LyraActorUtilities` | Actor 相关工具函数 |

**优先级**：`LyraGameInstance` → `LyraAssetManager` → `LyraGameSession` → `LyraReplicationGraph`
