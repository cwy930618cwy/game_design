// 版权所有 Epic Games, Inc. 保留所有权利。


// 头文件防重复包含：同一编译单元里多次 include 只展开一次。
#pragma once

// 引入基础类型层：uint8 / uint32 / uint64 / int32 / TCHAR 都来自这里。
#include "CoreTypes.h"
// 引入 ENUM_CLASS_FLAGS 宏：给 enum class 生成位运算和 EnumHasAllFlags 等工具。
#include "Misc/EnumClassFlags.h"

// 若外部已定义过 PF_MAX（旧版头残留），先取消定义，避免下面枚举重定义冲突。
#if defined(PF_MAX)
// 取消 PF_MAX 的旧定义。
#undef PF_MAX
// 条件编译结束。
#endif

// 前置声明：像素格式能力位标志（完整定义在本文件后面），先声明是为了让 FPixelFormatInfo 能引用它。
enum class EPixelFormatCapabilities : uint32;

// 像素格式枚举：底层类型 uint8（最多 256 种），每个值代表一种显存里的像素排布方式。
enum EPixelFormat : uint8
// 枚举体开始。
{
// 0 号：未知格式，占位用。
	PF_Unknown              =0,
// 1 号：四通道 32 位浮点（RGBA 共 128 位 / 16 字节），HDR 最高精度。
	PF_A32B32G32R32F        =1,
// 2 号：8 位 BGRA 各一通道，4 字节/像素，最常见的 32 位无压缩格式。
	PF_B8G8R8A8             =2,
// 3 号：8 位灰度（Gray 不是 Green），实际常用红色格式并把 R 复制到 RGB。
	PF_G8                   =3, // G8 表示灰度（Gray/Grey），不是绿色（Green）；实际通常用红色格式并把 R 复制到 RGB
// 4 号：16 位灰度，同 G8 但精度更高。
	PF_G16                  =4, // G16 和 G8 一样表示灰度（Gray/Grey）
// 5 号：DXT1 块压缩（BC1），4x4 块 8 字节。
	PF_DXT1                 =5,
// 6 号：DXT3 块压缩（BC2），4x4 块 16 字节，显式 4 位 alpha。
	PF_DXT3                 =6,
// 7 号：DXT5 块压缩（BC3），4x4 块 16 字节，插值 alpha。
	PF_DXT5                 =7,
// 8 号：YUV 4:2:2 打包格式，视频解码用。
	PF_UYVY                 =8,
// 9 号：三通道浮点，通常是 R11G11B10（不是 16F），具体随平台变。
	PF_FloatRGB             =9,  // FloatRGB 等于 PF_FloatR11G11B10，通常不是 16F，但随平台变化
// 10 号：四通道 16 位浮点（RGBA16F），HDR 渲染目标常用。
	PF_FloatRGBA            =10, // RGBA16F
// 11 号：深度 + 模板缓冲，具体位宽由平台决定。
	PF_DepthStencil         =11,
// 12 号：阴影贴图专用深度格式。
	PF_ShadowDepth          =12,
// 13 号：单通道 32 位浮点。
	PF_R32_FLOAT            =13,
// 14 号：双通道 16 位无符号归一化。
	PF_G16R16               =14,
// 15 号：双通道 16 位浮点。
	PF_G16R16F              =15,
// 16 号：双通道 16 位浮点，硬件可线性过滤。
	PF_G16R16F_FILTER       =16,
// 17 号：双通道 32 位浮点。
	PF_G32R32F              =17,
// 18 号：10:10:10:2 位打包，HDR 显示常用。
	PF_A2B10G10R10          =18,
// 19 号：四通道 16 位无符号归一化。
	PF_A16B16G16R16         =19,
// 20 号：24 位深度。
	PF_D24                  =20,
// 21 号：单通道 16 位浮点。
	PF_R16F                 =21,
// 22 号：单通道 16 位浮点，硬件可线性过滤。
	PF_R16F_FILTER          =22,
// 23 号：BC5 块压缩，双通道法线贴图常用。
	PF_BC5                  =23,
// 24 号：双通道 8 位有符号归一化（旧法线格式）。
	PF_V8U8                 =24,
// 25 号：1 位 alpha 遮罩。
	PF_A1                   =25,
// 26 号：11:11:10 位浮点打包，无 alpha 的 HDR。
	PF_FloatR11G11B10       =26,
// 27 号：8 位 alpha。
	PF_A8                   =27,
// 28 号：单通道 32 位无符号整数。
	PF_R32_UINT             =28,
// 29 号：单通道 32 位有符号整数。
	PF_R32_SINT             =29,
// 30 号：PowerVR 2 位块压缩（iOS 老设备）。
	PF_PVRTC2               =30,
// 31 号：PowerVR 4 位块压缩。
	PF_PVRTC4               =31,
// 32 号：单通道 16 位无符号整数。
	PF_R16_UINT             =32,
// 33 号：单通道 16 位有符号整数。
	PF_R16_SINT             =33,
// 34 号：四通道 16 位无符号整数。
	PF_R16G16B16A16_UINT    =34,
// 35 号：四通道 16 位有符号整数。
	PF_R16G16B16A16_SINT    =35,
// 36 号：5:6:5 位打包，16 位无 alpha。
	PF_R5G6B5_UNORM         =36,
// 37 号：8 位 RGBA 各一通道，4 字节/像素（通道序与 B8G8R8A8 相反）。
	PF_R8G8B8A8             =37,
// 38 号：旧版 32 位格式，只用于兼容加载，禁止新代码使用。
	PF_A8R8G8B8				=38,	// 只用于旧版兼容加载；禁止使用！
// 39 号：BC4 块压缩，单通道（灰度 / 粗糙度）。
	PF_BC4					=39,
// 40 号：双通道 8 位无符号归一化。
	PF_R8G8                 =40,	
// 41 号：ATC 压缩（Adreno 老设备），已不支持。
	PF_ATC_RGB				=41,	// 不支持的格式
// 42 号：ATC 带显式 alpha，已不支持。
	PF_ATC_RGBA_E			=42,	// 不支持的格式
// 43 号：ATC 带插值 alpha，已不支持。
	PF_ATC_RGBA_I			=43,	// 不支持的格式
// 44 号：24 位深度 + 8 位模板的别名视图，只用于建 SRV 读模板，不能建纹理。
	PF_X24_G8				=44,	// 用于创建 SRV，把深度模板缓冲别名成可读模板的视图；不要用它创建纹理。
// 45 号：ETC1 压缩（Android 老设备），已不支持。
	PF_ETC1					=45,	// 不支持的格式
// 46 号：ETC2 三通道压缩。
	PF_ETC2_RGB				=46,
// 47 号：ETC2 四通道压缩。
	PF_ETC2_RGBA			=47,
// 48 号：四通道 32 位无符号整数。
	PF_R32G32B32A32_UINT	=48,
// 49 号：双通道 16 位无符号整数。
	PF_R16G16_UINT			=49,
// 50 号：ASTC 4x4 块压缩，8.00 位/像素。
	PF_ASTC_4x4             =50,	// 8.00 位/像素
// 51 号：ASTC 6x6，3.56 位/像素。
	PF_ASTC_6x6             =51,	// 3.56 位/像素
// 52 号：ASTC 8x8，2.00 位/像素。
	PF_ASTC_8x8             =52,	// 2.00 位/像素
// 53 号：ASTC 10x10，1.28 位/像素。
	PF_ASTC_10x10           =53,	// 1.28 位/像素
// 54 号：ASTC 12x12，0.89 位/像素。
	PF_ASTC_12x12           =54,	// 0.89 位/像素
// 55 号：BC6H 块压缩，HDR 用。
	PF_BC6H					=55,
// 56 号：BC7 块压缩，高质量 RGBA。
	PF_BC7					=56,
// 57 号：单通道 8 位无符号整数。
	PF_R8_UINT				=57,
// 58 号：8 位亮度（Luminance）。
	PF_L8					=58,
// 59 号：双通道 + 8 位（X 为填充位）。
	PF_XGXR8				=59,
// 60 号：四通道 8 位无符号整数。
	PF_R8G8B8A8_UINT		=60,
// 61 号：四通道 8 位有符号归一化。
	PF_R8G8B8A8_SNORM		=61,
// 62 号：四通道 16 位无符号归一化。
	PF_R16G16B16A16_UNORM	=62,
// 63 号：四通道 16 位有符号归一化。
	PF_R16G16B16A16_SNORM	=63,
// 64 号：平台自定义 HDR 格式 0。
	PF_PLATFORM_HDR_0		=64,
// 65 号：平台自定义 HDR 格式 1（保留）。
	PF_PLATFORM_HDR_1		=65,	// 保留。
// 66 号：平台自定义 HDR 格式 2（保留）。
	PF_PLATFORM_HDR_2		=66,	// 保留。
// 67 号：NV12 视频格式（Y 平面 + UV 交错）。
	PF_NV12					=67,
// 68 号：双通道 32 位无符号整数。
	PF_R32G32_UINT          =68,
// 69 号：ETC2 单通道 11 位 EAC 压缩。
	PF_ETC2_R11_EAC			=69,
// 70 号：ETC2 双通道 11 位 EAC 压缩。
	PF_ETC2_RG11_EAC		=70,
// 71 号：单通道 8 位无符号归一化。
	PF_R8		            =71,
// 72 号：5:5:5:1 位打包。
	PF_B5G5R5A1_UNORM       =72,
// 73 号：ASTC 4x4 的 HDR 变体。
	PF_ASTC_4x4_HDR         =73,	
// 74 号：ASTC 6x6 的 HDR 变体。
	PF_ASTC_6x6_HDR         =74,	
// 75 号：ASTC 8x8 的 HDR 变体。
	PF_ASTC_8x8_HDR         =75,	
// 76 号：ASTC 10x10 的 HDR 变体。
	PF_ASTC_10x10_HDR       =76,	
// 77 号：ASTC 12x12 的 HDR 变体。
	PF_ASTC_12x12_HDR       =77,
// 78 号：双通道 16 位有符号归一化。
	PF_G16R16_SNORM			=78,
// 79 号：双通道 8 位无符号整数。
	PF_R8G8_UINT			=79,
// 80 号：三通道 32 位无符号整数。
	PF_R32G32B32_UINT		=80,
// 81 号：三通道 32 位有符号整数。
	PF_R32G32B32_SINT		=81,
// 82 号：三通道 32 位浮点。
	PF_R32G32B32F			=82,
// 83 号：单通道 8 位有符号整数。
	PF_R8_SINT				=83,	
// 84 号：单通道 64 位无符号整数。
	PF_R64_UINT				=84,
// 85 号：9:9:9 位尾数 + 5 位共享指数，HDR 打包。
	PF_R9G9B9EXP5			=85,
// 86 号：P010 视频格式（10 位 YUV）。
	PF_P010					=86,
// 87 号：ASTC 4x4 的 RG 双通道变体（存 LA 端点，需纹理 swizzle 支持）。
	PF_ASTC_4x4_NORM_RG		=87, // RG 格式存在 LA 端点里以获得更高精度（需要 RHI 支持纹理 swizzle）
// 88 号：ASTC 6x6 的 RG 变体。
	PF_ASTC_6x6_NORM_RG		=88,	
// 89 号：ASTC 8x8 的 RG 变体。
	PF_ASTC_8x8_NORM_RG		=89,	
// 90 号：ASTC 10x10 的 RG 变体。
	PF_ASTC_10x10_NORM_RG	=90,	
// 91 号：ASTC 12x12 的 RG 变体。
	PF_ASTC_12x12_NORM_RG	=91,	
// 92 号：双通道 16 位有符号整数。
	PF_R16G16_SINT			=92,
// 93 号：三通道 8 位无符号归一化。
	PF_R8G8B8				=93,
// 94 号：枚举总数哨兵值，不是真实格式；数组大小用它。
	PF_MAX					=94,
// 枚举体结束。
};
// 遍历宏：把枚举里每个值都套一遍 op(...)，供反射 / 序列化 / 调试输出统一遍历（新增格式必须同步加一行）。
#define FOREACH_ENUM_EPIXELFORMAT(op) \
	op(PF_Unknown) \
	op(PF_A32B32G32R32F) \
	op(PF_B8G8R8A8) \
	op(PF_G8) \
	op(PF_G16) \
	op(PF_DXT1) \
	op(PF_DXT3) \
	op(PF_DXT5) \
	op(PF_UYVY) \
	op(PF_FloatRGB) \
	op(PF_FloatRGBA) \
	op(PF_DepthStencil) \
	op(PF_ShadowDepth) \
	op(PF_R32_FLOAT) \
	op(PF_G16R16) \
	op(PF_G16R16F) \
	op(PF_G16R16F_FILTER) \
	op(PF_G32R32F) \
	op(PF_A2B10G10R10) \
	op(PF_A16B16G16R16) \
	op(PF_D24) \
	op(PF_R16F) \
	op(PF_R16F_FILTER) \
	op(PF_BC5) \
	op(PF_V8U8) \
	op(PF_A1) \
	op(PF_FloatR11G11B10) \
	op(PF_A8) \
	op(PF_R32_UINT) \
	op(PF_R32_SINT) \
	op(PF_PVRTC2) \
	op(PF_PVRTC4) \
	op(PF_R16_UINT) \
	op(PF_R16_SINT) \
	op(PF_R16G16B16A16_UINT) \
	op(PF_R16G16B16A16_SINT) \
	op(PF_R5G6B5_UNORM) \
	op(PF_R8G8B8A8) \
	op(PF_A8R8G8B8) \
	op(PF_BC4) \
	op(PF_R8G8) \
	op(PF_ATC_RGB) \
	op(PF_ATC_RGBA_E) \
	op(PF_ATC_RGBA_I) \
	op(PF_X24_G8) \
	op(PF_ETC1) \
	op(PF_ETC2_RGB) \
	op(PF_ETC2_RGBA) \
	op(PF_R32G32B32A32_UINT) \
	op(PF_R16G16_UINT) \
	op(PF_ASTC_4x4) \
	op(PF_ASTC_6x6) \
	op(PF_ASTC_8x8) \
	op(PF_ASTC_10x10) \
	op(PF_ASTC_12x12) \
	op(PF_BC6H) \
	op(PF_BC7) \
	op(PF_R8_UINT) \
	op(PF_L8) \
	op(PF_XGXR8) \
	op(PF_R8G8B8A8_UINT) \
	op(PF_R8G8B8A8_SNORM) \
	op(PF_R16G16B16A16_UNORM) \
	op(PF_R16G16B16A16_SNORM) \
	op(PF_PLATFORM_HDR_0) \
	op(PF_PLATFORM_HDR_1) \
	op(PF_PLATFORM_HDR_2) \
	op(PF_NV12) \
	op(PF_R32G32_UINT) \
	op(PF_ETC2_R11_EAC) \
	op(PF_ETC2_RG11_EAC) \
	op(PF_R8) \
	op(PF_B5G5R5A1_UNORM) \
	op(PF_ASTC_4x4_HDR) \
	op(PF_ASTC_6x6_HDR) \
	op(PF_ASTC_8x8_HDR) \
	op(PF_ASTC_10x10_HDR) \
	op(PF_ASTC_12x12_HDR) \
	op(PF_G16R16_SNORM) \
	op(PF_R8G8_UINT) \
	op(PF_R32G32B32_UINT) \
	op(PF_R32G32B32_SINT) \
	op(PF_R32G32B32F) \
	op(PF_R8_SINT) \
	op(PF_R64_UINT) \
	op(PF_R9G9B9EXP5) \
	op(PF_P010) \
	op(PF_ASTC_4x4_NORM_RG) \
	op(PF_ASTC_6x6_NORM_RG) \
	op(PF_ASTC_8x8_NORM_RG) \
	op(PF_ASTC_10x10_NORM_RG) \
	op(PF_ASTC_12x12_NORM_RG) \
