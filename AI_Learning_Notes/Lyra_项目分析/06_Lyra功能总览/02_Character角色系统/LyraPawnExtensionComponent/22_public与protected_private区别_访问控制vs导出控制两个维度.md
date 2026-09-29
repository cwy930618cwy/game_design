# 22 — 都已经用 `UE_API`/`MinimalAPI` 控制了暴露，类里的 `public:` `protected:` 又是干嘛的？

> **定位**：上一篇讲了 `UE_API`/`MinimalAPI` 是"类的一层暴露控制"。你看完会产生新困惑：**"外面都封一层了，类里面那个 `public:`、`protected:` 不是多余的吗？它俩又有什么区别？"**
>
> 这篇把**两个不同维度**彻底分清：
> 1. `UE_API` / `MinimalAPI` = **跨模块（DLL）层**的"谁看得到符号"。
> 2. `public:` / `protected:` / `private:` = **C++ 语法层**的"谁能写代码访问"。
>
> 它俩不冲突、不重复，是**叠在一起、两个都要满足**的关系。

---

## 〇、一句话先分清：这是"两层锁"，不是"一道锁"

> **`UE_API`/`MinimalAPI` 管的是"这个 DLL 里的东西，别的 DLL 能不能链接到"（门外的保安）。`public/protected/private` 管的是"就算进到这栋楼里，谁有权限碰这个房间"（门内的权限）。**
>
> 一个管"**跨模块能不能找到**"，一个管"**同模块内谁能访问**"——两把锁叠着用。

```
┌──────────────────────────────────────────────┐
│  DLL 边界（跨模块）                            │
│  UE_API / MinimalAPI = 门外保安                │
│  没导出 → 别的 DLL 连符号都找不到（链接错误）     │
│                                              │
│  ┌────────────────────────────────────────┐  │
│  │  C++ 语法层（类内部）                    │  │
│  │  public/protected/private = 门内门禁     │  │
│  │  没权限 → 编译时就报错（不能这样写）        │  │
│  │  ┌──────────────────────────────┐      │  │
│  │  │  类的实际成员                    │      │  │
│  │  └──────────────────────────────┘      │  │
│  └────────────────────────────────────────┘  │
└──────────────────────────────────────────────┘
```

---

## 一、两个维度分开看（先各自独立理解）

### 维度 1：`UE_API` / `MinimalAPI` —— 跨模块（DLL）层

作用在 **模块边界**。UE 每个模块编成独立 DLL，符号默认不跨 DLL 可见。

```cpp
// 没有 UE_API / 没被导出 → 别的 DLL 根本找不到这个符号
void HiddenFunc();                      // 别的模块 include 了也链接失败

// 有 UE_API → 别的 DLL 能链接到
UE_API void VisibleFunc();              // 别的模块可以用
```

**这一层回答的问题是：这个符号"出不出得了这个 DLL"？**

### 维度 2：`public:` / `protected:` / `private:` —— C++ 语法层

作用在 **同一个类里，谁能用代码访问成员**。哪怕都在同一个 DLL、同一个模块，也要过这关。

```cpp
class UMyClass
{
public:      // 谁都能写代码访问（同模块任何地方）
    float A;

protected:   // 只有本类 + 派生类（子类）能访问
    float B;

private:     // 只有本类自己能访问（连子类都不行）
    float C;
};
```

**这一层回答的问题是：就算符号能看到，你"有没有资格写 `obj.成员`"？**

---

## 二、核心：两个维度是"叠加的与关系"

要真正**从模块外访问一个成员**，两把锁**都得开**：

> **必须同时满足：① 符号被导出（UE_API / 类级导出）+ ② 成员是 public（或有权限）。**

```
别的 DLL 想访问 obj.SomeFunc()
  需要 ① UE_API 导出该函数？  ── 否 → 链接错误（LNK 找不到符号）
        │ 是
        ▼
  需要 ② SomeFunc 是 public？ ── 否 → 编译错误（不能访问私有成员）
        │ 是
        ▼
    ✅ 成功调用
```

