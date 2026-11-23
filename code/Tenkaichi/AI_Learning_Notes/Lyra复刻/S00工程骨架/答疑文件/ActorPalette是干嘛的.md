# 答疑 — `ActorPalette` 是干嘛的？有它没它有什么区别？

> 触发：`.uproject` 里这一项 ——
> ```jsonc
> {
>     // Actor调色板： 编辑器里把常用 Actor 存成面板，一键拖进场景
>     "Name": "ActorPalette",
>     "Enabled": true
> },
> ```
> 实查：`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\`（插件源，15 个源文件）
> 结论：**纯编辑器用的「Actor 素材面板」—— 从已有关卡里挑 Actor，直接拖进当前关卡。`"Type": "Editor"` 模块，打包零体积、运行时零影响。**

---

## 一、先说结论（3 句话）

1. **它是「Actor 调色板」** —— 打开一个只读的关卡预览面板，点住里面的 Actor 直接拖进你正在编辑的关卡。
2. **纯编辑器工具** —— 模块类型是 `"Type": "Editor"`，打包出去的包里**根本没有它**。
3. **有它没它，游戏跑起来一模一样** —— 只影响你（美术/关卡策划）做关卡时的效率。

---

## 二、官方描述（`.uplugin` 原文）

`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\ActorPalette.uplugin`

```json
{
    "FriendlyName" : "Actor Palette",
    "Description" : "Allows creation of Actor Palettes based on existing levels to quickly select actors and drag them into the level editor",
    "Category" : "Experimental",
    "IsBetaVersion" : true,
    "EnabledByDefault" : false,
    "Modules" : [
        {
            "Name" : "ActorPalette",
            "Type" : "Editor",
            "LoadingPhase" : "Default"
        }
    ]
}
```

逐字段：

| 字段 | 值 | 含义 |
|---|---|---|
| `FriendlyName` | `Actor Palette` | 编辑器里显示的名字 |
| `Description` | 「允许基于已有关卡创建 Actor 调色板，快速挑选 Actor 并拖进关卡编辑器」 | 官方一句话 |
| `Category` | `Experimental` | **实验性**插件（Epic 不保证 API 稳定） |
| `IsBetaVersion` | `true` | Beta 版 |
| `EnabledByDefault` | `false` | **引擎默认不启用**，必须工程自己写 `"Enabled": true` |
| `Modules[0].Type` | **`Editor`** | ★ 只在编辑器加载，不参与打包 |
| `LoadingPhase` | `Default` | 编辑器启动时加载 |

> 🔑 `"Type": "Editor"` 是理解这个插件的关键 —— 它**永远不会出现在打包出来的游戏里**。

---

## 三、它具体怎么工作（源码实查）

### 3.1 注册 4 个可停靠标签页

`Source\ActorPalette\Private\ActorPaletteModule.cpp:57-76`

```cpp
// 在编辑器的「工具」菜单下，加一个名为 "Actor Palette" 的分组
TSharedRef<FWorkspaceItem> ActorPaletteGroup = MenuStructure.GetToolsCategory()->AddGroup(
    LOCTEXT("WorkspaceMenu_ActorPaletteCategory", "Actor Palette"),
    LOCTEXT("ActorPaletteMenuTooltipText", "Open an Actor Palette tab."),
    ActorPaletteIcon,
    true);

// 注册 4 个标签页：ActorPaletteTab1 ~ ActorPaletteTab4
for (int32 TabIndex = 0; TabIndex < UE_ARRAY_COUNT(ActorPaletteTabs); TabIndex++)
{
    const FName TabID = FName(*FString::Printf(TEXT("ActorPaletteTab%d"), TabIndex + 1));
    ActorPaletteTabs[TabIndex].TabID = TabID;

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabID, ...)
        .SetDisplayName(GetActorPaletteLabelWithIndex(TabIndex))
        .SetTooltipText(LOCTEXT("ActorPaletteMenuTooltipText", "Open an Actor Palette tab."))
        .SetGroup(ActorPaletteGroup)
        .SetIcon(ActorPaletteIcon);
}
```

**最多 4 个调色板**（`ActorPaletteModule.h:10`）：

```cpp
#define MAX_ACTOR_PALETTES 4
```

### 3.2 每个标签页 = 一个「只读预览视口」

`Source\ActorPalette\Private\ActorPalette.h:12-25`

```cpp
// 每个标签页是一个 Slate 复合控件
class SActorPalette : public SCompoundWidget
{
    void Construct(const FArguments& InArgs, int32 InTabIndex);

private:
    TSharedPtr<SActorPaletteViewport>       ActorPaletteViewport;       // 视口控件
    TSharedPtr<FActorPaletteViewportClient> ActorPaletteViewportClient; // 视口客户端
    int32 TabIndex;                                                     // 自己是第几个标签
};
```

内部是个 **`FPreviewScene`**（隔离的小世界，只渲染不参与游戏）：

`Source\ActorPalette\Private\ActorPaletteViewportClient.cpp:34`

```cpp
PreviewScene = &OwnedPreviewScene;
```

### 3.3 ★「一键拖进场景」的真实实现：伪造一次 Slate 拖放

`Source\ActorPalette\Private\ActorPaletteViewportClient.cpp:88-116`

```cpp
// 先取消当前任何正在进行的拖放（避免和主编辑器的拖放冲突）
FSlateApplication::Get().CancelDragDrop();