// 注意：这一行引擎原文是 op(PF_R8G8B8) 加反斜杠，反斜杠前没有空格，照抄不改。
	op(PF_R8G8B8)\
// 宏定义结束（最后一行没有续行反斜杠）。
	op(PF_R16G16_SINT)

// 每个像素格式哪些通道有效的位标志（R/G/B/A 各占一位）。
// 定义每种像素格式哪些通道有效
// 通道标志枚举：底层 uint8，用位运算组合。
enum class EPixelFormatChannelFlags : uint8
// 枚举体开始。
{
// 第 0 位：R 通道有效。
	R = 1 << 0,
// 第 1 位：G 通道有效。
	G = 1 << 1,
// 第 2 位：B 通道有效。
	B = 1 << 2,
// 第 3 位：A 通道有效。
	A = 1 << 3,
// R + G 组合。
	RG = R | G,
// R + G + B 组合。
	RGB = R | G | B,
// R + G + B + A 组合。
	RGBA = R | G | B | A,

// 0：没有任何通道有效。
	None = 0,
// 枚举体结束。
};
// 给这个 enum class 生成位运算与 EnumHasAllFlags / EnumHasAnyFlags 等工具函数。
ENUM_CLASS_FLAGS(EPixelFormatChannelFlags);

// 像素格式能力位标志：能不能当 1D/2D/3D/Cube 纹理、能不能当渲染目标、能不能读写缓冲等。
enum class EPixelFormatCapabilities : uint32
// 枚举体开始。
{
// 0：没有任何能力。
    None             = 0,
// 第 1 位：可作 1D 纹理。
    Texture1D        = 1ull << 1,
// 第 2 位：可作 2D 纹理。
    Texture2D        = 1ull << 2,
// 第 3 位：可作 3D 纹理。
    Texture3D        = 1ull << 3,
// 第 4 位：可作 Cube 纹理。
    TextureCube      = 1ull << 4,
// 第 5 位：可作渲染目标（RenderTarget）。
    RenderTarget     = 1ull << 5,
// 第 6 位：可作深度模板缓冲。
    DepthStencil     = 1ull << 6,
// 第 7 位：支持 mipmap。
	TextureMipmaps   = 1ull << 7,
// 第 8 位：支持纹理 Load（按坐标读，不走采样器）。
	TextureLoad      = 1ull << 8,
// 第 9 位：支持纹理 Sample（走采样器过滤）。
	TextureSample    = 1ull << 9,
// 第 10 位：支持 Gather（一次取 4 个 texel）。
	TextureGather    = 1ull << 10,
// 第 11 位：支持纹理原子操作。
	TextureAtomics   = 1ull << 11,
// 第 12 位：支持混合（Blend）。
	TextureBlendable = 1ull << 12,
// 第 13 位：支持纹理 Store（写入）。
	TextureStore     = 1ull << 13,

// 第 14 位：可作缓冲（Buffer）。
	Buffer           = 1ull << 14,
// 第 15 位：可作顶点缓冲。
    VertexBuffer     = 1ull << 15,
// 第 16 位：可作索引缓冲。
    IndexBuffer      = 1ull << 16,
// 第 17 位：支持缓冲 Load。
	BufferLoad       = 1ull << 17,
// 第 18 位：支持缓冲 Store。
    BufferStore      = 1ull << 18,
// 第 19 位：支持缓冲原子操作。
    BufferAtomics    = 1ull << 19,

// 第 20 位：可作 UAV（无序访问视图）。
	UAV              = 1ull << 20,
// 第 21 位：支持带类型的 UAV 读。
    TypedUAVLoad     = 1ull << 21,
// 第 22 位：支持带类型的 UAV 写。
	TypedUAVStore    = 1ull << 22,

// 第 23 位：可被硬件线性过滤。
	TextureFilterable = 1ull << 23,
// 第 24 位：可做有损压缩。
	LossyCompressible = 1ull << 24,

// 组合：任意纹理类型（1D / 2D / 3D / Cube）。
	AnyTexture       = Texture1D | Texture2D | Texture3D | TextureCube,

// 组合：所有纹理相关能力位。
	AllTextureFlags  = AnyTexture | RenderTarget | DepthStencil | TextureMipmaps | TextureLoad | TextureSample | TextureGather | TextureAtomics | TextureBlendable | TextureStore,
// 组合：所有缓冲相关能力位。
	AllBufferFlags   = Buffer | VertexBuffer | IndexBuffer | BufferLoad | BufferStore | BufferAtomics,
// 组合：所有 UAV 相关能力位。
	AllUAVFlags      = UAV | TypedUAVLoad | TypedUAVStore,

// 组合：全部能力位。
	AllFlags         = AllTextureFlags | AllBufferFlags | AllUAVFlags
// 枚举体结束。
};
// 给这个 enum class 生成位运算与 EnumHasAllFlags 等工具函数。
ENUM_CLASS_FLAGS(EPixelFormatCapabilities);