### 两把锁各管一种错误（最容易记的区别）

| 场景 | 缺哪把锁 | 报什么错 | 错误的"层级" |
|---|---|---|---|
| 函数是 `public` 但**没写 UE_API** | 缺① | **链接错误**（LNK2001/LNK2019：找不到符号） | 链接期（link） |
| 函数写了 `UE_API` 但是 `private` | 缺② | **编译错误**（无法访问 private 成员） | 编译期（compile） |
| 全满足 | ✅ | 正常 | —— |

> **教学场景（用报错类型反推）**：你写了个工具类 `UMyTool`，别的模块调用 `UMyTool::DoThing()`：
> - 如果报 **C2248 "cannot access private member"** → 是 `private:` 的锅，把它挪到 `public:`。
> - 如果报 **LNK2019 "unresolved external symbol"** → 是没导出的锅，加 `UE_API`。
> - 两个错名不一样，就是因为它们是**两把不同的锁、两个不同的阶段**。

---

## 三、那 `public:` / `protected:` / `private:` 三者到底啥区别？

用一张表 + 例子讲清：

| 访问级别 | 谁能访问 | 典型用途 |
|---|---|---|
| `public:` | **所有人**（同模块任何类/函数都能 `obj.成员`） | 对外接口：别人要调用你的功能 |
| `protected:` | **本类 + 子类（派生类）** | 给子类扩展的钩子、子类要读的数据 |
| `private:` | **只有本类自己** | 内部实现细节：别人不该碰的 |

```cpp
class UBase
{
public:      void PublicFunc();    // 任何人：obj.PublicFunc()
protected:   void ProtectedFunc(); // 只有 Base 和子类能调
private:     void PrivateFunc();   // 只有 Base 自己能调
};

class UChild : public UBase
{
    void Test()
    {
        PublicFunc();     // ✅ 子类能用 public
        ProtectedFunc();  // ✅ 子类能用 protected（这就是它的意义！）
        // PrivateFunc(); // ❌ 编译错：子类也不能碰父类 private
    }
};

void SomeOutsideFunction(UBase* Obj)
{
    Obj->PublicFunc();     // ✅ 外部能用 public
    // Obj->ProtectedFunc(); // ❌ 外部不能碰 protected
    // Obj->PrivateFunc();   // ❌ 外部不能碰 private
}
```

> **记忆口诀**：`public` = 谁都能进的门；`protected` = 只有"自己家 + 儿女"能进；`private` = 只有"自己"能进。**范围：public ≥ protected ≥ private。**

---

## 四、回到真实文件：`LyraPawnExtensionComponent` 的 public/protected 分部

看这个类（`.h` 第 31~103 行），正好能说明"为什么要分层"：

### `public:` 区（第 31~81 行）—— 放"别人要调用的接口"

```cpp
public:
	UE_API ULyraPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);  // 构造函数：别人要创建它

	static UE_API const FName NAME_ActorFeatureName;   // 外部模块要引用的静态名（第 20 篇）

	// 状态接口（override）
	virtual FName GetFeatureName() const override;     // 接口必须 public，因为大管家要调它
	UE_API virtual bool CanChangeInitState(...) const override;
	UE_API virtual void OnActorInitStateChanged(...) override;

	// 对外功能：谁要指挥它就用这些
	UE_API void SetPawnData(...);                       // GameMode 调用
	UE_API void InitializeAbilitySystem(...);           // Hero 组件调用
	UE_API void HandleControllerChanged();              // Pawn 自己调
	...
```

**规律**：凡是"**别的类（GameMode / HeroComponent / Pawn 本身）要调我**"的函数 → 放 `public`。注意很多还配了 `UE_API`（既 public 又导出，两把锁都开），因为调用方在别的模块。

### `protected:` 区（第 82~103 行）—— 放"生命周期回调 + 内部数据"

