如果你的目标不是“背 CMake 语法”，而是最终能独立维护 C/C++ 工程，我建议按 **“会用 → 理解 target → 多目录工程 → 第三方库 → 安装/打包”** 这条路线学。

你已经有 C++、GCC、VSCode 经验，所以可以直接从工程实践开始。

开始练习前，可以使用仓库提供的容器环境：[CMake 通用学习环境](doc/00_development_environment.md)。

## 第一阶段：先搞懂 CMake 在做什么

本阶段归纳笔记：[第一阶段：理解 CMake 在做什么](doc/01_cmake_overview.md)

CMake 本身不是编译器，它更像一个**构建系统生成器**。

你写：

```cmake
CMakeLists.txt
```

然后 CMake 根据它生成：

```text
Makefile
Ninja files
Visual Studio solution
...
```

最后真正执行编译的是：

```text
gcc / g++
clang
MSVC
```

最基本流程：

```bash
cmake -S . -B build
cmake --build build
```

推荐你从一开始就习惯 **out-of-source build**：

```text
project/
├── CMakeLists.txt
├── src/
├── include/
└── build/
```

不要直接在源码目录里执行：

```bash
cmake .
```

---

## 第二阶段：第一个 CMake 项目

本阶段归纳笔记：[第二阶段：第一个 CMake 项目](doc/02_first_cmake_project.md)

目录：

```text
hello/
├── CMakeLists.txt
└── main.cpp
```

`main.cpp`：

```cpp
#include <iostream>

int main()
{
    std::cout << "Hello CMake\n";
    return 0;
}
```

`CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)

project(hello)

add_executable(hello main.cpp)
```

构建：

```bash
cmake -S . -B build
cmake --build build
```

运行：

```bash
./build/hello
```

Windows 多配置生成器下可能是：

```powershell
.\build\Debug\hello.exe
```

这里先理解三个命令：

```cmake
cmake_minimum_required(...)
project(...)
add_executable(...)
```

---

## 第三阶段：一定要建立“Target 思维”

这是现代 CMake 最重要的一点。

假设：

```text
demo/
├── CMakeLists.txt
├── include/
│   └── math.hpp
└── src/
    ├── main.cpp
    └── math.cpp
```

推荐写：

```cmake
cmake_minimum_required(VERSION 3.16)

project(demo LANGUAGES CXX)

add_executable(demo
    src/main.cpp
    src/math.cpp
)

target_include_directories(demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(demo PRIVATE cxx_std_17)
```

这里的核心是：

```text
demo
```

它是一个 **target**。

之后所有配置都围绕 target：

```cmake
target_include_directories()
target_compile_definitions()
target_compile_options()
target_link_libraries()
target_compile_features()
```

可以把它理解成：

```text
target = 一个可执行程序或库 + 它需要的所有构建属性
```

这是现代 CMake 的核心模型。

---

## 第四阶段：学会创建库

例如：

```text
demo/
├── CMakeLists.txt
├── app/
│   └── main.cpp
├── include/
│   └── calculator.hpp
└── src/
    └── calculator.cpp
```

写：

```cmake
cmake_minimum_required(VERSION 3.16)

project(demo LANGUAGES CXX)

add_library(calculator
    src/calculator.cpp
)

target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(calculator PUBLIC cxx_std_17)

add_executable(app
    app/main.cpp
)

target_link_libraries(app
    PRIVATE
        calculator
)
```

这个例子非常重要。

关系是：

```text
calculator
    ↑
    │ link
    │
   app
```

你需要重点理解：

```cmake
add_library()
target_link_libraries()
```

---

## 第五阶段：彻底理解 PRIVATE / PUBLIC / INTERFACE

这是 CMake 初学阶段最值得花时间的知识点。

例如：

```cmake
target_include_directories(calculator
    PUBLIC
        include
)
```

含义：

```text
calculator 自己需要这些头文件目录
+
使用 calculator 的 target 也需要
```

而：

```cmake
PRIVATE
```

表示：

```text
只有当前 target 自己需要
```

`INTERFACE`：

```text
自己不需要
但依赖它的人需要
```

可以记成：

| 类型 | 当前 target | 使用当前 target 的人 |
|---|---:|---:|
| PRIVATE | ✓ | ✗ |
| PUBLIC | ✓ | ✓ |
| INTERFACE | ✗ | ✓ |

例如头文件库：

```cmake
add_library(my_headers INTERFACE)

target_include_directories(my_headers
    INTERFACE
        include
)
```

---

## 第六阶段：学习多目录工程

真实工程一般不会只有一个 `CMakeLists.txt`。

比如：

```text
my_project/
├── CMakeLists.txt
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp
└── libs/
    └── math/
        ├── CMakeLists.txt
        ├── include/
        └── src/
```

顶层：

```cmake
cmake_minimum_required(VERSION 3.16)

project(my_project LANGUAGES CXX)

add_subdirectory(libs/math)
add_subdirectory(app)
```

`libs/math/CMakeLists.txt`：

```cmake
add_library(math
    src/math.cpp
)

target_include_directories(math
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

`app/CMakeLists.txt`：

```cmake
add_executable(app main.cpp)

