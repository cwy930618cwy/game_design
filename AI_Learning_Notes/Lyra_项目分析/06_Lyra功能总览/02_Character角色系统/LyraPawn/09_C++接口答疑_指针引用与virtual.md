# 09 — C++ 接口答疑：`*`/`&` 与 `virtual`

> **定位**：把学接口时最常卡住的两个地方总结清楚——**`*`（指针）和 `&`（引用）**，以及 **`virtual`（多态）**。
>
> **一句话**：`*` 是"门牌号（地址）"，`&` 是"直接拿原物不复印"，`virtual` 是"多态开关（按实际类型调用）"。

---

## 一、`*`（指针）和 `&`（引用）—— 两个完全不同的东西

### 1.1 这行代码拆开看

先看这行代码，它里面同时出现了 `*` 和 `&`：

```cpp
void makeSound(vector<ISpeakable*>& animals) {
//             ①             ②      ③
```

| 符号 | 位置 | 含义 |
|:---:|------|------|
| ① `ISpeakable*` | 在 `<>` 里 | **指针**：容器里存的是"指向 ISpeakable 的指针" |
| ② `&` | 参数名前 | **引用**：参数是 vector 的引用（不拷贝） |

**注意：这两个是不同东西**：

- `ISpeakable*` = **元素类型是指针**
- `vector<...>&` = **参数是引用**

---

### 1.2 `*`（指针）= "门牌号/地址"

`ISpeakable*` = **不是对象本身，是对象的地址（门牌号）**。

```cpp
// 直接存对象
vector<Dog> dogs;          // 存 Dog 对象本身

// 存指针（地址）
vector<ISpeakable*> animals;   // 存的是"指向对象的地址"
```

**为什么要存指针**：`ISpeakable` 是接口（不能实例化），只能存"指向子类对象的指针"。

```cpp
Dog dog;
Cat cat;

vector<ISpeakable*> animals;
animals.push_back(&dog);   // 存 Dog 的地址
animals.push_back(&cat);   // 存 Cat 的地址
```

---

### 1.3 `&`（引用）= "直接拿原物，不复印"

`vector<ISpeakable*>& animals` = **参数是引用，不拷贝原 vector**。

```cpp
// 传引用（&）：不拷贝，直接用原 vector（省内存、能改）
void makeSound(vector<ISpeakable*>& animals) { ... }

// 传值（无 &）：拷贝一份（浪费、改了外面不变）
void makeSound(vector<ISpeakable*> animals) { ... }
```

---

### 1.4 生活比喻

```
vector<ISpeakable*>& animals
= "一本装着动物门牌号的名单本，直接递给你用（不复印）"

  ├─ vector      = 名单本（盒子）
  ├─ ISpeakable* = 门牌号（动物地址）
  └─ &           = 直接拿原物（不复印）
```

| 符号 | 生活比喻 | 一句话 |
|:---:|---------|--------|
| `*`（指针） | **门牌号/地址** | 存的是"在哪"，不是东西本身 |
| `&`（引用） | **直接拿，不复印** | 不拷贝，用同一个 |

---

## 二、`virtual`（虚函数）= "多态开关"

### 2.1 它解决什么问题

看这段代码，`a` 是接口指针，但狗猫鸭的 `speak()` 不一样：

```cpp
vector<ISpeakable*> zoo = {&dog, &cat, &duck};

for (auto a : zoo) {
    a->speak();
    // a 是 ISpeakable* 类型
    // 怎么知道调用"狗的叫"还是"猫的叫"？
}
```

**`virtual` 就是答案**：让"`a->speak()`"能**根据实际是狗还是猫，调用对应的那个**。

---

### 2.2 `virtual` 是什么

**`virtual`（虚函数）= 告诉编译器"允许子类重写，并按实际类型调用"**。

```cpp
class ISpeakable {
public:
    virtual void speak() const = 0;   // virtual：允许重写 + 多态调用
};
```

- 加了 `virtual`：子类能重写，通过接口指针调用时**自动调实际类型的那个**（多态）
- 不加 `virtual`：按"声明的类型"调用，不会多态

---

### 2.3 有 virtual vs 没 virtual

