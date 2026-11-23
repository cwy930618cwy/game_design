// 版权声明：Epic Games 保留所有权利（源为英文版权行，按铁律译成中文，但版权行本身不翻译内容）

// 只编译一次；防止同一编译单元重复 include 导致重定义
#pragma once

// 引入日志宏头文件；DECLARE_LOG_CATEGORY_EXTERN 与 DEFINE_LOG_CATEGORY 都在这里
#include "Logging/LogMacros.h"

// 前置声明 UObject；第 14 行的函数参数只用到指针类型，不需要 include 整个 UObject 头文件
class UObject;

// 声明日志频道 LogLyra：Lyra 通用日志（游戏流程 / 初始化 / 杂项）
// 参数 1 = 频道变量名；参数 2 = 默认详细级别（Log）；参数 3 = 编译期可见级别（All）
// LYRAGAME_API 让其他模块也能用这个频道
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
// 声明日志频道 LogLyraExperience：Experience 系统专用（加载 Experience / GameFeature）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
// 声明日志频道 LogLyraAbilitySystem：GAS 专用（能力授予 / GE 应用 / 属性变化）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
// 声明日志频道 LogLyraTeams：队伍系统专用（队伍分配 / 切换）
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

// 声明工具函数：返回「服务端 / 客户端」上下文标记字符串，用于日志前缀
// 参数 ContextObject 默认 nullptr；传 Actor 取它的 LocalRole，传 Component 取它 Owner 的 Role
LYRAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);