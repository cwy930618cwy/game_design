# `Config\DefaultInput.ini` — 逐行讲解（你自己这份 85 行）

> 对象：`E:\ue5\game_design\code\Tenkaichi\Config\DefaultInput.ini`（**85 行**）
> 原文件**一个字都没动**，这份 md 只是把每行搬到下面、在**上方**加一行 `;` 中文注释。
> 代码行 = 你文件里的原样；`;` 开头的行 = 我加的讲解，你的文件里没有。

---

## 先懂这个文件在干什么

`UInputSettings` 类头写的是 `UCLASS(config=Input, defaultconfig)`（`InputSettings.h:21`），
所以**只从 `DefaultInput.ini` 读** —— 这也是为什么 Lyra 把输入全放这个文件。

整个文件只有 **1 个段**，但干了 4 件事：

| 块 | 行号 | 干什么 |
|---|---|---|
| ① 删旧轴 | L2-L8 | 先删掉引擎默认的 7 个轴配置 |
| ② 加新轴 | L9-L60 | 再加 52 个轴配置（手柄 / 鼠标 / 一堆 VR 手柄） |
| ③ 行为开关 | L61-L84 | 鼠标捕获、全屏、触屏、控制台键… |
| ④ 默认类 | L79-L81 | 用哪个 InputComponent / Input 类 |

🔑 **为什么先删再加**：`-AxisConfig=` 删掉引擎 `BaseInput.ini` 里的默认值，`+AxisConfig=` 再按我们想要的值加回来。不删的话会**重复两条同名轴配置**（删的那条是引擎的、加的那条是我们的）。

---

## 全文逐行注释