// 构造一个「资产拖放操作」，假装用户从内容浏览器拖了资产
TSharedPtr<FAssetDragDropOp> DragDropOperation = FAssetDragDropOp::New(FAssetData(Assets[0], true));

...

// 造一个假的鼠标事件
FDragDropEvent DragDropEvent(FakePointerEvent, DragDropOperation);
// 通知 Slate：开始拖放
FSlateApplication::Get().ProcessDragEnterEvent(OwnerWindow.ToSharedRef(), DragDropEvent);
```

**原理**：在调色板里点住某个 Actor 时，插件并不自己「生成」Actor，而是**伪造一次标准的资产拖放事件**丢给 Slate。主编辑器的关卡视口收到这个拖放事件后，按它自己那套「拖资产进关卡」的流程把 Actor 摆下去。

**好处**：完全复用引擎原生的拖放逻辑（吸附、对齐、Undo/Redo 全部都自动有）。

### 3.4 记住每个标签页上次打开的关卡

`Source\ActorPalette\Private\ActorPaletteSettings.h:28-58`

```cpp
// 配置存到「编辑器个人配置」（EditorPerProjectUserSettings）
UCLASS(config=EditorPerProjectUserSettings)
class UActorPaletteSettings : public UDeveloperSettings
{
public:
    // 每个关卡记住的位置等设置
    UPROPERTY(config) TArray<FActorPaletteMapEntry> SettingsPerLevel;

    // 最近打开过的关卡列表（任何标签页打开都会冒到最前）
    UPROPERTY(config) TArray<FString> RecentlyUsedList;

    // 每个标签页上次打开的关卡（按标签索引）
    UPROPERTY(config) TArray<FString> MostRecentLevelByTab;

    // 收藏的关卡（标为「常用调色板」的）
    UPROPERTY(config) TArray<FString> FavoritesList;

    // 最近关卡保留条数，默认 10，范围 0~25
    UPROPERTY(config, EditAnywhere, Category=ActorPalette, meta=(ClampMin=0, ClampMax=25))
    int32 NumRecentLevelsToKeep = 10;
};
```

数据模型（源码注释原文翻译）：

```
· 每个标签页记住自己上次打开的关卡
· 有一个共享的「最近关卡」列表（任何标签页打开关卡都会冒到顶部）
· 有一个共享的「收藏」列表
· 标签页打开期间有独立的视口设置，但只有最近一次交互会更新最近/收藏列表
```

---

## 四、使用场景（具体例子）

### 场景：美术要做「森林场景」

需要大量不同种类的树、灌木、石头。

**没有这个插件**：

```
① 打开「树素材.umap」→ 找到那棵橡木 → Ctrl+C
② 切回「森林场景.umap」→ Ctrl+V
③ 想换一棵 → 再切回去找 → 再复制粘贴
④ 需要 50 棵不同种类 → 重复 50 次以上
```

**有这个插件**：

```
① 工具菜单 → Actor Palette → 打开 ActorPaletteTab1
   选「树素材.umap」→ 视口里显示所有树，可以直接点选
② 切回「森林场景.umap」，从调色板面板点住橡木 → 拖进关卡 ✅
③ 打开 ActorPaletteTab2 → 选「灌木素材.umap」→ 拖灌木
④ 打开 ActorPaletteTab3 → 选「石头素材.umap」→ 拖石头
   （4 个标签页 = 同时盯着 4 个素材库）
⑤ 每个标签页还会记住上次打开的关卡，下次打开直接就在
```

### 另一个场景：做 Boss 战关卡

需要复用「旧版本 Boss 房的装饰」：

```
调色板打开「旧Boss房.umap」
 → 视口里逐个点选石柱、火盆、雕像
 → 直接拖进新关卡，位置/旋转/缩放原样带过来
 → 不用在两个关卡之间反复切换
```

---

## 五、有它 vs 没它

### 5.1 编辑器层

| | 有 | 没有 |
|---|---|---|
| 「工具」菜单 | 多一个 **Actor Palette** 分组 | 没有 |
| 可打开标签页 | `ActorPaletteTab1` ~ `Tab4`（4 个） | 没有 |
| 跨关卡拖 Actor | ✅ 点住直接拖 | ❌ 只能 Ctrl+C / Ctrl+V |
| 最近关卡 / 收藏列表 | ✅ 有 | ❌ |
| 编译 | 多编一个编辑器 DLL（15 个源文件） | — |
| 编辑器内存 | 每个标签页开一个 `FPreviewScene`（吃一点显存） | — |

### 5.2 打包 / 运行时层（★ 关键）

| | 有 | 没有 |
|---|---|---|
| 打出来的 `.pak` 体积 | **完全一样** | — |
| 游戏运行时 | **完全一样** | — |
| 服务器端 | **完全一样** | — |
| 玩家看到的内容 | **完全一样** | — |

**原因**：

```
"Type": "Editor"
   ↓