// 说明：EPixelFormat 目前和 uint8 混用，很多调用点还收 uint8；
// EPixelFormat 目前和 uint8 混用，大多数接收 uint8 的调用点
// 理想情况应改成收 EPixelFormat，过渡期用下面这个宏消除类型转换警告。
// 应该改成接收 EPixelFormat，但过渡期用下面这个宏来消除
// 说明结束。
// 类型转换警告
// 把 EPixelFormat 显式转成 uint8（消除隐式转换警告）。
#define UE_PIXELFORMAT_TO_UINT8(argument) static_cast<uint8>(argument)

// 判断是不是 ETC 系列块压缩格式（ETC2 三通道 / 四通道 / 单通道 EAC / 双通道 EAC）。
inline bool IsETCBlockCompressedPixelFormat(EPixelFormat PixelFormat)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告（这里只关心少数几个值）。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (PixelFormat)
// switch 体开始。
	{
// ETC2 三通道压缩。
	case PF_ETC2_RGB:
// ETC2 四通道压缩。
	case PF_ETC2_RGBA:
// ETC2 单通道 11 位 EAC。
	case PF_ETC2_R11_EAC:
// ETC2 双通道 11 位 EAC。
	case PF_ETC2_RG11_EAC:
// 命中：是 ETC 块压缩。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不是。
	return false;
// 函数结束。
}