```ini
; ─────────────────────────────────────────────
; L1：段头。UInputSettings = 「项目设置 → 引擎 → 输入」
;     类声明：Engine\Classes\GameFramework\InputSettings.h:21
;             UCLASS(config=Input, defaultconfig, MinimalAPI)
;                          ↑ 只从 DefaultInput.ini 读
; ─────────────────────────────────────────────
[/Script/Engine.InputSettings]

; ═══════════════ ① 删掉引擎默认的 7 个轴（L2-L8）═══════════════
; 注意：这些是「删除」不是「注释」，前面的 - 表示从引擎 BaseInput.ini 里剔除这一项
; 写法故意用 0.25 / 1.f 这种短格式 —— 它必须和引擎里那行**字面完全一致**才能删掉

; L2：删掉引擎默认的手柄左摇杆 X 轴（死区 0.25）
-AxisConfig=(AxisKeyName="Gamepad_LeftX",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
; L3：删掉引擎默认的手柄左摇杆 Y 轴
-AxisConfig=(AxisKeyName="Gamepad_LeftY",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
; L4：删掉引擎默认的手柄右摇杆 X 轴
-AxisConfig=(AxisKeyName="Gamepad_RightX",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
; L5：删掉引擎默认的手柄右摇杆 Y 轴
-AxisConfig=(AxisKeyName="Gamepad_RightY",AxisProperties=(DeadZone=0.25,Exponent=1.f,Sensitivity=1.f))
; L6：删掉引擎默认的鼠标 X 轴（灵敏度 0.07）
-AxisConfig=(AxisKeyName="MouseX",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))
; L7：删掉引擎默认的鼠标 Y 轴
-AxisConfig=(AxisKeyName="MouseY",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))
; L8：删掉引擎默认的鼠标 2D 轴
-AxisConfig=(AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.f,Exponent=1.f,Sensitivity=0.07f))

; ═══════════════ ② 重新加回来的轴（L9-L60，共 52 条）═══════════════
; 每条格式：AxisKeyName=哪个物理轴；AxisProperties 里三个参数：
;     DeadZone    死区（摇杆轻微晃动不响应；0.25 = 摇杆推到 25% 才开始有值）
;     Sensitivity 灵敏度（乘数）
;     Exponent    指数曲线（1.0 = 线性；>1 前段更迟钝后段更灵敏）
;     bInvert     是否反向

; ── 常规手柄摇杆（L9-L12）──
; L9：手柄左摇杆 X（左右移动）
+AxisConfig=(AxisKeyName="Gamepad_LeftX",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L10：手柄左摇杆 Y（前后移动）
+AxisConfig=(AxisKeyName="Gamepad_LeftY",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L11：手柄右摇杆 X（左右看）
+AxisConfig=(AxisKeyName="Gamepad_RightX",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L12：手柄右摇杆 Y（上下看）
+AxisConfig=(AxisKeyName="Gamepad_RightY",AxisProperties=(DeadZone=0.250000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── 鼠标（L13-L16）──
; L13：鼠标 X（左右转视角）；死区 0、灵敏度 0.07
+AxisConfig=(AxisKeyName="MouseX",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
; L14：鼠标 Y（上下转视角）；灵敏度 0.07
+AxisConfig=(AxisKeyName="MouseY",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
; L15：鼠标 2D（带轴输入的鼠标设备，一般鼠标用不到）
+AxisConfig=(AxisKeyName="Mouse2D",AxisProperties=(DeadZone=0.000000,Sensitivity=0.070000,Exponent=1.000000,bInvert=False))
; L16：鼠标滚轮（映射成轴，可用来切武器/缩放）
+AxisConfig=(AxisKeyName="MouseWheelAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── 手柄扳机 / 特殊键（L17-L20）──
; L17：手柄左扳机（LT/L2）
+AxisConfig=(AxisKeyName="Gamepad_LeftTriggerAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L18：手柄右扳机（RT/R2）
+AxisConfig=(AxisKeyName="Gamepad_RightTriggerAxis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L19：手柄特殊键左区 X（老式手柄的方向轴）
+AxisConfig=(AxisKeyName="Gamepad_Special_Left_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L20：手柄特殊键左区 Y
+AxisConfig=(AxisKeyName="Gamepad_Special_Left_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── HTC Vive 手柄（L21-L26）★ VR 用，PC 键鼠开发完全用不到 ──
; L21：Vive 左扳机
+AxisConfig=(AxisKeyName="Vive_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L22：Vive 左触摸板 X
+AxisConfig=(AxisKeyName="Vive_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L23：Vive 左触摸板 Y
+AxisConfig=(AxisKeyName="Vive_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L24：Vive 右扳机
+AxisConfig=(AxisKeyName="Vive_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L25：Vive 右触摸板 X
+AxisConfig=(AxisKeyName="Vive_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L26：Vive 右触摸板 Y
+AxisConfig=(AxisKeyName="Vive_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── Windows Mixed Reality 手柄（L27-L36）★ VR 用 ──
; L27：MR 左扳机
+AxisConfig=(AxisKeyName="MixedReality_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L28：MR 左摇杆 X
+AxisConfig=(AxisKeyName="MixedReality_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L29：MR 左摇杆 Y
+AxisConfig=(AxisKeyName="MixedReality_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L30：MR 左触摸板 X
+AxisConfig=(AxisKeyName="MixedReality_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L31：MR 左触摸板 Y
+AxisConfig=(AxisKeyName="MixedReality_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L32：MR 右扳机
+AxisConfig=(AxisKeyName="MixedReality_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L33：MR 右摇杆 X
+AxisConfig=(AxisKeyName="MixedReality_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L34：MR 右摇杆 Y
+AxisConfig=(AxisKeyName="MixedReality_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L35：MR 右触摸板 X
+AxisConfig=(AxisKeyName="MixedReality_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L36：MR 右触摸板 Y
+AxisConfig=(AxisKeyName="MixedReality_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── Oculus Touch 手柄（L37-L44）★ VR 用 ──
; L37：Oculus 左握把（Grip）
+AxisConfig=(AxisKeyName="OculusTouch_Left_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L38：Oculus 左扳机
+AxisConfig=(AxisKeyName="OculusTouch_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L39：Oculus 左摇杆 X
+AxisConfig=(AxisKeyName="OculusTouch_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L40：Oculus 左摇杆 Y
+AxisConfig=(AxisKeyName="OculusTouch_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L41：Oculus 右握把
+AxisConfig=(AxisKeyName="OculusTouch_Right_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L42：Oculus 右扳机
+AxisConfig=(AxisKeyName="OculusTouch_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L43：Oculus 右摇杆 X
+AxisConfig=(AxisKeyName="OculusTouch_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L44：Oculus 右摇杆 Y
+AxisConfig=(AxisKeyName="OculusTouch_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ── Valve Index 手柄（L45-L60）★ VR 用，最长的一组 ──
; L45：Index 左握把
+AxisConfig=(AxisKeyName="ValveIndex_Left_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L46：Index 左握把力度（Force 传感）
+AxisConfig=(AxisKeyName="ValveIndex_Left_Grip_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L47：Index 左扳机
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L48：Index 左摇杆 X
+AxisConfig=(AxisKeyName="ValveIndex_Left_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L49：Index 左摇杆 Y
+AxisConfig=(AxisKeyName="ValveIndex_Left_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L50：Index 左触摸板 X
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L51：Index 左触摸板 Y
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L52：Index 左触摸板力度
+AxisConfig=(AxisKeyName="ValveIndex_Left_Trackpad_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L53：Index 右握把
+AxisConfig=(AxisKeyName="ValveIndex_Right_Grip_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L54：Index 右握把力度
+AxisConfig=(AxisKeyName="ValveIndex_Right_Grip_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L55：Index 右扳机
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trigger_Axis",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L56：Index 右摇杆 X
+AxisConfig=(AxisKeyName="ValveIndex_Right_Thumbstick_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L57：Index 右摇杆 Y
+AxisConfig=(AxisKeyName="ValveIndex_Right_Thumbstick_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L58：Index 右触摸板 X
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_X",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L59：Index 右触摸板 Y
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_Y",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))
; L60：Index 右触摸板力度
+AxisConfig=(AxisKeyName="ValveIndex_Right_Trackpad_Force",AxisProperties=(DeadZone=0.000000,Sensitivity=1.000000,Exponent=1.000000,bInvert=False))

; ═══════════════ ③ 行为开关（L61-L78）═══════════════

; L61：Alt+Enter 切换全屏 = True
bAltEnterTogglesFullscreen=True
; L62：F11 切换全屏 = True
bF11TogglesFullscreen=True
; L63：是否用鼠标模拟触屏 = False（False = 鼠标就是鼠标，不当手指）
bUseMouseForTouch=False
; L64：鼠标平滑 = True（对鼠标输入做平滑，移动更顺但有一点延迟）
bEnableMouseSmoothing=True
; L65：FOV 缩放 = True（视野变化时对输入做补偿）
bEnableFOVScaling=True
; L66：启动时捕获鼠标 = True（游戏窗口获得焦点就把鼠标锁在窗口内）
bCaptureMouseOnLaunch=True
; L67：启用旧版输入缩放 = True（保留 UE4 时代的轴缩放行为）
bEnableLegacyInputScales=True
; L68：启用体感控制 = True（手机陀螺仪/加速度）
bEnableMotionControls=True
; L69：是否按平台用户过滤输入 = False（False = 不按登录用户区分输入设备）
bFilterInputByPlatformUser=False
; L70：视口失去焦点时清空按下的键 = True（防止切出去再切回来还在"按住"）
bShouldFlushPressedKeysOnViewportFocusLost=True
; L71：总是显示触屏界面 = False
bAlwaysShowTouchInterface=False
; L72：四指点击显示控制台 = True（手机上四指点击调出控制台）
bShowConsoleOnFourFingerTap=True
; L73：启用手势识别 = False
bEnableGestureRecognizer=False
; L74：自动更正 = False（触屏输入法自动纠错）
bUseAutocorrect=False
; L75：视口鼠标捕获模式 = CapturePermanently_IncludingInitialMouseDown
;      永久捕获（含第一次按下鼠标时也不释放）
DefaultViewportMouseCaptureMode=CapturePermanently_IncludingInitialMouseDown
; L76：视口鼠标锁定模式 = LockOnCapture（一旦捕获就锁定在窗口中心）
DefaultViewportMouseLockMode=LockOnCapture
; L77：视野缩放系数 = 0.011110（配合 L65 的 bEnableFOVScaling）
FOVScale=0.011110
; L78：双击判定时间 = 0.2 秒
DoubleClickTime=0.200000

; ═══════════════ ④ 默认输入类（L79-L84）═══════════════

; L79：默认 PlayerInput 类 = 增强输入的 UEnhancedPlayerInput（不是旧版 UPlayerInput）
;      增强输入（Enhanced Input）的运行时入口点
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
; L80：默认 InputComponent 类 = 增强输入的 UEnhancedInputComponent
;      ⚠️ Lyra 这里是 /Script/LyraGame.LyraInputComponent（Lyra 自己的子类，带"按住/松开"绑定）
;         我们在 S13 建完 ULyraInputComponent 后要改成它
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
; L81：默认触屏界面 = 引擎自带的虚拟摇杆（手机端显示两个虚拟摇杆）
;      ⚠️ Lyra 这里是 None（Lyra 用 CommonUI 自己做触屏 UI，不用引擎摇杆）
DefaultTouchInterface=/Engine/MobileResources/HUD/DefaultVirtualJoysticks.DefaultVirtualJoysticks

; L82：删掉引擎默认的控制台开关键 Tilde（即 ~ 键）
-ConsoleKeys=Tilde
; L83：再把 Tilde（~）加回来 —— 删了又加，效果等于保留
+ConsoleKeys=Tilde
; L84：额外加 Caret（^，即 Shift+6）也能开控制台
+ConsoleKeys=Caret
; L85：文件最后一个空行
```