UBT 只在 TargetType.Editor 时编这个模块
   ↓
打包 Game / Client / Server 时，这个模块压根不参与
   ↓
游戏包里没有 Unreal-ActorPalette.dll
```

> 🔑 **它 100% 只活在你的编辑器里。** 关掉它，游戏跑起来一模一样，只是做关卡时少了个效率面板。

---

## 六、为什么 Lyra 要启用它

| # | 理由 | 说明 |
|---|---|---|
| 1 | **Lyra 是关卡密集型示例工程** | 有多张地图 + 大量共享素材（角色 / 武器 / 掩体 / 装饰），跨关卡复用需求大 |
| 2 | **Epic 官方新建工程默认就带** | Launcher 上「新建 Lyra 工程」时，插件列表里就有 `ActorPalette` |
| 3 | **零代价** | Editor 模块，不影响打包体积、不影响运行时性能 |
| 4 | **一比一** | Lyra `.uproject:23-26` 就这么写的 |

### 与 Lyra 源的对照

`E:\ue5\LyraStarterGame\LyraStarterGame.uproject:23-26`

```json
{
    "Name": "ActorPalette",
    "Enabled": true
},
```

和你这份**一字不差** ✅

---

## 七、注意事项

### 7.1 它是「实验性」插件

```
"Category": "Experimental"
"IsBetaVersion": true
```

含义：
- Epic **不保证 API 稳定**，未来引擎版本可能改接口甚至移除
- 编辑器升级后如果菜单里找不到它 → 先去「编辑 → 插件」面板确认有没有被默认关掉

### 7.2 它是 `EnabledByDefault: false`

```
"EnabledByDefault" : false
```

**引擎默认不启用它**。你现在能用的唯一原因就是 `.uproject` 里明确写了 `"Enabled": true`。

如果哪天误删了这一项 → 重启编辑器后「工具」菜单里的 Actor Palette 分组会消失。**不会报错，只是功能没了**（静默失效）。

### 7.3 别把它和 `AESGCMHandlerComponent` 搞混

| 插件 | 类型 | 干什么 | 运行时影响 |
|---|---|---|---|
| `ActorPalette` | **Editor** | 编辑器里的 Actor 素材面板 | **零** |
| `AESGCMHandlerComponent` | **Runtime** | 网络封包 AES-256-GCM 加密 | 配了 ini 才生效，否则零 |

两者在 `.uproject` 里是相邻的两项（第 1 / 第 2），但**功能毫不相干**。

---

## 八、一句话总结

**`ActorPalette` = 纯编辑器的「Actor 素材面板」：从已有关卡里挑 Actor，直接拖进当前关卡（实现方式是伪造 Slate 资产拖放事件）。`"Type": "Editor"` 模块 → 打包零体积、运行时零影响；有它做关卡更快，没它游戏照跑。它是实验性插件且默认不启用，靠 `.uproject` 里那行 `"Enabled": true` 才有。**

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\ActorPalette.uplugin:1-25`（`FriendlyName` / `Description` / `Category:Experimental` / `IsBetaVersion:true` / `EnabledByDefault:false` / `Modules[0].Type:Editor`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Public\ActorPaletteModule.h:10`（`#define MAX_ACTOR_PALETTES 4`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Public\ActorPaletteModule.h:12-38`（`FActorPaletteModule` 类）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Private\ActorPaletteModule.cpp:49-77`（`StartupModule`：注册工具菜单分组 + 4 个标签页）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Private\ActorPalette.h:12-25`（`SActorPalette` 控件）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Private\ActorPaletteViewportClient.cpp:34`（`PreviewScene = &OwnedPreviewScene;`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Private\ActorPaletteViewportClient.cpp:88-116`（伪造 `FAssetDragDropOp` + `ProcessDragEnterEvent`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\Private\ActorPaletteSettings.h:10-58`（`UActorPaletteSettings`：4 个列表 + `NumRecentLevelsToKeep`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\ActorPalette\Source\ActorPalette\`（15 个源文件：`ActorPalette.cpp` / `ActorPaletteViewport.cpp` 16967 B / `ActorPaletteViewportClient.cpp` 6886 B / `ActorPaletteSettings.cpp` 3400 B / `ActorPaletteStyle.cpp` 3137 B 等）
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject:23-26`（`"Name": "ActorPalette", "Enabled": true`）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject`（第 1 项与 Lyra 一字不差）