```cpp
// ✅ 有 virtual：多态，按实际类型调用
class Animal {
public:
    virtual void speak() { cout << "动物"; }
};
class Dog : public Animal {
public:
    void speak() override { cout << "汪汪"; }
};
Animal* a = &dog;
a->speak();   // ✅ 输出"汪汪"（按实际是狗）

// ❌ 没 virtual：按声明类型调用
class Animal {
public:
    void speak() { cout << "动物"; }   // 没 virtual
};
class Dog : public Animal {
public:
    void speak() { cout << "汪汪"; }
};
Animal* a = &dog;
a->speak();   // ❌ 输出"动物"（按 Animal 类型，不按狗）
```

**区别**：

- **有 virtual**：`a->speak()` 调的是**狗的叫**（多态）
- **没 virtual**：`a->speak()` 调的是**Animal 的叫**（不多态）

---

### 2.4 `= 0`（纯虚函数）= "抽象方法，子类必须实现"

```cpp
virtual void speak() const = 0;
//                          ↑ = 0：这个函数"没有实现"，只声明"必须要有"
```

- **`= 0`** = 抽象方法，**没有实际实现**
- 意思 = "所有继承我的类，必须自己实现 speak()"
- 有纯虚函数的类 = **接口/抽象类**，不能直接创建对象

```cpp
// ISpeakable 是接口（有 = 0）
// 意思：谁继承我，谁必须实现 speak()
class Dog : public ISpeakable {
    void speak() const override { cout << "汪汪"; }   // 必须实现
};
```

---

### 2.5 `virtual ~ISpeakable()`（虚析构）= "删除时正确释放"

```cpp
virtual ~ISpeakable() {}   // 空的虚析构函数
```

**作用**：通过接口指针 `delete` 对象时，能正确调用子类的析构函数（避免内存泄漏）。

```cpp
ISpeakable* a = new Dog(...);
delete a;
// 没虚析构 → 只调 ISpeakable 析构，不调 Dog 的 → 可能泄漏
// 有虚析构 → 正确调 Dog 的析构
```

---

### 2.6 接口那三行分别是什么

```cpp
class ISpeakable {
public:
    virtual void speak() const = 0;   // ① 纯虚：子类必须实现（多态）
    virtual ~ISpeakable() {}          // ② 虚析构：删除时正确释放
};
```

| 代码 | 作用 |
|------|------|
| `virtual void speak() const = 0` | 抽象方法：子类必须实现，支持多态 |
| `virtual ~ISpeakable() {}` | 虚析构：通过接口删除对象时正确释放 |

---

## 三、为什么要用 `vector<ISpeakable*>` 和 `virtual`？（连起来理解）

```
C++ 是强类型语言：

  → 没有 any，类型编译期定死
  → 要强校验
  → 但想统一处理不同类型（狗猫鸭）

接口 + 指针 + virtual 的组合解决：

  ① vector<ISpeakable*>：用"接口类型"装所有动物（统一身份）
  ② virtual：让 a->speak() 按实际类型调用（多态）
  ③ 结果：一个函数、一个容器，统一处理所有动物
```

**关键**：

- `ISpeakable*` 让你"能装进同一个容器"
- `virtual` 让你"调用时各调各的"
- 两者配合实现**多态**

---

## 四、速查表

| 语法 | 是什么 | 一句话 |
|------|--------|--------|
| `ISpeakable*` | 指针 | 门牌号（地址） |
| `vector<...>&` | 引用 | 直接拿原物（不复印） |
| `vector` | 动态数组 | 能装一串东西的盒子 |
| `virtual` | 虚函数 | 多态开关（按实际类型调用） |
| `= 0` | 纯虚函数 | 抽象方法，子类必须实现 |
| `virtual ~` | 虚析构 | 删除时正确释放 |

---

## 一句话总结

> `*`（指针）= 门牌号/地址，`&`（引用）= 直接拿原物不复印，`virtual` = 多态开关（按实际类型调用），`= 0` = 子类必须实现的抽象方法，`virtual ~` = 删除时正确释放。
>
> **`vector<ISpeakable*>` 装所有动物，`virtual` 让调用时各调各的，两者配合实现多态。**
