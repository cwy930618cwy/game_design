# LyraAssetManagerStartupJob.h 是干嘛的

> 文件：`LyraStarterGame/Source/LyraGame/System/LyraAssetManagerStartupJob.h`（只有 46 行）
> 它不是一个"功能类"，而是一个**小工具模板**：给"启动时要做的一件加载任务"统一包装成**名字 + 权重 + 能报进度**的形式。
> 主角其实是**进度条**。

---

## 0. 先讲为什么要这个文件

游戏刚启动时，`ULyraAssetManager` 要连着干好几件重活：读 GameData、初始化 GameplayCueManager 的资源、加载默认 PawnData 等。

如果主界面一直转圈、没有任何进度提示，玩家会以为卡死了。所以 UE/Lyra 的做法是：

1. 把每件重活包成一个"**启动任务**"；
2. 每个任务给它一个**权重**（占总进度的多少）；
3. 任务执行过程中不断向外面**报进度**；
4. 汇总成 0%～100%，喂给加载界面。

`FLyraAssetManagerStartupJob` 就是第 1～3 步的载体。

> 类比：一张"任务卡"。卡上写着：这活叫什么（JobName）、要干什么（JobFunc）、占整个工程的百分之几（JobWeight）、以及"干到哪儿了"怎么对外喊（SubstepProgressDelegate）。

---

## 1. 第 7 行：先定义一个"喇叭类型"

```cpp
DECLARE_DELEGATE_OneParam(FLyraAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);
```

这行是 UE 的宏，意思是：**声明一个"只有一个 float 参数的回调类型"**，名字叫 `FLyraAssetManagerStartupJobSubstepProgress`。

人话：**定义了一个"只报进度的喇叭"的类型**。喇叭一响就吐一个 0～1 之间的数（"我这个任务干到 0.37 了"）。

`DECLARE_DELEGATE_OneParam(类型名, 参数类型)` 拆开看：

```
DECLARE_DELEGATE_  → 声明一个委托（回调）
OneParam           → 它带 1 个参数
SubstepProgress    → 报的是"子步骤"的进度
```

---

## 2. 五个成员变量（第 12～16 行）：这个任务卡上写了什么

```cpp
FLyraAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;   // 喇叭
TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;  // 要干的活
FString JobName;      // 任务名（打日志用）
float   JobWeight;    // 权重（占总进度的比例）
mutable double LastUpdate = 0;  // 上次报进度的时间（节流用）
```

| 成员 | 人话 | 谁关心它 |
|---|---|---|
| `SubstepProgressDelegate` | 报进度用的"喇叭"（回调） | 由外部（AssetManager）绑，任务自己喊 |
| `JobFunc` | 真正要执行的代码（一个可调用体） | 任务执行时被 `DoJob` 调用 |
| `JobName` | 任务名字 | 日志、调试 |
| `JobWeight` | 这活有多重 | 换算总体进度 |
| `LastUpdate` | 上一次报进度的时间戳 | 只是节流用，别频繁喊 |

两个细节：

- **`JobFunc` 的签名有两个参数**，第 2 个是 `TSharedPtr<FStreamableHandle>&`（引用）。
  意思是：**这个函数要负责"去加载并把自己产生的加载句柄塞进第 2 个参数里"**，让外面能知道"这活有没有产生异步加载"。
- **`LastUpdate` 是 `mutable` 的**：因为下面报进度的函数是 `const` 成员函数（承诺"不改对象状态"），但为了节流又必须更新时间戳。`mutable` 就是"虽然我对外是只读的，但这个字段内部允许改"。

---

## 3. 构造函数（第 19～23 行）：把任务卡填好

```cpp
FLyraAssetManagerStartupJob(const FString& InJobName, const TFunction<...>& InJobFunc, float InJobWeight)
    : JobFunc(InJobFunc), JobName(InJobName), JobWeight(InJobWeight)
{}
```

就是"给我名字、活、权重，我记下来"。注意它只收三样东西，**喇叭是后面外部再绑定的**（所以可以先建卡、后配喇叭）。

注释 `Simple job that is all synchronous` 是在说明：**这个构造函数构造出来的最典型用法就是"同步干完的活"**——不管你是同步还是异步，接口都一样。

实际使用中最方便的是配 **lambda**，比如 Lyra 里的宏（`LyraAssetManager.cpp` 第 30～31 行）：

```cpp
#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FLyraAssetManagerStartupJob(#JobFunc, [this](const FLyraAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)
```

读法：`STARTUP_JOB_WEIGHTED(GetGameData(), 25.f)` → 建一个名字叫 `"GetGameData()"`、权重 25 的任务卡，塞进 `StartupJobs` 数组。

---

## 4. `DoJob()`：真正触发一次执行（声明在第 26 行，实现在 .cpp）

```cpp
/** Perform actual loading, will return a handle if it created one */
TSharedPtr<FStreamableHandle> DoJob() const;
```

