# 04 — UPrimaryDataAsset 详解

> **定位**：Lyra 里大量类继承的基类——**`UPrimaryDataAsset`**。Experience、PawnData、ActionSet 都继承它。
>
> **一句话**：`UPrimaryDataAsset` = UE 的**"主数据资产"基类**。专为"被引用的数据资产"设计，带**主资产 ID（PrimaryAssetId）**，方便资产注册表查找、异步加载。
>
> **文件**：`Engine/Source/Runtime/Engine/Classes/Engine/PrimaryAssetId.h`、`DataAsset.h`

---

## 一、先搞懂：什么是"数据资产"（DataAsset）

**数据资产（DataAsset）= 一种 UObject，用来存"配置数据"**，而不是游戏逻辑。

```
DataAsset（数据资产）
  ├─ 存数据（血量、伤害、名字、规则）
  ├─ 在编辑器里创建、编辑
  ├─ 能被引用、能存盘
  └─ 不是 Actor，不是代码逻辑，就是"数据"
```

**类比**：DataAsset = 游戏的"配置表"。像技能配置、怪物数据、经验规则——都存成 DataAsset，在编辑器里改，不用改代码。

```cpp
// 一个普通 DataAsset（存技能数据）
UCLASS()
class USkillData : public UDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY() float Damage;    // 伤害
    UPROPERTY() float Cooldown;  // 冷却
};
```

---

## 二、UPrimaryDataAsset 比 UDataAsset 多了什么

**UPrimaryDataAsset 是"更高级的 DataAsset"**，多了**主资产 ID（PrimaryAssetId）**：

```
UDataAsset（基础数据资产）
  └─ UPrimaryDataAsset（主数据资产）
       └─ 多了 PrimaryAssetId（主资产 ID）
```

**PrimaryAssetId 是什么**：每个 PrimaryDataAsset 有一个**唯一 ID**，由"类型 + 名字"组成：

```
PrimaryAssetId = (Type: 类型, Name: 名字)
  例：Experience: TeamDeathmatch
       PawnData: HeroPawn
```

**这个 ID 有什么用**：
1. **唯一标识**：每个数据资产一个 ID，方便查找
2. **资产注册表**：能被 AssetRegistry 索引
3. **异步加载**：能用 ID 异步加载（`LoadPrimaryAsset`）

---

## 三、为什么 Lyra 大量继承它（核心原因）

Lyra 的 Experience、PawnData、ActionSet 都继承 `UPrimaryDataAsset`，因为**这些都需要"被引用 + 异步加载"**。

### 真实源码：LyraExperienceActionSet 继承 UPrimaryDataAsset

```cpp
// LyraExperienceActionSet.h（真实源码）
UCLASS(BlueprintType, NotBlueprintable)
class ULyraExperienceActionSet : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 组合进 Experience 的动作
    UPROPERTY(EditAnywhere, Instanced, Category="Actions to Perform")
    TArray<TObjectPtr<UGameFeatureAction>> Actions;

    // 要启用的 GameFeature
    UPROPERTY(EditAnywhere, Category="Feature Dependencies")
    TArray<FString> GameFeaturesToEnable;
};
```

**为什么继承 UPrimaryDataAsset**：
- Experience 要用 ID 被"切换/加载" → 需要 PrimaryAssetId
- 数据资产要被异步加载 → PrimaryDataAsset 支持
- 要能被资产注册表索引 → PrimaryDataAsset 支持

---

## 四、Lyra 里继承 UPrimaryDataAsset 的类

从源码看，Lyra 这些类都继承它：

```
UPrimaryDataAsset
├── ULyraExperienceDefinition（Experience 规则）
├── ULyraExperienceActionSet（玩法包）
├── ULyraPawnData（玩家数据）
├── ULyraUserFacingExperienceDefinition（用户可见的体验）
└── ...（各种配置资产）
```

**共同点**：都是"**数据驱动的配置资产**"，需要被引用、被异步加载、被切换。

---

## 五、PrimaryAssetId 怎么用（核心操作）

### 获取/设置

```cpp
// 获取一个数据资产的 PrimaryAssetId
FPrimaryAssetId AssetId = MyAsset->GetPrimaryAssetId();
// 结果：(Type=MyType, Name=MyAssetName)

// 直接用 ID 异步加载资产
FSoftObjectPath Path;
FStreamableManager::Get().RequestAsyncLoad(
    AssetId.ToString(), 
    [](FName) { /* 加载完成回调 */ });
```

### 用 ID 查找

```cpp
// 按 ID 在资产注册表查找
FPrimaryAssetId Id(TEXT("Experience"), TEXT("TeamDeathmatch"));
UObject* Asset = UAssetManager::Get().GetPrimaryAssetObject(Id);
```

---

## 六、DataAsset vs UPrimaryDataAsset vs 普通 UObject

| | 普通 UObject | UDataAsset | UPrimaryDataAsset |
|---|---|---|---|
| 存数据 | 能 | 专为数据设计 | 专为数据设计 |
| PrimaryAssetId | 无 | 无 | **有** |
| 异步加载 | 普通 | 普通 | **支持** |
| 资产注册表 | 一般 | 一般 | **可索引** |
| Lyra 用 | 少 | 少 | **大量**（Experience等） |

**结论**：Lyra 数据资产多用 `UPrimaryDataAsset`，因为它**能被 ID 引用、异步加载、注册表索引**——这正是 Experience 切换玩法需要的。

---

## 七、总结速查

```
UPrimaryDataAsset = 主数据资产基类
  ├─ 继承自 UDataAsset（存数据）
  ├─ 多了 PrimaryAssetId（类型+名字的唯一ID）
  ├─ 支持异步加载
  └─ 可被资产注册表索引

Lyra 用它：Experience / PawnData / ActionSet（都是配置资产）

为什么用：
  要被引用 → 需要 ID
  要被切换/加载 → 支持异步
  要被查找 → 可索引
```

**一句话**：`UPrimaryDataAsset` 是 UE 的**主数据资产基类**，比普通 DataAsset 多了 **PrimaryAssetId（主资产 ID）**，支持**异步加载、资产注册表索引**。**Lyra 的 Experience、PawnData、ActionSet 都继承它**，因为它们是"数据驱动的配置资产"，需要被引用、切换、异步加载。

---

## 八、下一步

理解了 UPrimaryDataAsset，下一步可以看 **AssetManager 怎么管理和加载这些数据资产**（异步加载、Bundle），或深入 Lyra 用它的具体类（PawnData/Experience）。
