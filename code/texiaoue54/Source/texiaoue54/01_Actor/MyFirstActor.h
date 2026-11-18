// Copyright Epic Games, Inc. All Rights Reserved.
// 版权声明：UE 模板文件的固定首行。纯注释，删掉也能编译，但保留是引擎惯例

#pragma once
// 头文件保护：保证本文件在一次编译单元里只被展开一次
// 不写 → 同一个 .h 被两处 include 时，类被重复定义 → 报 C2011: 'AMyFirstActor': class type redefinition

#include "CoreMinimal.h"
// 引擎核心基础头（引擎路径：Engine\Source\Runtime\Core\Public\CoreMinimal.h）
// 带进 UE 最常用的基础设施：int32 / FString / TArray / UE_LOG / TEXT() 等

#include "GameFramework/Actor.h"
// AActor 基类的定义所在（引擎路径：Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h）
// 不 include → 报 C2504: 'AActor': 未定义基类

#include "MyFirstActor.generated.h"
// UHT（Unreal Header Tool）在编译前自动生成的反射代码
// ⚠️ 三条铁律：
//   1. 文件名必须与 .h 完全一致（MyFirstActor.h → MyFirstActor.generated.h），大小写也要一致
//   2. 必须是本文件最后一个 #include，后面不能再 include 任何东西
//   3. 只有标了 UCLASS() 的类才需要它

// 前置声明（Forward Declaration）
// 作用：告诉编译器「UStaticMeshComponent 是一个类，具体长什么样我后面再说」
// 为什么不在 .h 里 #include "Components/StaticMeshComponent.h"：
//   · .h 里只用到「指针」，编译器不需要知道类的完整定义（指针大小固定）
//   · 少 include 一个头 → 编译更快、依赖更少
//   · 完整定义放到 .cpp 里 include（见 MyFirstActor.cpp）
// 不写 → 下面的 TObjectPtr<UStaticMeshComponent> 报 C2065: 未声明的标识符
class UStaticMeshComponent;

UCLASS()
// UCLASS()：告诉 UHT「这是一个 UE 反射类，请为它生成反射代码」
// 必须紧贴在 class 声明的正上方，中间不能插别的语句
// 不写 → UHT 完全忽略这个类 → 编辑器里看不到、蓝图里找不到

class TEXIAOUE54_API AMyFirstActor : public AActor
// TEXIAOUE54_API ：模块导出宏 = 模块名全大写 + _API（模块名见 texiaoue54.Build.cs:5）
//                  作用 = 让这个类的符号从 DLL 里导出，别的模块才能链接到它
// AMyFirstActor   ：类名。UE 的 Actor 类必须以 A 开头
// : public AActor ：公有继承 AActor —— 这是「能放进关卡」的唯一前提
{
	GENERATED_BODY()
	// GENERATED_BODY()：UHT 必需宏，必须放在类体的第一行
	// 展开成：构造函数声明、StaticClass()、反射注册、序列化、GC 相关函数等
	// 不写 → 编译报 GENERATED_BODY 相关错误，或类没有构造函数

public:
	// public 区：外部（引擎、蓝图、其他类）可以直接访问

	// 构造函数：创建对象时调用，用来设置默认值
	AMyFirstActor();

protected:
	// protected 区：只有自己和子类能访问，外部不能直接调
	// UE 惯例：生命周期函数（BeginPlay / EndPlay 等）都放 protected

	// BeginPlay：游戏开始（PIE 点 Play）或 Actor 运行时被生成时，调用一次
	virtual void BeginPlay() override;

public:
	// Tick：每帧调用一次，参数 DeltaSeconds = 距上一帧经过的秒数
	// 本课在构造函数里把 Tick 关掉了，所以这个函数实际不会被调用
	virtual void Tick(float DeltaSeconds) override;

	// 静态网格组件
	// UPROPERTY(VisibleAnywhere) ：
	//   · UPROPERTY()  —— 告诉 UHT「这是反射属性」，必须加，否则 GC 会把它当垃圾回收掉
	//   · VisibleAnywhere —— 在编辑器 Details 面板里「可见但不可改」
	//     为什么用 VisibleAnywhere 而不是 EditAnywhere：
	//       组件指针本身不该让用户随便替换（会破坏组件层级），
	//       但用户需要看到「这个 Actor 有哪些组件」
	//   · Category = "MyFirstActor" —— Details 面板里的分组名
	// 不写 UPROPERTY → 组件被 GC 回收 → 运行时崩溃（访问野指针）
	UPROPERTY(VisibleAnywhere, Category = "MyFirstActor")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	// TObjectPtr<UStaticMeshComponent> ：UE5 的「对象指针」包装类型
	//   · 本质还是指针，但能配合 GC 追踪、能自动处理「对象被回收后置空」
	//   · UE5 起官方推荐用 TObjectPtr<T> 替代裸 T*
	//   · 老代码里的 UStaticMeshComponent* 也能编译，但新代码统一用 TObjectPtr
	// MeshComponent ：成员名。UE 惯例：组件成员用「组件类型去掉 U 前缀 + Component」命名
};