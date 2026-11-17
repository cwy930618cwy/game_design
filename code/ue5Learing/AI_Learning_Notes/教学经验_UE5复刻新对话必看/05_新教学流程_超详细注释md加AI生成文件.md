# 05 — 新教学流程：超详细注释 md + AI 直接生成还原文件

> **定位**：2026-11-16 用户明确要求的新工作流，**取代之前「只教不写代码」的做法**。
> **用户原话**：「以后这么教学把 新建一个超详细的md 然后生成文件 我就不写了」
> **适用范围**：`从零复刻ue5\` 全线所有还原源码文件。

---

## 一、新流程（3 步）

```
① 建超详细注释 md
   → 答疑文件\NN_<文件名>逐行注释版.md
   → 每行代码上面都有详细中文注释（讲清「干什么 + 为什么 + 类型从哪来」）

② AI 直接生成还原文件
   → ue_5.6\<同名路径>\<文件名>
   → 内容 = 超详细注释 md 里的代码（注释一起写进去）

③ 用户不写代码，只验收
```

---

## 二、和旧流程的区别

| 项 | 旧流程（作废） | 新流程（当前） |
|---|---|---|
| 谁写代码 | **用户手抄** | **AI 直接生成** |
| 讲义形式 | 干净代码（零注释）+ 逐段拆解 | **超详细注释版**（每行都有注释） |
| 用户做什么 | 照着抄 | **只验收** |
| 注释密度 | 类/方法上方一句 | **每行上面 1~5 行详细注释** |

> ⚠️ 旧铁律「只教不写代码」**作废**（2026-11-16 用户明确要求）。

---

## 三、超详细注释 md 的写法（照抄这个）

### 3.1 文件位置

```
AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\答疑文件\NN_<文件名>逐行注释版.md
```

### 3.2 注释密度标准

**每一行代码上面都要有注释**，且注释要讲清 4 件事：

| # | 讲什么 | 例子 |
|---|---|---|
| 1 | **这行在干什么** | 「判断这次编的是不是 Android 平台」 |
| 2 | **涉及的类型从哪来** | 「`Target.Platform` 是 `ReadOnlyTargetRules.cs:56` 的属性」 |
| 3 | **为什么这么写** | 「为什么只要头：`AndroidPlatformMisc.cpp` 要读 Vulkan 常量，但不调函数」 |
| 4 | **真实文件在哪**（涉及模块时） | 「真实位置：`Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs`」 |

### 3.3 样板（`09_Core.Build.cs逐行注释版.md`）

```csharp
// 判断「这次编的是不是 Android 平台」。
// Target.Platform 是 ReadOnlyTargetRules.cs:56 的属性，返回 UnrealTargetPlatform。
// UnrealTargetPlatform 是 partial struct（不是 enum），定义在 UEBuildTarget.cs:31。
// UnrealTargetPlatform.Android 是它的静态属性，定义在 UEBuildTarget.cs:267。
// == 运算符重载在 UEBuildTarget.cs:107，比较两个平台的内部 Id。
if (Target.Platform == UnrealTargetPlatform.Android)
{
	// 链 Google 游戏 SDK（第三方模块）。
	// 真实位置：Engine\Source\ThirdParty\GoogleGameSDK\GoogleGameSDK.Build.cs
	// 里面有 libswappy_static.a 等 Android 游戏库。
	PrivateDependencyModuleNames.Add("GoogleGameSDK");
```

### 3.4 md 结构

```
# NN — <文件名> 逐行注释版（超详细）

> 依据：<引擎真实路径>（N 行）
> 用途：看不懂代码时看这份——每一行上面都有详细中文注释
> 注意：这份是讲解版，不是交付版

## 一、<第一段>（第 X~Y 行）
```<语言>
<带超详细注释的代码>
```

## 二、<第二段>（第 X~Y 行）
...

## 八、速查：这段代码在干什么（一句话版）
| 段 | 行 | 一句话 |

## 九、<语言>语法小抄（本文件用到的）
| 语法 | 含义 | 例子 |

## 待确认
## 源码依据
```

---

## 四、生成还原文件的规则

### 4.1 内容

**直接复制超详细注释 md 里的代码**（注释一起写进去），写到：

```
ue_5.6\<同名路径>\<文件名>
```

### 4.2 必须保留的引擎原文细节

| 项 | 说明 |
|---|---|
| 引擎错字 | `plVatform` / `VisibileForDll` / `CreateModulRules` 等，**照抄不改** |
| 特殊空白 | 带 Tab 的空行、空格缩进的 `{`，**照抄** |
| 重复项 | `AtomicQueue` / `AutoRTFM` 重复出现，**不许去重** |
| 被注释掉的代码 | `// Target.Platform != ...`，**原样保留** |

### 4.3 验证（生成后必做）

用脚本比对「非注释非空代码行」：

```powershell
$our = Get-Content <还原文件> -Encoding UTF8 | Where-Object { $_.Trim() -ne '' -and -not $_.TrimStart().StartsWith('//') }
$eng = Get-Content <引擎原文> -Encoding UTF8 | Where-Object { $_.Trim() -ne '' -and -not $_.TrimStart().StartsWith('//') }
# 逐行比对，差异应只有「行尾英文注释译成中文」
```

**通过标准**：代码行数一致 + 差异只有行尾注释中文化。

---

## 五、硬规则（4 条）

| # | 规则 |
|---|---|
| 1 | **先建超详细注释 md**，再生成还原文件（顺序不能反） |
| 2 | **每行代码上面都要有注释**，不许只写类/方法上方 |
| 3 | **注释要讲清 4 件事**：干什么 / 类型从哪来 / 为什么 / 真实文件在哪 |
| 4 | **生成后必须脚本验证**：代码行数一致 + 差异只有行尾注释 |

---

## 六、样板文件

| 文件 | 说明 |
|---|---|
| `...\Core\答疑文件\09_Core.Build.cs逐行注释版.md` | **超详细注释 md 样板**（804 行 / 36.2KB） |
| `ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs` | **生成的还原文件样板**（669 行 / 31.7KB，代码零差异） |

---

## 七、配套铁律

| 文件 | 管什么 |
|---|---|
| `01_下一步前先检查还原文件.md` | 验收（现在验收 AI 生成的文件） |
| `02_讲义写法_照旧Lyra格式.md` | 两份 md 结构（总 md + 完整代码 md） |
| `03_属性首次出现必须教学并登记.md` | 属性首次出现必教 + 登记台账 |
| `04_讲义就地教_遇到新元素马上讲.md` | 就地教 + 一行一个 |
| **本文件 `05`** | **超详细注释 md + AI 生成文件** |

---

## 源码依据

- 用户 2026-11-16 20:07 原话（第一节）
- `E:\ue5\game_design\code\ue5Learing\AI_Learning_Notes\从零复刻ue5\Engine\Source\Runtime\Core\答疑文件\09_Core.Build.cs逐行注释版.md`（样板）
- `E:\ue5\game_design\code\ue5Learing\ue_5.6\Engine\Source\Runtime\Core\Core.Build.cs`（生成结果）