它在 `.cpp` 里做的事，用文字说就是三步：

1. **调用 `JobFunc`**（把"活"跑起来，并把产生的加载句柄回填到第 2 个参数）；
2. **如果产生了句柄**：把本任务的"喇叭"接到这个句柄的更新事件上（`BindUpdateDelegate`），然后 `WaitUntilComplete` **阻塞等它加载完**；
3. **返回句柄**。

为什么值得存在这么一层？因为它把"**加载 + 报进度 + 阻塞等待**"这三件琐事**统一封装**了。写每个具体任务的人，只需要往 `JobFunc` 里写"我要加载什么"，不用每次重复写绑定进度、等待完成这些模板代码。

---

## 5. 两个报进度的函数（第 28～45 行）

### 5.1 `UpdateSubstepProgress`：手动喊一声

```cpp
void UpdateSubstepProgress(float NewProgress) const
{
    SubstepProgressDelegate.ExecuteIfBound(NewProgress);
}
```

任务内部自己算出了进度（比如"我要加载 10 个东西，现在到第 3 个"），就调它喊一声。

`ExecuteIfBound` = **"喇叭绑了才喊，没绑就什么都不做"**。这是 UE 委托的标准安全写法，避免空调用崩溃。

### 5.2 `UpdateSubstepProgressFromStreamable`：让"流式加载句柄"自己报进度

```cpp
void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
{
    if (SubstepProgressDelegate.IsBound())
    {
        // StreamableHandle::GetProgress traverses() a large graph and is quite expensive
        double Now = FPlatformTime::Seconds();
        if (LastUpdate - Now > 1.0 / 60)
        {
            SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
            LastUpdate = Now;
        }
    }
}
```

先看它的用途：`FStreamableHandle` 是 UE 的资源异步加载句柄，它自己知道"我加载到百分之几了"。这个函数就是**把句柄的进度转发到喇叭上**，这样任务在等异步加载时进度条也会动。

注释里那句很重要：**`GetProgress()` 会遍历一张很大的依赖图，相当贵**。所以不能每帧都调，要**节流**——本意是"最多每秒报 60 次"。

**⚠️ 一个读代码时值得注意的地方**：这里的判断按字面看是写反了 —— `LastUpdate - Now > 1.0/60`（旧时间减当前时间）永远不可能成立（`LastUpdate` 初始是 0，只有条件成立才会被更新，于是永远不成立），那这个分支实际就**不会执行**，子进度也就不会通过句柄上报。

如果你之后要抄这个模板到自己项目里，这里应该写成 `Now - LastUpdate > 1.0 / 60`。这是这个文件里唯一需要留个心眼的地方，看源码时被绕住说明你要比那些"扫一眼就说懂了"的人认真。

---

## 6. 它在整个启动流程里的位置

三方分工很清楚（都在 `LyraAssetManager.cpp` 里）：

| 角色 | 负责 |
|---|---|
| `ULyraAssetManager::StartupJobs`（`TArray<FLyraAssetManagerStartupJob>`） | 攒一堆任务卡 |
| `FLyraAssetManagerStartupJob`（本文件） | 一张卡：干活 + 报自己的进度 |
| `ULyraAssetManager::DoAllStartupJobs()` | 按顺序执行 + **把每个任务的进度折算成总进度** |

折算的算法（`DoAllStartupJobs` 的核心）：

```
 ① 先把所有任务的 JobWeight 加起来 = TotalJobValue
 ② 每条任务开始前，给它的喇叭绑一个 lambda：
      总进度 = (已完成的权重和 + 本任务权重 × 本任务子进度) / 总权重
 ③ 调 StartupJob.DoJob() 执行
 ④ 执行完，解绑喇叭，把这条任务的权重加进"已完成"
```

所以 `JobWeight` 不是摆设：`GetGameData()` 在 Lyra 里权重是 **25**（`STARTUP_JOB_WEIGHTED(GetGameData(), 25.f)`），意味着它一个人就占启动进度的相当大一块。

还有一个细节：**独立服务器（DedicatedServer）直接跳过进度汇报**，把任务挨个 `DoJob()` 跑完就行——因为服务器上没有加载界面，汇报进度纯属浪费。

---

## 7. 一句话总结

`FLyraAssetManagerStartupJob` = **"一张启动任务卡"**：

- 卡上写：**叫什么**（`JobName`）、**干什么**（`JobFunc`）、**多重**（`JobWeight`）；
- 卡上有：**一个喇叭**（`SubstepProgressDelegate`）用来对外报"我干到哪了"；
- 卡的执行入口是 `DoJob()`：干活 → 有异步句柄就接上喇叭并等它完成。

它的价值是**解耦**：具体任务只管往里塞"要加载什么"，进度怎么汇总、界面怎么显示，全都由 `ULyraAssetManager` 负责。这也是为什么 Lyra 要单独抽出这 46 行的一个小结构，而不是在每个加载函数里各写一遍进度上报代码。
