// Copyright Epic Games, Inc. All Rights Reserved.
// 版权声明，同 .h

#include "MyFirstActor.h"
// IWYU 规则：.cpp 的第一个 include 必须是自己的 .h
// 不写 → 报 C2039 / C2065：AMyFirstActor 未定义

#include "Components/StaticMeshComponent.h"
// UStaticMeshComponent 的完整定义（引擎路径：Engine\Source\Runtime\Engine\Classes\Components\StaticMeshComponent.h）
// 为什么在 .cpp 里 include 而不是 .h 里：
//   · .h 里只用了指针（前置声明够了）
//   · .cpp 里要调 CreateDefaultSubobject<UStaticMeshComponent>() 和 SetStaticMesh()，
//     必须知道类的完整定义（大小、成员函数签名）
// 不写 → 报 C2027: 使用了未定义类型 "UStaticMeshComponent"

#include "UObject/ConstructorHelpers.h"
// ConstructorHelpers::FObjectFinder 的定义所在
// （引擎路径：Engine\Source\Runtime\CoreUObject\Public\UObject\ConstructorHelpers.h）
// 这个工具类专门用于「在构造函数里按路径加载资源」
// 不写 → 报 C2653: "ConstructorHelpers": 不是类或命名空间名

AMyFirstActor::AMyFirstActor()
// 类外定义构造函数：格式是「类名::类名()」
{
	// PrimaryActorTick 是 AActor 的成员（类型 FActorTickFunction），控制「这个 Actor 要不要 Tick」
	// bCanEverTick = false → 引擎永远不会为这个 Actor 调用 Tick()
	// 好处：省掉每帧的函数调用开销
	PrimaryActorTick.bCanEverTick = false;

	// 创建组件
	// CreateDefaultSubobject<T>(名字)
	//   · 只能在构造函数里调用（引擎内部有断言检查，别处调用会崩）
	//   · 作用：创建一个「默认子对象」，它会随 Actor 一起被创建、序列化、复制
	//   · 模板参数 <UStaticMeshComponent> ：要创建什么类型的组件
	//   · 参数 TEXT("MeshComponent") ：子对象名，必须唯一
	//     这个名字会出现在：
	//       · 蓝图里的组件列表
	//       · 存档 / 网络复制的标识
	//       · 报错信息里
	//     改名会导致已有蓝图/存档找不到组件 → 所以定下来就别改
	//   · 返回值：指向新组件的指针
	// 不调用 → MeshComponent 是空指针 → 后面 SetStaticMesh 直接崩溃
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));

	// 设为根组件
	// RootComponent 是 AActor 的成员（AActor.h:881），类型 TObjectPtr<USceneComponent>
	// 把 MeshComponent 赋给它 = 「这个组件就是整个 Actor 的变换基准」
	// 为什么可以直接赋值（UStaticMeshComponent* → USceneComponent*）：
	//   继承链 UStaticMeshComponent → UMeshComponent → UPrimitiveComponent → USceneComponent
	//   子类指针可以隐式转成父类指针
	// 不设置 → Actor 没有根组件 → 关卡里无法选中/移动，组件也不会被渲染
	RootComponent = MeshComponent;

	// 加载 Cube 网格资源
	// ConstructorHelpers::FObjectFinder<UStaticMesh>
	//   · 专门用于「构造函数里按路径加载资源」的工具
	//   · 内部调用 StaticLoadObject，只在「第一次构造」时真正加载，之后走缓存
	//   · 必须声明为 static：
	//       原因 —— 构造函数会被调用很多次（编辑器加载、蓝图编译、CDO 创建…），
	//               static 保证「只加载一次」，后续直接复用
	//       不写 static → 每次构造都重新加载 → 编辑器启动变慢、内存浪费
	//   · 参数 TEXT("/Engine/BasicShapes/Cube.Cube") ：
	//       格式 = 「包路径.对象名」
	//       /Engine/BasicShapes/Cube  = 包路径（.uasset 文件位置，不含扩展名）
	//       .Cube                     = 包里的对象名（通常与文件名相同）
	//       完整磁盘路径 = D:\ue5\Epic Games\UE_5.4\Engine\Content\BasicShapes\Cube.uasset
	//       ⚠️ /Engine/ 是引擎内容目录的虚拟根，不是磁盘路径
	//   · 为什么用引擎自带 Cube 而不是自己导入：
	//       本课目标是「验证组件能渲染」，不引入额外资产依赖
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	// Succeeded() ：判断资源是否加载成功（内部就是 return !!Object;）
	// 为什么要判断：
	//   · 路径写错 / 引擎内容被裁剪 → 加载失败
	//   · 不判断直接 SetStaticMesh(CubeAsset.Object) → 传 nullptr → 组件变成空网格（不崩溃但看不见）
	//   · 判断后可以打日志，方便定位问题
	if (CubeAsset.Succeeded())
	{
		// SetStaticMesh(UStaticMesh*) ：给组件指定要渲染的网格
		// （声明在 StaticMeshComponent.h:363）
		// CubeAsset.Object ：FObjectFinder 的成员，类型 TObjectPtr<UStaticMesh>
		// 调用后：组件有了几何数据 → 关卡里就能看到方块
		MeshComponent->SetStaticMesh(CubeAsset.Object);
	}
}

void AMyFirstActor::BeginPlay()
{
	// Super = 父类 AActor 的指针别名（由 GENERATED_BODY() 生成）
	// Super::BeginPlay() 必须第一个调用：
	//   AActor::BeginPlay() 里做了 bHasBegunPlay = true、通知所有组件 BeginPlay、
	//   触发蓝图 BeginPlay 事件等初始化
	// 漏掉 → 组件不初始化、蓝图事件不触发、后续出现各种诡异 bug
	Super::BeginPlay();

	// UE_LOG(日志分类, 级别, 格式串, 参数...)
	//   LogTemp  ：临时日志分类（引擎内置，不用自己声明）
	//   Warning  ：级别。Log / Warning / Error / Fatal，Warning 默认显示
	//   TEXT("...")：把普通字符串转成 UE 的宽字符 TCHAR
	//   %s       ：格式化占位符，对应 const TCHAR*
	//   *GetName()：GetName() 返回 FString，* 取出内部 const TCHAR*
	UE_LOG(LogTemp, Warning, TEXT("AMyFirstActor::BeginPlay 已执行，Actor 名称：%s"), *GetName());
}

void AMyFirstActor::Tick(float DeltaSeconds)
{
	// 调父类 Tick。AActor::Tick 目前是空实现，但保持惯例
	// DeltaSeconds 本课没用到，01.4 会用它做「每秒转 90 度」这类与帧率无关的旋转
	Super::Tick(DeltaSeconds);
}