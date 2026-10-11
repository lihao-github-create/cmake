# 第七阶段：第三方库

## 学习目标

- 使用 `find_package()` 查找第三方库并加载其 target。
- 通过 target 依赖获得头文件路径等使用要求。
- 区分安装依赖、查找依赖和使用依赖。
- 使用 `Eigen3_DIR` 和 `CMAKE_PREFIX_PATH` 指定查找位置。
- 根据公开接口的需要选择 `PRIVATE`、`PUBLIC` 或 `INTERFACE`。

## 从工程内的库到第三方库

上一阶段通过 `add_subdirectory(libs/math)` 处理库的源码目录，并在其中用 `add_library()` 创建 `math` target。

对于已安装的 Eigen，可以通过包配置获得它提供的导入目标（imported target）：

```cmake
find_package(Eigen3 REQUIRED CONFIG)
target_link_libraries(app PRIVATE Eigen3::Eigen)
```

使用库的核心仍然是建立 target 依赖。第三方 target 可以携带头文件路径、编译要求、库文件位置和其他依赖，使用者无须分别硬编码这些信息。

`Eigen3` 是包名，`Eigen3::Eigen` 是 target 名。`::` 是常见的命名空间写法，也可用于别名 target，并非只表示导入目标。

## 准备 Eigen3

本仓库的 Dockerfile 通过 Ubuntu 软件包 `libeigen3-dev` 安装 Eigen 的头文件和 CMake 包配置。旧镜像需要按 [README](../README.md) 的说明重新构建。

在容器中检查：

```bash
dpkg -s libeigen3-dev
```

输出包含 `Status: install ok installed` 表示已安装。

三个步骤各有职责：

| 步骤 | 作用 |
|---|---|
| 安装 `libeigen3-dev` | 在容器中准备依赖文件 |
| `find_package()` | 查找并加载包配置，获得 target |
| `target_link_libraries()` | 让指定 target 获得依赖的使用要求 |

本例中的 `find_package()` 不负责下载安装 Eigen，也不会自动给所有 target 添加头文件路径。

## 最小练习

练习目录：

```text
exercises/06_external_library/
├── CMakeLists.txt
└── main.cpp
```

`main.cpp`：

```cpp
#include <Eigen/Dense>
#include <iostream>

int main()
{
    Eigen::Vector3d a(1, 2, 3);
    Eigen::Vector3d b(4, 5, 6);

    std::cout << (a + b).transpose() << '\n';
}
```

`CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)

project(external_library_demo LANGUAGES CXX)

find_package(Eigen3 REQUIRED CONFIG)

add_executable(app main.cpp)

target_compile_features(app PRIVATE cxx_std_17)

target_link_libraries(app PRIVATE Eigen3::Eigen)
```

`REQUIRED` 表示找不到包就报错；`CONFIG` 明确使用包配置模式，查找并加载 Eigen 提供的配置文件。

在练习目录执行：

```bash
cmake -S . -B build
cmake --build build --verbose
./build/app
```

预期输出：

```text
5 7 9
```

## 从构建命令观察使用要求

本次容器中的编译命令包含：

```text
/usr/bin/c++ -isystem /usr/include/eigen3 ... -c .../main.cpp
```

`-isystem` 指定系统头文件搜索目录。这个路径由 `Eigen3::Eigen` 传给 `app`，无需手动调用 `target_include_directories(app ...)`。

链接命令为：

```text
/usr/bin/c++ CMakeFiles/app.dir/main.cpp.o -o app
```

Eigen 是头文件库，本例无须链接独立的 Eigen 二进制库。`target_link_libraries()` 在这里建立依赖并传递使用要求，即使没有 `.a` 或 `.so` 文件也适用。

如果目标已经是最新状态，可以用下面的命令重新显示完整编译过程：

```bash
cmake --build build --clean-first --verbose
```

## 实验：找到包不等于使用包

保留 `find_package()`，只注释掉：

```cmake
# target_link_libraries(app PRIVATE Eigen3::Eigen)
```

重新配置和构建：

```bash
cmake -S . -B build
cmake --build build --clean-first --verbose
```

本次实验中，配置成功，但编译命令失去了 `-isystem /usr/include/eigen3`，并报错：

```text
fatal error: Eigen/Dense: No such file or directory
```

这发生在编译阶段，尚未进入链接阶段。恢复 target 依赖后，搜索路径恢复，构建成功。