// 判断是不是 ASTC 系列块压缩格式（普通 / HDR / RG 三组，共 15 个）。
FORCEINLINE bool IsASTCBlockCompressedTextureFormat(EPixelFormat PixelFormat)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (PixelFormat)
// switch 体开始。
	{
// ASTC 4x4。
	case PF_ASTC_4x4:
// ASTC 6x6。
	case PF_ASTC_6x6:
// ASTC 8x8。
	case PF_ASTC_8x8:
// ASTC 10x10。
	case PF_ASTC_10x10:
// ASTC 12x12。
	case PF_ASTC_12x12:
// ASTC 4x4 HDR。
	case PF_ASTC_4x4_HDR:
// ASTC 6x6 HDR。
	case PF_ASTC_6x6_HDR:
// ASTC 8x8 HDR。
	case PF_ASTC_8x8_HDR:
// ASTC 10x10 HDR。
	case PF_ASTC_10x10_HDR:
// ASTC 12x12 HDR。
	case PF_ASTC_12x12_HDR:
// ASTC 4x4 RG 双通道。
	case PF_ASTC_4x4_NORM_RG:
// ASTC 6x6 RG 双通道。
	case PF_ASTC_6x6_NORM_RG:
// ASTC 8x8 RG 双通道。
	case PF_ASTC_8x8_NORM_RG:
// ASTC 10x10 RG 双通道。
	case PF_ASTC_10x10_NORM_RG:
// ASTC 12x12 RG 双通道。
	case PF_ASTC_12x12_NORM_RG:
// 命中：是 ASTC 块压缩。
		return true;
// 其余格式走 default。
	default:
// 不是 ASTC。
		return false;
// switch 体结束。
	}
