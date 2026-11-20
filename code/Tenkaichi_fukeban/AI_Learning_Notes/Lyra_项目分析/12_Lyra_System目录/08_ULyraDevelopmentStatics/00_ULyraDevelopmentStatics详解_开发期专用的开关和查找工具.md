# ULyraDevelopmentStatics 是干嘛的

> 文件：`LyraStarterGame/Source/LyraGame/System/LyraDevelopmentStatics.h`（53 行）+ 同名 `.cpp`（199 行）
> **一句话**：这是 Lyra 的"**开发期工具箱**"——里面全是**只在编辑器里才生效**的调试开关，外加两个开发时好用的小工具（找 PIE 的服务器世界、按短名字找类）。
> 名字里的 `Development` 就是这个意思：跟正式发行版本的游戏玩法没关系。

---

## 0. 它为什么会被单独抽出来

调试游戏时会遇到一堆"**开发时想省事，上线时绝不能这样**"的需求：

- 编辑器里点 Play，不想每次都等"匹配热身/等玩家"那一套流程，想直接进玩法；
- 编辑器里不加载花里胡哨的外观背景，让 PIE 启动快一点；
- 让 AI（bots）在编辑器里先别打人，方便自己看动画；
- PIE 同时开了客户端和服务器两个窗口，作弊命令要往哪个窗口发？

这些需求都是"**只在编辑器里**、**只在开发时**"的。Lyra 的做法是：全部集中到这个文件里，函数名很直白，实现里用 `#if WITH_EDITOR` 保证打包后行为固定。

---

## 1. 三个"开关查询"函数（第 24～35 行）

它们都是 `UFUNCTION(BlueprintCallable, Category="Lyra")`，所以**蓝图中直接可调**。它们本身不存状态，只是去读 `ULyraDeveloperSettings`（`Development/LyraDeveloperSettings.h`，在项目设置里能看到的 Lyra 开发者设置）里的开关。

| 函数 | 编辑器里读的设置 | 打包后固定返回 |
|---|---|---|
| `ShouldSkipDirectlyToGameplay()` | `bTestFullGameFlowInPIE` 取反 | `false` |
| `ShouldLoadCosmeticBackgrounds()` | `bSkipLoadingCosmeticBackgroundsInPIE` 取反 | `true` |
| `CanPlayerBotsAttack()` | `bAllowPlayerBotsToAttack` | `true` |

看一个实现（第 16～25 行）：

```cpp
bool ULyraDevelopmentStatics::ShouldSkipDirectlyToGameplay()
{
#if WITH_EDITOR
    if (GIsEditor)
    {
        return !GetDefault<ULyraDeveloperSettings>()->bTestFullGameFlowInPIE;
    }
#endif
    return false;
}
```

三个关键点：

1. **`#if WITH_EDITOR`**：这段代码**在打包出来的游戏里根本不存在**（编译器直接删掉），所以不占性能、也不会被误触发。
2. **`GIsEditor`**：即使是在编辑器里编译的版本，运行时也要确认"我确实在编辑器中"。
3. **`GetDefault<...>()`**：读的是设置类的默认对象（也就是项目设置面板里你填的那份），不需要任何实例。

所以它的语义就是：**"只有在编辑器里、且你改了设置"才会改变行为；其它一切情况都是固定值**。这就是头文件注释里写的：

- `ShouldSkipDirectlyToGameplay`："**永远是 false**，除非在编辑器中且 `bTestFullGameFlowInPIE` 为 false"
- `ShouldLoadCosmeticBackgrounds`："**永远是 true**，除非在编辑器中且 `bSkipLoadingCosmeticBackgroundsInPIE` 为 true"