## 查看包配置位置

在容器中列出软件包提供的 CMake 文件：

```bash
dpkg -L libeigen3-dev | grep '\.cmake$'
```

本次环境中包含：

```text
/usr/share/eigen3/cmake/Eigen3Config.cmake
/usr/share/eigen3/cmake/Eigen3ConfigVersion.cmake
/usr/share/eigen3/cmake/Eigen3Targets.cmake
/usr/share/eigen3/cmake/UseEigen3.cmake
```

其中 `Eigen3Config.cmake` 是包配置入口，`Eigen3Targets.cmake` 定义导入 target，版本配置文件用于版本匹配。

查看当前构建目录缓存的查找结果：

```bash
cmake -LA -N build | grep Eigen3_DIR
```

本次输出：

```text
Eigen3_DIR:PATH=/usr/share/eigen3/cmake
```

注意区分：

| 路径 | 使用者 | 用途 |
|---|---|---|
| `/usr/share/eigen3/cmake` | CMake | 加载包配置和 target |
| `/usr/include/eigen3` | C++ 编译器 | 查找 Eigen 头文件 |

## 指定自定义安装位置

如果 Eigen 安装在 `/opt/eigen`，且包配置位于 `/opt/eigen/share/eigen3/cmake`，可以精确指定配置目录：

```bash
cmake -S . -B build_custom \
    -DEigen3_DIR=/opt/eigen/share/eigen3/cmake
```

也可以提供安装前缀，让 CMake 在其下的约定位置查找包：

```bash
cmake -S . -B build_custom \
    -DCMAKE_PREFIX_PATH=/opt/eigen
```

以上是自定义安装的示例路径。`Eigen3_DIR` 针对 Eigen3，`CMAKE_PREFIX_PATH` 可以帮助查找多种包。比较不同查找方式时，使用不同的新构建目录，避免已有缓存影响观察。

对于当前容器，可执行以下练习：

```bash
cmake -S . -B build_explicit \
    -DEigen3_DIR=/usr/share/eigen3/cmake
cmake --build build_explicit
./build_explicit/app
```

`Eigen3_DIR` 的判断依据是目录中是否包含可用的包配置，而非目录名称。如果头文件目录确实包含有效的包配置，且引用的文件和路径正确，也可以指定它；单独复制一个配置文件过去则未必有效。

工程通过 target 描述依赖，当前机器的安装位置通过配置参数提供。

## 第三方依赖的传播范围

假设 `app` 依赖自己编写的 `math` 库：

| Eigen 的用途 | 配置 |
|---|---|
| 仅 `math.cpp` 内部使用，公开头文件不需要 | `target_link_libraries(math PRIVATE Eigen3::Eigen)` |
| `math` 自己和公开头文件都需要 | `target_link_libraries(math PUBLIC Eigen3::Eigen)` |
| `math` 是接口库，其头文件使用 Eigen | `target_link_libraries(math INTERFACE Eigen3::Eigen)` |

如果公开头文件中包含 Eigen：

```cpp
#include <Eigen/Dense>

Eigen::Vector3d calculate();
```

那么 `app` 包含这个头文件时也需要 Eigen，即使 `app` 没有直接包含 Eigen 头文件。

对于纯头文件的接口库：

```cmake
add_library(math INTERFACE)
target_link_libraries(math INTERFACE Eigen3::Eigen)
```

`math` 没有自己的源码编译过程，只向使用者提供要求，因此这里必须使用 `INTERFACE`，不能使用 `PUBLIC`。

## 常见错误与排查

| 现象 | 优先检查 |
|---|---|
| 配置时报找不到 `Eigen3Config.cmake` | 是否安装开发包，包配置目录或安装前缀是否正确 |
| 配置成功，编译时报找不到 `Eigen/Dense` | 是否建立 target 依赖，使用要求是否正确传播 |
| 修改查找位置后仍使用旧路径 | 检查 `Eigen3_DIR` 缓存，必要时用新构建目录验证 |

## 本章结论

```text
准备第三方库
    ↓
find_package() 查找并加载包配置
    ↓
获得库提供的 target
    ↓
target_link_libraries() 建立依赖
    ↓
使用要求沿 target 依赖传播
```

接入第三方库延续了现代 CMake 的 target 思维：通过依赖表达需要什么，让库的 target 提供使用它所需的构建信息。