// 恢复警告设置（引擎原文在 return 之后，实际不可达，照抄不改）。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 函数结束。
}

// 判断是不是 BC / DXT 系列块压缩格式（DXT1/3/5 + BC4/5/6H/7）。
inline bool IsBlockCompressedFormat(EPixelFormat Format)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (Format)
// switch 体开始。
	{
// DXT1（BC1）。
	case PF_DXT1:
// DXT3（BC2）。
	case PF_DXT3:
// DXT5（BC3）。
	case PF_DXT5:
// BC4 单通道。
	case PF_BC4:
// BC5 双通道。
	case PF_BC5:
// BC6H HDR。
	case PF_BC6H:
// BC7 高质量 RGBA。
	case PF_BC7:
// 命中：是 BC/DXT 块压缩。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不是。
	return false;
// 函数结束。
}

// 判断是不是任意一种块压缩格式（BC/DXT + ASTC + ETC 三类取并集）。
FORCEINLINE bool IsAnyBlockCompressedPixelFormat(EPixelFormat PixelFormat)
// 函数体开始。
{
// 三个判断函数任一为真即为真。
	return IsBlockCompressedFormat(PixelFormat) || IsASTCBlockCompressedTextureFormat(PixelFormat) || IsETCBlockCompressedPixelFormat(PixelFormat);
// 函数结束。
}