> **⚠️ 两处可以留意的地方**
> 1. `CanPlayerBotsAttack()` 上方的注释（第 32～33 行）写着 "Should game logic load cosmetic backgrounds in the editor? ... `bSkipLoadingCosmeticBackgroundsInPIE`" —— 这明显是从上一个函数复制过来的，**注释写错了**，它实际管的是"bots 能不能攻击"，读的是 `bAllowPlayerBotsToAttack`。这类"注释与代码不符"在引擎/示例代码里很常见，读的时候要认代码不认注释。
> 2. `ShouldLoadCosmeticBackgrounds` 多了一个 `meta=(ExpandBoolAsExecs="ReturnValue")`（第 29 行）：这是给蓝图看的，效果是这个节点在蓝图里**展开成 true / false 两条执行引脚**，可以像 `Branch` 一样直接分岔（和 `LyraActorUtilities` 里 `ExpandEnumAsExecs` 是同一类技巧，只是这次展开的是 bool）。

---

## 2. `FindPlayInEditorAuthorityWorld()`（第 39 行声明，第 50～87 行实现）

### 它解决什么问题

PIE（Play In Editor）里你可能同时开着好几个窗口：一个 listen server、一个客户端、或者一个 dedicated server + 一个客户端。

**"作弊命令"（cheat）需要发到服务器那个世界才有意义**——因为服务器才有判定权。这个函数就是"**帮你在 PIE 的一堆世界里挑出最像服务器的那一个**"。

### 实现思路（第 50～87 行）

```cpp
UWorld* ServerWorld = nullptr;
#if WITH_EDITOR
for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())   // 遍历所有世界
{
    if (WorldContext.WorldType == EWorldType::PIE)                      // 只看 PIE 的
    {
        if (UWorld* TestWorld = WorldContext.World())
        {
            if (WorldContext.RunAsDedicated)     // ① 最理想：独立服务器
            { ServerWorld = TestWorld; break; }
            else if (ServerWorld == nullptr)     // ② 没有候选，先记下这个
            { ServerWorld = TestWorld; }
            else
            {
                // ③ 已有候选，比一比谁"更服务器"
                if (TestWorld->GetNetMode() < ServerWorld->GetNetMode())
                { return ServerWorld; }
            }
        }
    }
}
#endif
return ServerWorld;
```

挑选优先级：

1. **`RunAsDedicated` 的 PIE 世界**（就是 PIE 里选 "Dedicated Server" 那个）——最理想，直接拿走；
2. 否则**第一个碰到的 PIE 世界**当候选；
3. 后面对每个 PIE 世界都比较一下网络模式，**越小越像服务器**（回忆上一篇讲的：`NM_Standalone(0) < NM_DedicatedServer(1) < NM_ListenServer(2) < NM_Client(3)`，所以 `<` 意味着更偏向服务器端）。

**⚠️ 顺带提一下**：第 76～79 行那个分支比较完之后是 `return ServerWorld;`（直接把**旧候选**返回了），既没有把新世界替换成候选，也没有继续找。所以这段"比较逻辑"和上面注释表达的"挑更好的那个"的意图并不一致——更像是"发现有个更服务器的就收工"。实际效果上：因为 `RunAsDedicated` 优先、否则取第一个，所以大多数常见 PIE 配置（单窗口 / 服务器+客户端）结果都是对的，但这是读源码时可以记一笔的可疑写法。

### 谁在用它

`Cosmetics/LyraCosmeticDeveloperSettings.cpp`（第 59～62 行）：

```cpp
#if WITH_SERVER_CODE
    // Update the loadout on all players
    UWorld* ServerWorld = ULyraDevelopmentStatics::FindPlayInEditorAuthorityWorld();
```

场景：在编辑器里用"开发者设置"改外观配置，需要把改动**推给服务器世界上所有玩家**。它自己不在服务器上，所以要先"找到服务器世界"。

---

## 3. `FindClassByShortName`（第 42 行 + 第 44～48 行的模板重载）

### 它解决什么问题

在**作弊控制台**里手动敲类名时，没人愿意敲 `Type'/Game/Characters/BP_LyraCharacter.BP_LyraCharacter_C'` 这种长路径。这个函数就是"**你给个短名字，我尽量帮你找出来**"。

用法有两种：