```cpp
protected:
	UE_API virtual void OnRegister() override;    // 引擎生命周期回调
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(...) override;

	UFUNCTION()
	UE_API void OnRep_PawnData();                 // 网络同步回调（引擎调）

	// 内部数据：不应该被外部直接改
	UPROPERTY(...) TObjectPtr<const ULyraPawnData> PawnData;            // 用 SetPawnData 改，别直接碰
	UPROPERTY(...) TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;  // 内部缓存
```

**规律**：凡是"**引擎/框架回调我**（不是我给别人调）"或者"**内部数据不想被外部乱改**"的 → 放 `protected`。放 protected 意味着：
- **引擎能调**（通过虚函数机制，不受访问控制限制）；
- **子类能重写/能读**（方便 Lyra 以后派生扩展）；
- **外部普通代码不能乱调/乱改**（保持封装）。

> **对照着看**：`SetPawnData` 是 public（因为 GameMode 要从外面设置数据），而数据本身 `PawnData` 是 protected（不让外部直接 `PawnData = xxx`，必须走 `SetPawnData` 这个"正规通道"）。**public 提供"入口"，protected/private 藏住"实现"。** 这就是封装的意义。

---

## 五、为什么 UE 类里几乎没有 `private:` 数据？—— 常见现象解释

你可能注意到很多 UE 类的数据成员放在 `protected` 而不是 `private`。原因：
- UE 大量用**蓝图继承 / C++ 子类**，作者希望子类能访问、能扩展 → 放 protected。
- `UPROPERTY` + `BlueprintReadOnly` 等可以配合，让蓝图"只读"而 C++ 子类可改。
- 真正的 `private:` 在 UE 类里较少见（除非绝不希望子类碰），因为 UE 强调"可继承、可扩展"。

> 所以别纠结"为什么不全是 private"——**UE 的封装习惯是"外部代码别碰"用 protected 就够，因为外部代码本来也不继承它；而自己人（子类/蓝图）需要留口子。**

---

## 六、一张总图：两套控制叠在一起

```
        维度1：跨模块（UE_API / MinimalAPI / LYRAGAME_API）
        "这个符号出不出得了 DLL？"  → 决定：链接期找不找得到
┌───────────────────────────────────────────────────────────┐
│                   维度2：C++ 访问控制                      │
│                   public / protected / private            │
│              "进了门，谁有权限碰？"  → 决定：编译期准不准写   │
│  ┌─────────────────────────────────────────────────────┐  │
│  │  public   = 谁都能调（对外接口）                      │  │
│  │  protected= 本类+子类（生命周期回调/内部数据/扩展钩子） │  │
│  │  private  = 只有本类（UE 类少见）                     │  │
│  └─────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────┘
  两把锁都必须开，才能跨模块真正使用一个成员：
  public 且被导出 → 别人能调用
  public 但没导出 → 链接错误
  导出但 private  → 编译错误
```

---

## 七、总结一句话

> **`UE_API`/`MinimalAPI` 和 `public/protected/private` 是两层不同的控制，不是重复。** 前者管**跨 DLL 的符号可见性**（缺了链接期报"找不到符号"），后者管**C++ 代码层谁能访问**（缺了编译期报"不能访问私有成员"）。想从模块外真正用一个成员，两把锁都得开：**要 public（编译期有权限）且要导出（链接期找得到）**。在 `LyraPawnExtensionComponent` 里：`public` 区放"别人调我的接口"（如 `SetPawnData`/`InitializeAbilitySystem`），`protected` 区放"引擎/框架回调我 + 内部数据"（如 `OnRegister`/`PawnData`）——**public 给入口、protected/private 藏实现，这就是封装。**

---

## 八、下一步

- 试着把 `SetPawnData` 从 public 挪到 private，编译看报错类型（C2248）——理解"语法层访问控制"。
- 试着删掉某个成员前的 `UE_API`，编译+链接看报错（LNK）——理解"模块层导出"。
- 去 UE 引擎类里搜 `private:` 出现较少的现象，思考 UE 为什么偏爱 protected + UPROPERTY 组合。