// 说明：IsHDR 的语义其实不明确，另见 IsFloatFormat。
// IsHDR 到底该表示什么并不明确
// 判断是不是 HDR 格式（浮点 / BC6H / ASTC HDR 系列）。
//	另见 IsFloatFormat
// 函数体开始。
FORCEINLINE bool IsHDR(EPixelFormat PixelFormat)
// 逐个比较：RGBA16F、BC6H、R16F、R32F、RGBA32F，或任一 ASTC HDR 变体。
{
// 函数结束。
	return PixelFormat == PF_FloatRGBA || PixelFormat == PF_BC6H || PixelFormat == PF_R16F || PixelFormat == PF_R32_FLOAT || PixelFormat == PF_A32B32G32R32F
		|| PixelFormat == PF_ASTC_4x4_HDR || PixelFormat == PF_ASTC_6x6_HDR || PixelFormat == PF_ASTC_8x8_HDR || PixelFormat == PF_ASTC_10x10_HDR || PixelFormat == PF_ASTC_12x12_HDR;
}

// 判断是不是整数格式（UINT / SINT 系列，共 17 个）。
FORCEINLINE bool IsInteger(EPixelFormat PixelFormat)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (PixelFormat)
// switch 体开始。
	{
// R32 无符号整数。
	case PF_R32_UINT:
// R32 有符号整数。
	case PF_R32_SINT:
// R16 无符号整数。
	case PF_R16_UINT:
// R16 有符号整数。
	case PF_R16_SINT:
// RGBA16 无符号整数。
	case PF_R16G16B16A16_UINT:
// RGBA16 有符号整数。
	case PF_R16G16B16A16_SINT:
// RGBA32 无符号整数。
	case PF_R32G32B32A32_UINT:
// RG16 无符号整数。
	case PF_R16G16_UINT:
// R8 无符号整数。
	case PF_R8_UINT:
// RGBA8 无符号整数。
	case PF_R8G8B8A8_UINT:
// RG32 无符号整数。
	case PF_R32G32_UINT:
// RG8 无符号整数。
	case PF_R8G8_UINT:
// RGB32 无符号整数。
	case PF_R32G32B32_UINT:
// RGB32 有符号整数。
	case PF_R32G32B32_SINT:
// R8 有符号整数。
	case PF_R8_SINT:
// R64 无符号整数。
	case PF_R64_UINT:
// RG16 有符号整数。
	case PF_R16G16_SINT:
// 命中：是整数格式。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不是。
	return false;
// 函数结束。
}

// 说明：IsFloatFormat 不含全部浮点格式，另见 IsHDR。
// IsFloatFormat 并不包含全部浮点格式；另见 IsHDR
// 判断是不是浮点格式（10 个）。
inline bool IsFloatFormat(EPixelFormat Format)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (Format)
// switch 体开始。
	{
// RGBA32F。
	case PF_A32B32G32R32F:
// 三通道浮点（R11G11B10）。
	case PF_FloatRGB:
// RGBA16F。
	case PF_FloatRGBA:
// R32F。
	case PF_R32_FLOAT:
// RG16F。
	case PF_G16R16F:
// RG16F 可过滤。
	case PF_G16R16F_FILTER:
// RG32F。
	case PF_G32R32F:
// R16F。
	case PF_R16F:
// R16F 可过滤。
	case PF_R16F_FILTER:
// R11G11B10 打包浮点。
	case PF_FloatR11G11B10:
// 命中：是浮点格式。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不是。
	return false;
// 函数结束。
}

// 判断是不是深度或模板格式（D24 / DepthStencil / X24_G8 / ShadowDepth / R32F）。
FORCEINLINE bool IsDepthOrStencilFormat(EPixelFormat Format)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (Format)
// switch 体开始。
	{
// 24 位深度。
	case PF_D24:
// 深度 + 模板。
	case PF_DepthStencil:
// 深度 + 模板的别名视图。
	case PF_X24_G8:
// 阴影深度。
	case PF_ShadowDepth:
// R32F 也常被当深度用。
	case PF_R32_FLOAT:
// 命中：是深度/模板格式。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不是。
	return false;
// 函数结束。
}