target_link_libraries(app
    PRIVATE
        math
)
```

这一阶段掌握：

```cmake
add_subdirectory()
```

以及一个非常重要的思想：

```text
每个模块自己描述自己的构建需求
顶层负责组织模块
```

---

## 第七阶段：第三方库

这是你以后做 ROS2、CUDA、Eigen、Fast DDS 都会大量遇到的。

最典型：

```cmake
find_package(Eigen3 REQUIRED)

target_link_libraries(app
    PRIVATE
        Eigen3::Eigen
)
```

注意现代 CMake 推荐：

```cmake
Eigen3::Eigen
```

而不是手动写：

```cmake
-I/usr/include/eigen3
```

好的第三方库通常会提供 **imported target**。

例如：

```cmake
find_package(Threads REQUIRED)

target_link_libraries(app
    PRIVATE
        Threads::Threads
)
```

你以后看到：

```text
Foo::Bar
```

一般就应该想到：

> 这是某个库导出的 CMake target。

---

## 第八阶段：学会 Debug / Release

常见命令：

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug
```

然后：

```bash
cmake --build build
```

Release：

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release
```

不过 Visual Studio 这种 multi-config generator 通常使用：

```bash
cmake --build build --config Release
```

这两个模型以后要区分清楚：

```text
single-config
    Makefiles
    Ninja

multi-config
    Visual Studio
    Ninja Multi-Config
```

---

## 第九阶段：学会看 CMake 到底执行了什么

非常实用：

```bash
cmake --build build --verbose
```

你会看到真正的：

```bash
g++ ...
```

这对排查：

```text
头文件路径
宏定义
编译选项
链接库
链接顺序
```

极其有帮助。

还可以看缓存：

```bash
cmake -LA -N build
```

或者直接研究：

```text
build/CMakeCache.txt
```

你之前遇到 ROS 2 构建时 CMake 错选 Conda Python，本质上就是很典型的 **CMake cache / dependency discovery** 问题，所以这部分尤其值得重点掌握。

---

## 第十阶段：变量和条件判断

这部分可以稍后学，不要一开始沉迷 CMake 脚本语法。

例如：

```cmake
set(MY_OPTION ON)

if(MY_OPTION)
    message(STATUS "MY_OPTION enabled")
endif()
```

自定义 option：

```cmake
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
```

配置：

```bash
cmake -S . -B build -DBUILD_TESTS=OFF
```

常见变量：

```cmake
CMAKE_CURRENT_SOURCE_DIR
CMAKE_CURRENT_BINARY_DIR
CMAKE_SOURCE_DIR
CMAKE_BINARY_DIR
CMAKE_BUILD_TYPE
CMAKE_CXX_COMPILER
```

---

## 第十一阶段：测试

建议顺手学习 CTest：

```cmake
enable_testing()

add_executable(test_math test_math.cpp)

target_link_libraries(test_math PRIVATE calculator)

add_test(
    NAME math_test
    COMMAND test_math
)
```

然后：

```bash
cmake --build build
ctest --test-dir build
```

真实 C++ 项目一般是：

```text
CMake + GoogleTest + CTest
```

---

## 第十二阶段：install

当你开始考虑“我的库怎么给别人用”，就学习：

```cmake
install()
```

例如：

```cmake
install(
    TARGETS calculator
    DESTINATION lib
)

install(
    DIRECTORY include/
    DESTINATION include
)
```

安装：

```bash
cmake --install build --prefix ./install
```

得到：

```text
install/
├── include/
└── lib/
```

再往后才需要研究：

```text
install(EXPORT)
configure_package_config_file()
write_basic_package_version_file()
```

这些属于“库作者级 CMake”。

---

# 我推荐你的学习顺序

不要按 CMake 官方文档从头往后啃，而是做 6 个小项目：

```text
01_hello
    add_executable

02_library
    add_library
    target_link_libraries

03_include
    target_include_directories
    PRIVATE / PUBLIC / INTERFACE

04_multi_directory
    add_subdirectory

05_external_library
    find_package(Eigen3)
    imported target

06_real_project
    app/
    libs/
    tests/
    install/
```

最后工程长这样：

```text
cmake_study/
├── CMakeLists.txt
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp
├── libs/
│   └── math/
│       ├── CMakeLists.txt
│       ├── include/
│       └── src/
├── tests/
│   ├── CMakeLists.txt
│   └── test_math.cpp
└── build/
```

掌握这个工程以后，你基本就不再是 CMake 初学者了。

## 有几个原则建议从第一天就遵守

尽量写：

```cmake
target_include_directories()
target_link_libraries()
target_compile_features()
target_compile_options()
```

少写这种全局配置：

```cmake
include_directories()
link_directories()
add_definitions()
```

现代 CMake 的核心思想可以浓缩成一句话：

```text
Everything is a target.
```

也就是：

```text
创建 target
   ↓
描述 target 的依赖
   ↓
描述 target 的编译属性
   ↓
让属性沿依赖关系传播
```

如果你把 **target、PUBLIC/PRIVATE/INTERFACE、add_subdirectory、find_package** 这四件事真正搞懂，后面学 ROS2、CUDA、Eigen、GoogleTest 等项目的 CMake 会轻松很多。

如果按实战方式学习，我建议下一步直接从 **第 1 个 `hello_cmake` 项目开始**，然后逐步把它扩展成“可执行程序 → 静态库 → 多目录 → Eigen → GoogleTest”的完整练习工程。
