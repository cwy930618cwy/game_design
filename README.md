# 天下第一武斗会

> **类型**：Steam 单机动作战斗  
> **引擎**：Unreal Engine 5.6  
> **对标**：Jump 大乱斗系列（Jump Ultimate Stars / 火影忍者：究极风暴）  
> **核心体验**：打击感 · 冲击波 · 技能释放 · 飞行 · 变身

---

## Git 仓库

```
https://github.com/cwy930618cwy/game_design.git
```

---

## 项目使用流程

### 0. 安装 Git

#### Windows 用户

> 推荐安装 Git Bash，它提供了类 Linux 的命令行环境，比 CMD/PowerShell 更好用。

**安装步骤：**

1. 下载地址：https://git-scm.com/downloads/win
2. 运行安装程序，一路默认 Next 即可（推荐选项如下）
3. 安装完成后，在任意文件夹空白处 **右键 → Git Bash Here** 即可打开

**推荐安装选项：**

| 步骤 | 推荐选择 |
|------|----------|
| Select Components | 默认即可，确保勾选 "Git Bash Here" |
| Default editor | 选 "Use Visual Studio Code as Git's default editor"（可选） |
| Adjusting your PATH | 选 "Git from the command line and also from 3rd-party software" |
| Choosing HTTPS transport | 选 "Use the OpenSSL library" |
| Configuring the line ending conversions | 选 "Checkout Windows-style, commit Unix-style line endings"（推荐） |

**验证安装成功：**

```bash
git --version
# 输出：git version 2.xx.x.windows.x 即成功
```

---

#### Mac 用户

> macOS 通常已自带 Git，但版本可能较旧，推荐通过 Homebrew 安装最新版。

**方式一：检查是否已安装**

已更新，Mac 打开终端的方式现在包括：
⌘ + 空格 → 输入 "Terminal" → 回车

```bash
git --version
# 若输出版本号则已安装，可跳过
```

**方式二：通过 Homebrew 安装（推荐）**

```bash
# 1. 如果未安装 Homebrew，先安装 Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. 安装 Git
brew install git

# 3. 验证
git --version
# 输出：git version 2.xx.x 即成功
```

**方式三：官网下载安装包**

1. 下载地址：https://git-scm.com/downloads/mac
2. 下载对应版本（Intel 或 Apple Silicon）
3. 双击 .dmg 文件，按提示安装

**Mac 打开终端方式：**

| 方式 | 操作 |
|------|------|
| Spotlight 搜索（最快） | 按 `⌘ + 空格`，输入 "Terminal" 或 "终端"，回车 |
| Launchpad | 打开 Launchpad → 其他 → 终端 |
| Finder 路径 | 应用程序 → 实用工具 → 终端.app |
| iTerm2（推荐） | 更好用的第三方终端，支持分屏/搜索/快捷键 |
| VS Code 内置终端 | 编辑器内 `` ⌃ + ` `` 打开，方便快捷 |

---

### 1. 克隆仓库

```bash
git clone https://github.com/cwy930618cwy/game_design.git
cd game_design
```

### 2. 目录结构

```
game_design/
├── README.md                  # 项目说明（本文件）
├── code/                      # UE5.6 项目工程
│   ├── code.uproject          # 项目文件（双击打开）
│   ├── Config/                # 引擎/项目配置
│   ├── Content/               # 游戏资产（蓝图/材质/贴图等）
│   └── Source/                # C++ 源码
├── 设计大纲/
│   ├── 设计大纲.md            # 游戏整体设计大纲
│   └── 项目结构设计.md        # 技术选型、目录结构、命名规范
├── 美术/
│   └── 美术设计步骤/
│       └── 美术设计流程.md    # 角色/场景/UI 美术制作流程
├── 技术/                      # 技术文档
└── .codebuddy/                # AI 辅助开发记忆（勿删）
```

### 3. 日常开发流程

```
git pull origin main          # 拉取最新代码
    ↓
进行开发/设计工作
    ↓
git add .                     # 暂存所有修改
git commit -m "描述修改内容"    # 提交
git push origin main          # 推送到远程
```

### 4. 分支策略

| 分支 | 用途 |
|------|------|
| `main` | 主分支，稳定版本 |
| `dev` | 开发分支（可选） |
| `feature/xxx` | 功能分支（如 feature/combat-system） |

### 5. 提交规范

```
feat: 新增角色战斗系统
fix: 修复打击感卡肉时间过长
docs: 更新设计大纲
art: 完成角色001建模
design: 新增关卡设计文档
```

### 6. 首次使用 UE5.6 打开项目

```bash
# 1. 用 UE 5.6 打开 code/code.uproject 文件
# 2. 等待引擎编译着色器（首次较慢）
# 3. 确认所有插件已启用（GAS / Niagara / Motion Warping）
# 4. 打开主关卡即可开始开发
```

---

## 设计文档索引

| 文档 | 路径 | 内容 |
|------|------|------|
| 设计大纲 | `设计大纲/设计大纲.md` | 游戏流程、战斗系统、关卡、AI、角色、结算 |
| 项目结构 | `设计大纲/项目结构设计.md` | 美术风格、建模、动画、目录结构、命名规范、渲染方案 |
| 美术流程 | `美术/美术设计步骤/美术设计流程.md` | 角色/场景/UI 制作步骤与排期 |

---

## 开发环境

| 软件 | 版本 | 用途 |
|------|------|------|
| Unreal Engine | 5.6 | 游戏引擎 |
| Blender | 4.2+ | 建模 / 动画 |
| Substance Painter | 10.x | 贴图绘制 |
| Visual Studio | 2022 | C++ 编译 |
| Git + Git LFS | 2.x | 版本管理 |
