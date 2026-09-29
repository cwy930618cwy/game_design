# P04-2 — 建 TenkaichiInputComponent（输入组件：把字典真正用起来）

> **本步定位**：P04 第 2 步。上一步（P04-1）建好了「翻译字典」`TenkaichiInputConfig`，但字典光摆着没用——这一步建一个**输入组件**，负责「查字典 → 把按键绑定到回调函数」。
>
> **玩家此刻**：字典有了（`InputTag.Move` → `IA_Move`），但还没有任何代码去「查这本字典、然后真的接住按键」。角色依然站桩。
>
> **本步拆法**：沿用 `.h`/`.cpp` 分开教、分开建 md：
> - 本文件 = 概念 + 为什么需要输入组件 + 全景（先只建这一个，等你确认）
> - 下一个 `P04-2b_..._h.md` = 写 `.h`
> - 再下一个 `P04-2c_..._cpp.md` = 写 `.cpp`

---

## 一、这一步要解决什么问题（先讲为什么）

上一步我们建了字典 `TenkaichiInputConfig`，里面存着「`InputTag.Move` ↔ `IA_Move`」这种对应关系。

但问题是：**谁来用这本字典？**

- 玩家按 W 键 → 引擎（Enhanced Input）触发了 `IA_Move` 这个 InputAction → 然后呢？**没有代码去接住它。**
- 我们代码里想写「移动逻辑」，但代码不想直接写死 `IA_Move`（那样又绑死按键了，违背上一步讲的「解耦」）。

所以需要**一个中间人**，它干两件事：

1. **查字典**：你给它一个 Tag（如 `InputTag.Move`），它去字典里查出对应的 `IA_Move`。
2. **绑定**：把「`IA_Move` 被触发」这件事，接到一个回调函数（如 `Input_Move`）上。

这个「中间人」就是 **输入组件 `TenkaichiInputComponent`**。

> 类比：字典是「电话本」（名字 → 号码），输入组件是「接线员」——你报名字（Tag），它查电话本找到号码（InputAction），然后帮你拨通（绑定回调）。

---

## 二、为什么 Lyra 要单独建一个「输入组件」，而不是直接写在角色里？

这是 Lyra 输入体系的关键设计，也是这一步的重点：

- 引擎自带一个 `UEnhancedInputComponent`，已经能绑定按键了。
- 但 Lyra **不直接用**引擎的组件，而是**继承它**，建一个自己的 `ULyraInputComponent`。

**为什么？** 因为引擎的 `BindAction` 需要你直接传「`IA_Move` 这个资产」；而 Lyra 的代码里**只想认 Tag，不想认资产**。

所以 Lyra 在自己组件上加了一个 **`BindNativeAction`** 方法，它的签名是「传 Tag，不传资产」：

```cpp
BindNativeAction(InputConfig, InputTag, TriggerEvent, Object, Func, bLogIfNotFound);
//                 ↑字典      ↑Tag     ↑触发时机   ↑对象  ↑回调  ↑找不到要不要报错
```

它内部做了我们第 1 步写的 `FindNativeInputActionForTag`（查字典），查到了再调引擎的 `BindAction`（真正绑定）。

> 一句话：**Lyra 把「查字典」这一步，封装进了自己的输入组件，让上层代码永远只跟 Tag 打交道，不碰具体 InputAction 资产。**

---

## 三、全景：输入组件在整个输入链里的位置

```
按键(W) ──> MappingContext(按键说明，P04步4配) ──> 触发 IA_Move(InputAction)
                                                        │
                                                        ▼
                              TenkaichiInputComponent.BindNativeAction(Tag)
                                                        │ ①查字典 FindNativeInputActionForTag
                                                        ▼
                                              找到 IA_Move
                                                        │ ②BindAction 绑定
                                                        ▼
                                            回调函数 Input_Move(角色移动逻辑)
```

- **上一步（P04-1）** 只建了「字典」（红框里的查询用数据）。
- **这一步（P04-2）** 建「接线员」`TenkaichiInputComponent`，把「查字典 + 绑定」串起来。
- **后续步** 才会真正调用 `BindNativeAction`，把 `Input_Move` 等回调接上（那是角色身上的事）。

---

## 四、这一步的减法（相对 Lyra，我们砍掉了什么）

按铁律「只做减法、说清删了什么」，对照 Lyra 的 `LyraInputComponent.h/.cpp`：

| Lyra 有 | 我们这一步 | 为什么先不教 |
|---------|-----------|-------------|
| `BindAbilityActions`（技能输入绑定） | **删掉** | 技能输入是 GAS 那套，阶段三才用；且依赖 `AbilityInputActions` 数组（P04-1 已删） |
| `AddInputMappings` / `RemoveInputMappings` 的**完整逻辑** | 保留函数壳，但函数体是空的（Lyra 里本来就是空的，只留 `check`） | Lyra 源码里这两个函数体本来就是空实现（注释写着「需要时再自定义」），照抄即可 |
| `.cpp` 里 `#include "Player/LyraLocalPlayer.h"` / `Settings/LyraSettingsLocal.h` | **删掉** | 这两个 include 是 Lyra 本地玩家/设置相关，我们还没建，用不到 |

> 核心保留：`BindNativeAction` 模板函数（**一比一照抄**，只换前缀 `Lyra`→`Tenkaichi`）+ `RemoveBinds`。

---

## 五、这一步会新出现、需要先教的方法（先预告，写代码前逐个讲）

这一步代码里会出现几个**之前没教过**的方法/关键字，写 `.h`/`.cpp` 前我会逐个讲清（铁律 18）：

1. **`template`（模板函数）** —— `BindNativeAction` 为什么是模板？先讲清 C++ 模板是干嘛的。
2. **`BindAction`** —— 引擎 `UEnhancedInputComponent` 提供的真正绑定方法（已核实：`EnhancedInputComponent.h` 第 448 行附近）。
3. **`ETriggerEvent`** —— 触发时机枚举（`Triggered`=按住持续触发 / `Completed`=松开 / `Started`=按下瞬间）。
4. **`check(...)`** —— 引擎的断言宏，参数为空就崩溃（开发期抓 bug 用）。
5. **`RemoveBindingByHandle`** —— 按句柄移除绑定。

> 这些都会在写对应 `.h`/`.cpp` 时，先讲「干嘛 + 源码在哪 + 为什么」，再给代码。

---

## 六、下一步

看完这个总概念，跟我说「懂了 / 下一步」，我再建 `P04-2b`（写 `.h`）。**先不建 `.h`/`.cpp`，等你确认。**