// 判断是不是带模板的格式（只有 DepthStencil 和 X24_G8 两个）。
FORCEINLINE bool IsStencilFormat(EPixelFormat Format)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (Format)
// switch 体开始。
	{
// 深度 + 模板。
	case PF_DepthStencil:
// 深度 + 模板的别名视图。
	case PF_X24_G8:
// 命中：带模板。
		return true;
// switch 体结束。
	}
// 恢复警告设置。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 其余格式都不带模板。
	return false;
// 函数结束。
}

// 判断是不是 DXTC（BC）系列块压缩格式（判断集合与 IsBlockCompressedFormat 相同）。
FORCEINLINE bool IsDXTCBlockCompressedTextureFormat(EPixelFormat PixelFormat)
// 函数体开始。
{
// 临时关闭 switch 未覆盖所有枚举值的警告。
	PRAGMA_DISABLE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 按格式分支。
	switch (PixelFormat)
// switch 体开始。
	{
// DXT1。
	case PF_DXT1:
// DXT3。
	case PF_DXT3:
// DXT5。
	case PF_DXT5:
// BC4。
	case PF_BC4:
// BC5。
	case PF_BC5:
// BC6H。
	case PF_BC6H:
// BC7。
	case PF_BC7:
// 命中：是 DXTC 块压缩。
		return true;
// 其余格式走 default。
	default:
// 不是 DXTC。
		return false;
// switch 体结束。
	}
// 恢复警告设置（引擎原文在 return 之后，不可达，照抄不改）。
	PRAGMA_RESTORE_SWITCH_UNHANDLED_ENUM_CASE_WARNINGS;
// 函数结束。
}

// 判断这个格式是否要求宽高按 4 对齐。
FORCEINLINE bool RequiresBlock4Alignment(EPixelFormat PixelFormat)
// 函数体开始。
{
// 说明：BC/DXTC 格式要求 4x4 对齐，ASTC / ETC 不要求。
	// BCN/DXTC 格式要求 4x4 对齐，但 ASTC/ETC 不要求
// 直接复用 DXTC 判断结果。
	return IsDXTCBlockCompressedTextureFormat(PixelFormat);
// 函数结束。
}

