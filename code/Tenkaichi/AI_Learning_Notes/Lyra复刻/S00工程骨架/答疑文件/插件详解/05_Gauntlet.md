# 05 — `Gauntlet` 详解

> 位置：`.uproject` 的 `Plugins` 数组第 **5** 项
> 来源：引擎插件（`D:\ue5\Epic Games\UE_5.4\Engine\Plugins\...`）
> 当前状态：`"Enabled": true`，`EnabledByDefault` = `False`
> 分类：`Testing`
> 官方描述：Provides a helper class for creating and managing tests in your game
> 模块：`Gauntlet`(Runtime)

---

## 一、一句话

无人值守跑游戏并判成败的自动化集成测试框架（两层：C++ 控制器 + C# 测试节点）。

## 二、它到底做什么

C++ 侧 `UGauntletTestController` 做「游戏内的测试导演」（`OnInit` / `OnPreMapChange` / `OnTick` / `EndTest(ExitCode)`）；C# 侧 `BaseTestNode` / `EpicGameTestNode` 负责启动进程、传参、收日志、判成败。外层通过 `RunUAT.bat RunUnreal -test=XXX` 调用，游戏进程内由 `-gauntlet=XXX` 参数触发控制器创建。

## 三、有它 vs 没它

**不传 `-gauntlet=` 就完全空转**（`GauntletModule.cpp:173` 的 `if (ControllerString.Len())` 整段跳过）。Runtime 模块，打包体积略增（几十 KB）。

## 四、Lyra 怎么用它

`.uproject` + `LyraGame.Build.cs` 私有依赖（因为 `Source\LyraGame\Tests\LyraTestControllerBootTest.h` include 了它）。

**Lyra 源码里的引用位置**：

- `LyraStarterGame.uproject` (x1)
- `Source\LyraGame\LyraGame.Build.cs` (x1)

## 五、本复刻线什么时候碰它

S21 复刻 `Tests\LyraTestControllerBootTest.h/.cpp` 时才会碰它；搭 CI 时用。

## 六、注意

不是单元测试框架。单元测试用 `IMPLEMENT_SIMPLE_AUTOMATION_TEST`；Gauntlet 测的是「打包后的游戏能不能跑起来」。

---

## 源码依据

- `D:\ue5\Epic Games\UE_5.4\Engine\Plugins\Experimental\Gauntlet\Gauntlet.uplugin`（`.uplugin` 原文）
- `E:\ue5\game_design\code\Tenkaichi\Tenkaichi.uproject` 第 5 项
- `E:\ue5\LyraStarterGame\LyraStarterGame.uproject`（Lyra 原版 76 项对照）
