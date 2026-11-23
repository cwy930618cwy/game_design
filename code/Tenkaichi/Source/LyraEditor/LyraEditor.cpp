// 引入模块管理头文件；IMPLEMENT_MODULE 宏与 FDefaultModuleImpl 都在这里
#include "Modules/ModuleManager.h"

// 注册普通模块；把实现类 FDefaultModuleImpl 与模块名 LyraEditor 绑定
IMPLEMENT_MODULE(FDefaultModuleImpl, LyraEditor);