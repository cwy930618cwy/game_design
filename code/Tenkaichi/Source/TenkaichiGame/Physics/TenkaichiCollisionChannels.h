// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


/**
 * 当你修改这个文件时请注意：这些通道信息可能随实例一起被保存，
 * 而且 DefaultEngine.ini 里的 [/Script/Engine.CollisionProfile] 必须和这里列出的保持一致。
 **/

// 与提供交互功能的 Actor/Component 进行射线检测。
#define Tenkaichi_TraceChannel_Interaction				ECC_GameTraceChannel1

// 武器使用的射线，会命中物理资源（Physics Asset）而不是胶囊体。
#define Tenkaichi_TraceChannel_Weapon					ECC_GameTraceChannel2

// 武器使用的射线，会命中角色的胶囊体而不是物理资源。
#define Tenkaichi_TraceChannel_Weapon_Capsule			ECC_GameTraceChannel3

// 武器使用的射线，会穿透多个角色，而不是停在第一个命中的目标上。
#define Tenkaichi_TraceChannel_Weapon_Multi				ECC_GameTraceChannel4

// 这条通道（ECC_GameTraceChannel5）由 ShooterCore 游戏功能插件分配给了瞄准辅助（Aim Assist）。
// ECC_GameTraceChannel5