---

## 这个文件的 4 个「必须知道」

### 1️⃣ L2-L8 的删除行**必须字面一模一样**

`-AxisConfig=...` 的删除是**按字面匹配**的：`0.25` 不能写成 `0.250000`，`1.f` 不能写成 `1.000000`。
因为它删的是引擎 `Engine\Config\BaseInput.ini` 里那行原文。写错了 → 删不掉 → 最终会有两条同名轴。

> ✅ 你这份是对的（照模板生成的），不用动。

### 2️⃣ 52 条轴配置里，**42 条是 VR 手柄，PC 开发完全用不到**

| 手柄 | 行 | 条数 |
|---|---|---|
| 常规手柄 + 鼠标 + 扳机 | L9-L20 | 12 |
| HTC Vive | L21-L26 | 6 |
| Windows MR | L27-L36 | 10 |
| Oculus Touch | L37-L44 | 8 |
| Valve Index | L45-L60 | 18 |

Lyra 的 `DefaultInput.ini:13-23` **只留 10 条**（手柄 4 + 鼠标 3 + 滚轮 + 2 个扳机），**VR 那 42 条全删了**。

> 📌 **S13（Input 阶段）会把整个文件换成 Lyra 的 51 行版**，那时 VR 那些就没了。现在不用管。

### 3️⃣ 有两处与 Lyra 不一致，S13 要改