// 像素格式信息结构体：描述一种格式的块尺寸、字节数、通道数、平台能力等。
/** 
// 说明：大部分字段在静态初始化后就有效；但 Capabilities / PlatformFormat 等由 RHI 模块填充，
* 关于一种像素格式的信息。这个结构体的大部分内容在静态初始化后就有效，但 RHI 会在这里
// 没有 RHI 的普通程序里这些字段无效（下面字段注释里会标注）。
* 保留一些由该模块初始化的状态，没有 RHI 的普通程序不应使用这些字段
// 说明结束。
* （注释里会标注出来）。
*/
// 结构体开始。
struct FPixelFormatInfo
{
// 禁止默认构造：必须传全部参数，避免出现空描述。
	FPixelFormatInfo() = delete;
// 构造函数：由 GPixelFormats 表在静态初始化时调用。
	FPixelFormatInfo(
// 参数：对应的 EPixelFormat 枚举值。
		EPixelFormat InUnrealFormat,
// 参数：格式名字符串（如 PF_B8G8R8A8）。
		const TCHAR* InName,
// 参数：压缩块宽度（未压缩格式为 1）。
		int32 InBlockSizeX,
// 参数：压缩块高度（未压缩格式为 1）。
		int32 InBlockSizeY,
// 参数：压缩块深度（未压缩格式为 1）。
		int32 InBlockSizeZ,
// 参数：一个压缩块占多少字节。
		int32 InBlockBytes,
// 参数：通道数（1~4）。
		int32 InNumComponents,
// 参数：当前平台是否支持。
		bool  InSupported);

// 格式名字符串。
	const TCHAR*				Name;
// 对应的枚举值。
	EPixelFormat				UnrealFormat;
// 压缩块宽度。
	int32						BlockSizeX;
// 压缩块高度。
	int32						BlockSizeY;
// 压缩块深度。
	int32						BlockSizeZ;
// 一个压缩块占多少字节。
	int32						BlockBytes;
// 通道数。
	int32						NumComponents;

// 该格式在当前平台的能力位（由 RHI 模块初始化，否则无效）。
	/** 该格式在各平台上的能力位（由 RHI 模块初始化，否则无效） */
// 能力位字段，默认 None。
	EPixelFormatCapabilities	Capabilities = EPixelFormatCapabilities::None;

// 平台原生格式编号（由 RHI 模块初始化，否则无效）。
	/** 平台特定的转换后格式（由 RHI 模块初始化，否则无效） */
// 平台格式编号字段，默认 0。
	uint32						PlatformFormat{ 0 };

// 当前平台 / 渲染组合下是否支持该格式（1 位位域）。
	/** 该纹理格式在当前平台 / 渲染组合下是否受支持 */
// 支持标志，占 1 位。
	uint8						Supported : 1;

// 说明：为 false 时按 32 位浮点处理（由 RHI 模块初始化，否则无效）。
	// 为 false 时按 32 位浮点处理（由 RHI 模块初始化，否则无效）
// 是否是 24 位 UNORM 深度模板（1 位位域）。
	uint8						bIs24BitUnormDepthStencil : 1;
	
// 说明：计算 2D/3D 图像或纹理的字节大小。这里算的是编码后数据的存储大小，
	/** 
// 不考虑 GPU 对齐 / 填充；对平铺或打包的 mip 尾部（主机平台 cook 后的 mip）也无效。
	* 获取 2D/3D 图像 / 纹理的字节大小。这里算的是编码后图像数据的存储大小，不考虑
// 只在你确定处理的是块式像素格式的标准纹理 / 图像时使用。
	* 任何 GPU 对齐 / 填充约束。它对平铺或打包的 mip 尾部（例如主机平台 cook 后的 mip）
// 说明结束。
	* 也无效。只在你确定处理的是块式像素格式的标准纹理 / 图像时才使用。
	*/
// 算 2D 图像总字节数。
	CORE_API uint64 Get2DImageSizeInBytes(uint32 InWidth, uint32 InHeight) const;
// 算 2D 纹理某一级 mip 的字节数。
	CORE_API uint64 Get2DTextureMipSizeInBytes(uint32 InTextureWidth, uint32 InTextureHeight, uint32 InMipIndex) const;
// 算 2D 纹理全部 mip 的字节数。
	CORE_API uint64 Get2DTextureSizeInBytes(uint32 InTextureWidth, uint32 InTextureHeight, uint32 InMipCount) const;
// 算 3D 图像总字节数。
	CORE_API uint64 Get3DImageSizeInBytes(uint32 InWidth, uint32 InHeight, uint32 InDepth) const;
// 算 3D 纹理某一级 mip 的字节数。
	CORE_API uint64 Get3DTextureMipSizeInBytes(uint32 InTextureWidth, uint32 InTextureHeight, uint32 InTextureDepth, uint32 InMipIndex) const;
// 算 3D 纹理全部 mip 的字节数。
	CORE_API uint64 Get3DTextureSizeInBytes(uint32 InTextureWidth, uint32 InTextureHeight, uint32 InTextureDepth, uint32 InMipCount) const;
	
// 说明：算给定宽度需要多少个压缩块。
	/**
// 说明结束。
	* 获取容纳给定尺寸所需的压缩块数量。
	*/
// 算宽度方向需要的块数。
	CORE_API uint64 GetBlockCountForWidth(uint32 InWidth) const;
// 算高度方向需要的块数。
	CORE_API uint64 GetBlockCountForHeight(uint32 InHeight) const;
// 结构体结束。
};


// 说明：枚举转字符串。注意返回的不是 FPixelFormatInfo::Name。
/**
// 返回示例：PF_B8G8R8A8。
 * 枚举转字符串
// 说明结束。
 * 注意：这不是 FPixelFormatInfo::Name
 * @return 例如 PF_B8G8R8A8
 */
// 枚举转字符串（如 PF_B8G8R8A8 转成字符串）。
CORE_API const TCHAR* GetPixelFormatString(EPixelFormat InPixelFormat);
// 说明：字符串转枚举（不区分大小写）。注意不是 FPixelFormatInfo::Name。
/**
// 参数示例：PF_B8G8R8A8，不能为空指针。
 * 字符串转枚举（不区分大小写）
// 说明结束。
 * 注意：这不是 FPixelFormatInfo::Name
 * @param InPixelFormatStr 例如 PF_B8G8R8A8，不能为 0
 */
// 字符串转枚举（不区分大小写）。
CORE_API EPixelFormat GetPixelFormatFromString(const TCHAR* InPixelFormatStr);


// 全局像素格式表：下标就是 EPixelFormat 的值，元素是描述该格式的 FPixelFormatInfo。
extern CORE_API FPixelFormatInfo GPixelFormats[PF_MAX];		// 把 EPixelFormat 的每个成员映射到描述该格式的 FPixelFormatInfo。

// 命名空间 UE::PixelFormat：放像素格式相关的工具函数。
namespace UE::PixelFormat
// 命名空间开始。
{
// 判断某个格式是否具备指定的全部能力位。
	inline bool HasCapabilities(EPixelFormat InFormat, EPixelFormatCapabilities InCapabilities)
// 函数体开始。
	{
// 查全局表拿到该格式的能力位，再用 EnumHasAllFlags 判断是否包含要求的全部位。
		return EnumHasAllFlags(GPixelFormats[InFormat].Capabilities, InCapabilities);
// 函数体结束。
	}
// 命名空间结束。
}