```cpp
// 1) 传基类
UClass* C = ULyraDevelopmentStatics::FindClassByShortName(TEXT("BP_Foo"), AActor::StaticClass());

// 2) 模板版（第 44～48 行），省得写 StaticClass()
TSubclassOf<AActor> C = ULyraDevelopmentStatics::FindClassByShortName<AActor>(TEXT("BP_Foo"));
```

模板重载本身只是把 `AActor::StaticClass()` 这步包起来（第 47 行），**真正的逻辑全在非模板版本里**。

### 非模板版本干的事（第 130～199 行）

它是"先快后慢、层层兜底"的三段式：

**第一段：判断你给的名字"长得合不合法"**（第 137～164 行）

- 空字符串、或含空格 → 直接判定无效；
- 名字**不是短名**（即看起来像带路径的长名）时：
  - 如果含 `.`，就把 `Type'路径'` 这种形式转成对象路径（`FPackageName::ExportTextPathToObjectPath`），再拆成"包名 / 对象名"，并校验包名是否合法；
  - 不含 `.` → 判定无效；
- 是短名（比如 `BP_Foo`）→ 保持有效，后面直接用。

**第二段：先问"已加载的类"**（第 166～170 行）

```cpp
ResultClass = UClass::TryFindTypeSlow<UClass>(TargetName);
```

源码注释（第 136 行）解释了为什么它排在最前：**"先查原生类/已加载资产，再才轮到资产注册表"**——因为资产注册表查询慢，能省则省。

注意这里的 `TryFindTypeSlow` 本身就带 "Slow"：它是"按名字在所有已注册类型里找"，不是那种极快的哈希命中，但**比查资产注册表还是快得多**。

**第三段：还没有 → 去资产注册表翻蓝图**（第 173～176 行 → 第 89～128 行）

- `GetAllBlueprints()`：用 `FAssetRegistryModule` 拉一份**全项目的 `UBlueprint` 列表**（`FARFilter` + 递归路径）；
- `FindBlueprintClass()`：遍历列表，名字**或**对象路径匹配就取它的 `GeneratedClass`（蓝图编译后生成的真正类），再检查是不是目标基类的子类；
- 匹配前会先把名字结尾的 `_C` 去掉（第 107 行）——因为蓝图生成类的名字习惯带 `_C` 后缀。

**收尾：校验 + 报错**（第 178～196 行）

- 找到了但不是 `DesiredBaseClass` 的子类 → 打警告 "Found an asset %s but it wasn't of type %s"，返回 `nullptr`；
- 完全没找到 → 打警告 "Failed to find class of type %s named %s"。
- 这两个警告只有在 `bLogFailures = true`（默认）时才输出，调用方觉得失败很常见时可以关掉。

### 谁在用它

`Cosmetics/LyraCosmeticCheats.cpp`（第 30～33 行）：

```cpp
#if UE_WITH_CHEAT_MANAGER
    if (ULyraControllerComponent_CharacterParts* CosmeticComponent = GetCosmeticComponent())
    {
        TSubclassOf<AActor> PartClass = ULyraDevelopmentStatics::FindClassByShortName<AActor>(AssetName);
```

也就是在作弊命令里，**敲个名字就能给自己换外观部件**。

---

## 4. 一句话总结

`ULyraDevelopmentStatics` = **Lyra 的"开发期工具箱"**，东西不多但分两类：

| 类型 | 成员 | 本质 |
|---|---|---|
| **调试开关**（编辑器限定） | `ShouldSkipDirectlyToGameplay` / `ShouldLoadCosmeticBackgrounds` / `CanPlayerBotsAttack` | 去读 `ULyraDeveloperSettings`，`#if WITH_EDITOR + GIsEditor` 包住，**打包后固定值** |
| **开发小工具** | `FindPlayInEditorAuthorityWorld` / `FindClassByShortName`（+模板重载） | PIE 找服务器世界；按短名字找类（先查已加载类型，再查资产注册表） |

你要记住的判断标准其实就一条：**这个文件里的东西，都是为了"调试方便"，不属于游戏正式逻辑**。正式版里它们要么被编译掉，要么返回写死的安全值。