| 行 | 我们 | Lyra | 影响 |
|---|---|---|---|
| L80 `DefaultInputComponentClass` | `EnhancedInput.EnhancedInputComponent` | `LyraGame.LyraInputComponent` | Lyra 的子类提供「按住 / 松开」触发、输入配置注入；**必须等 S13 建完类才能改** |
| L81 `DefaultTouchInterface` | `DefaultVirtualJoysticks` | `None` | 手机端会浮出引擎原生虚拟摇杆 |

### 4️⃣ `bFilterInputByPlatformUser` 也不同

| 行 | 我们 | Lyra |
|---|---|---|
| L69 `bFilterInputByPlatformUser` | `False` | `True` |

Lyra 设 `True`：多人同屏时，每个玩家只响应自己那份输入设备。

---

## Lyra 的 `DefaultInput.ini` 多出 2 个段（我们没有）

| Lyra 段 | Lyra 行 | 作用 | 补的时机 |
|---|---|---|---|
| `[/Script/Engine.PlayerInput]` | 1-10 | **8 条调试快捷键**：`Insert` 传送 / `Delete` 切调试相机 / `Home` 显示调试 / `K` 自爆 / `PageUp/Down` 切目标 / `NumPad4` 取消技能 | **S12**（有 PlayerController 才有意义） |
| `[/Script/CommonUI.CommonUIInputSettings]` | 36-41 | UI 返回键映射（`Escape` / `Gamepad_Special_Right` → `UI.Action.Escape`）+ 模拟光标加速曲线 | **S18**（UI 阶段） |
| `[/Script/OpenXRInput.OpenXRInputSettings]` | 43-45 | `bUseEnhancedInput=True` | S06（留着无害） |
| `[/Script/EnhancedInput.EnhancedInputDeveloperSettings]` | 47-50 | `bEnableUserSettings=True` + `UserSettingsClass=LyraInputUserSettings` + `DefaultPlayerMappableKeyProfileClass=LyraPlayerMappableKeyProfile`（**用户自定义按键**功能） | **S13** |

---

## 一句话

**85 行 = 1 个段 + 4 件事：删 7 个旧轴（L2-8）、加 52 个新轴（L9-60，其中 42 条是 VR）、20 个行为开关（L61-78）、3 个默认类（L79-81）。现在不用改任何一行；S13 会整体换成 Lyra 的 51 行版（去掉 VR、改 `LyraInputComponent`、补 8 条调试快捷键）。**

---

## 源码依据

- `E:\ue5\game_design\code\Tenkaichi\Config\DefaultInput.ini:1-85`（你这份全文，逐行对应上面注释）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\InputSettings.h:21`（`UCLASS(config=Input, defaultconfig)`）
- `D:\ue5\Epic Games\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\PlayerInput.h:407`（`UCLASS(config=Input, transient)`，即 `[/Script/Engine.PlayerInput]` 那个段）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:1-51`（Lyra 全文：4 段 + 10 条轴）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:1-10`（8 条调试快捷键）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:33`（`DefaultInputComponentClass=/Script/LyraGame.LyraInputComponent`）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:36-41`（CommonUI 输入设置）
- `E:\ue5\LyraStarterGame\Config\DefaultInput.ini:47-50`（EnhancedInput 用户设置）