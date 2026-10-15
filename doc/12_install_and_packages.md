# 第十二阶段：安装与导出 CMake 包

## 学习目标

- 使用 `install()` 安装库和公开头文件。
- 区分源码目录、构建目录和安装目录。
- 导出 target 的使用要求，生成可供 `find_package()` 查找的包。
- 在独立工程中链接安装后的库。
- 排查安装前缀和包查找路径问题。

本章沿用 [11_testing](../exercises/11_testing/CMakeLists.txt) 中的 `calculator` 库，并创建独立的 [12_package_consumer](../exercises/12_package_consumer/CMakeLists.txt) 工程。

## 构建和安装的区别

| 目录 | 用途 |
|---|---|
| 源码目录 | 保存源码、头文件和 CMake 配置 |
| 构建目录 `build/` | 保存缓存、目标文件、库和测试程序 |
| 安装目录 `install/` | 按安装规则存放供使用者使用的文件 |

三个步骤各有职责：

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix ./install
```

配置阶段生成构建规则和安装规则；构建阶段生成产物；安装阶段按规则复制产物。`cmake --install` 不会自动替你构建，所以需要先完成构建。

本章命令使用单配置生成器。多配置生成器应在构建和安装时选择一致的配置，例如分别使用 `--config Release`。

## 安装库和头文件

基础安装规则如下，写在库工程的顶层 `CMakeLists.txt` 中：

```cmake
install(
    TARGETS calculator
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    FILES calculator.hpp
    DESTINATION include
)
```

| 产物类别 | 常见对应文件 |
|---|---|
| `ARCHIVE` | 静态库，例如 `libcalculator.a`；Windows 导入库也属于此类 |
| `LIBRARY` | 共享库，例如 Linux 的 `.so` |
| `RUNTIME` | 可执行程序、Windows 的 `.dll` |

本次构建的是静态库，使用 `ARCHIVE DESTINATION lib`。相对 `DESTINATION` 路径相对于安装前缀解析，因此在 `exercises/11_testing` 中执行：

```bash
cmake --install build --prefix ./install
```

得到：

```text
install/
├── include/
│   └── calculator.hpp
└── lib/
    └── libcalculator.a
```

`test_math` 不会自动安装，因为没有为它指定安装规则：

| 命令 | 职责 |
|---|---|
| `add_executable()` | 创建程序的构建目标 |
| `add_test()` | 注册测试 |
| `install()` | 指定安装的产物和位置 |

安装日志中的 `Up-to-date` 表示相应文件无须更新。`Install configuration: ""` 表示此次安装的配置名称为空；在没有设置 `CMAKE_BUILD_TYPE` 的单配置构建中，这是正常现象。

## 先手动验证安装产物

本次先使用 `exercises/11_testing/consumer.cpp` 验证库：

```cpp
#include "calculator.hpp"
#include <iostream>

int main()
{
    std::cout << add(10, 20) << '\n';
    return 0;
}
```

在 `exercises/11_testing` 中编译并运行：

```bash
g++ consumer.cpp \
    -I./install/include \
    -L./install/lib \
    -lcalculator \
    -o consumer
./consumer
```

实际输出为 `30`。`-I` 指定头文件搜索目录，`-L` 指定库搜索目录，`-lcalculator` 在本例链接 `libcalculator.a`。

编译时需要函数声明，链接时需要函数实现。后面的 CMake 包将这些使用要求交给导出的 target 管理，使用者无须手动填写这些参数。

## 区分构建接口和安装接口

原来的头文件配置直接指向源码目录：

```cmake
target_include_directories(calculator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

安装包不应依赖库作者机器上的源码路径，因此改为：

```cmake
target_include_directories(calculator
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(calculator PUBLIC cxx_std_17)
```

`$<...>` 是生成器表达式。这两个表达式区分不同使用场景：

| 表达式 | 本例用途 |
|---|---|
| `BUILD_INTERFACE` | 当前构建树中的使用者使用源码目录 |
| `INSTALL_INTERFACE` | 安装导出的 target 使用安装前缀下的 `include` |

`INSTALL_INTERFACE:include` 相对于安装前缀，不是相对于使用者的源码目录。`PUBLIC` 的头文件目录和 C++ 标准要求会成为导出 target 的使用要求。

## 导出 target

将库的安装规则扩展为：

```cmake
install(
    TARGETS calculator
    EXPORT CalculatorTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
```

`EXPORT CalculatorTargets` 将该 target 加入导出集合。然后安装这个集合的描述文件：

```cmake
install(
    EXPORT CalculatorTargets
    FILE CalculatorTargets.cmake
    NAMESPACE Calculator::
    DESTINATION lib/cmake/Calculator
)
```

两处 `EXPORT` 的作用不同：前者收集 target，后者生成并安装导出描述文件。

当前工程内部仍使用 `calculator`，使用者看到的导入目标名称是 `Calculator::calculator`。描述文件记录库位置和使用要求，使用者链接它不会重新编译库源码。

本次安装生成了 `CalculatorTargets.cmake` 和 `CalculatorTargets-noconfig.cmake`。后者保存对应配置的产物信息；配置不同时，后缀也可能是 `debug` 或 `release`。

## 生成包入口和版本文件

仅有 Targets 文件还不够，`find_package(Calculator CONFIG ...)` 需要包入口。

先在顶层设置版本：

```cmake
project(stage11_testing VERSION 1.0.0 LANGUAGES CXX)
```

新建 `cmake/CalculatorConfig.cmake.in`：

```cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/CalculatorTargets.cmake")

check_required_components(Calculator)
```

`@PACKAGE_INIT@` 会被辅助函数替换为初始化代码。`CMAKE_CURRENT_LIST_DIR` 指当前正在执行的 `.cmake` 文件所在目录，用于加载包入口旁边的 Targets 文件。它不同于表示源码目录的 `CMAKE_CURRENT_SOURCE_DIR`。

`check_required_components()` 检查请求的必需组件；本例没有定义组件，保留标准检查。

在顶层追加：

```cmake
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CalculatorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/Calculator
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfigVersion.cmake"
    DESTINATION lib/cmake/Calculator
)
```

`configure_package_config_file()` 生成包入口；`write_basic_package_version_file()` 生成版本匹配规则；`install(FILES ...)` 将二者安装到包目录。

### configure_package_config_file() 的参数

这个函数根据模板生成包入口，三个主要参数分别说明“模板在哪、生成到哪、将来安装到哪”：

| 参数 | 本例的值 | 含义 |
|---|---|---|
| 第一个位置参数 | `${CMAKE_CURRENT_SOURCE_DIR}/cmake/CalculatorConfig.cmake.in` | 源码目录中的输入模板 |
| 第二个位置参数 | `${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfig.cmake` | 配置阶段生成的输出文件 |
| `INSTALL_DESTINATION` | `lib/cmake/Calculator` | 包入口预期的安装位置，相对路径以安装前缀为基准 |

`.cmake.in` 是作者编写的模板，生成的 `.cmake` 才是供使用者加载的包入口。处理模板时，`@PACKAGE_INIT@` 会替换为初始化代码，包括默认提供的辅助宏。

模板中的 `${CMAKE_CURRENT_LIST_DIR}` 会保留在生成文件中，在使用者加载包入口时求值。因此下面的语句能找到安装后与包入口位于同一目录的 Targets 文件：

```cmake
include("${CMAKE_CURRENT_LIST_DIR}/CalculatorTargets.cmake")
```

`INSTALL_DESTINATION` 用于生成与安装布局有关的初始化代码，帮助包在移动位置后仍能定位文件。它不会执行安装，也不会把输出文件直接写入安装目录。

实际安装仍由 `install(FILES ...)` 完成，其 `DESTINATION` 应与这里的 `INSTALL_DESTINATION` 保持一致。本例二者均为 `lib/cmake/Calculator`。

完整流程为：

```text
源码目录：cmake/CalculatorConfig.cmake.in
    ↓ configure_package_config_file()，配置阶段生成
构建目录：CalculatorConfig.cmake
    ↓ cmake --install，按 install(FILES ...) 规则安装
安装目录：lib/cmake/Calculator/CalculatorConfig.cmake
    ↓ find_package(Calculator CONFIG REQUIRED)
加载包入口及导出的 target
```

常用可选参数如下，本练习保留默认行为即可：

| 参数 | 用途 |
|---|---|
| `PATH_VARS` | 为指定的安装路径变量生成 `PACKAGE_<变量名>` 辅助变量，供模板通过 `@PACKAGE_<变量名>@` 使用，帮助路径随安装位置变化 |
| `INSTALL_PREFIX` | 指定计算相对路径时使用的安装前缀，必须是绝对路径；默认使用 `CMAKE_INSTALL_PREFIX` |
| `NO_SET_AND_CHECK_MACRO` | 不生成 `set_and_check()` 辅助宏 |
| `NO_CHECK_REQUIRED_COMPONENTS_MACRO` | 不生成 `check_required_components()` 辅助宏 |

本例模板调用了 `check_required_components()`，因此不应禁用该宏的生成，除非自行提供对应实现。

`INSTALL_PREFIX` 是配置阶段生成包文件时使用的参数；`cmake --install ... --prefix ...` 则是在安装阶段选择实际安装根目录，不要混淆二者。

参数定义参见 [CMake 官方文档：生成包配置文件](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-configuration-file)。

### write_basic_package_version_file() 的参数

这个函数生成供 `find_package()` 使用的版本检查文件。结合上面的调用，各参数含义如下：

| 参数 | 本例的值 | 含义 |
|---|---|---|
| 第一个位置参数 | `${CMAKE_CURRENT_BINARY_DIR}/CalculatorConfigVersion.cmake` | 生成文件的位置，本例位于 `build/` 中 |
| `VERSION` | `${PROJECT_VERSION}`，即 `1.0.0` | 库作者当前提供的包版本 |
| `COMPATIBILITY` | `SameMajorVersion` | 声明版本匹配规则 |

第一个参数是输出文件路径，不是安装目录。函数在配置阶段生成文件，之后由 `install(FILES ...)` 安装它。

`PROJECT_VERSION` 来自前面的声明：

```cmake
project(stage11_testing VERSION 1.0.0 LANGUAGES CXX)
```

因此 `VERSION ${PROJECT_VERSION}` 相当于 `VERSION 1.0.0`。要区分提供版本和请求版本：

```cmake
# 库作者：提供版本 1.0.0，写入版本文件
# VERSION ${PROJECT_VERSION}

# 使用者：请求兼容 1.0 的包
find_package(Calculator 1.0 CONFIG REQUIRED)
```

对于本例没有 `EXACT` 的单一版本请求，`SameMajorVersion` 要求主版本相同，且安装版本不低于请求版本。当前 `1.0.0` 可以满足请求 `1.0`，不能满足 `1.1` 或 `2.0`。

假设以后安装的包版本升级为 **1.2.3**，匹配结果如下：

| 使用者请求的版本 | 是否符合版本规则 | 原因 |
|---|---|---|
| `1.0` | 是 | 同为主版本 1，安装版本更高 |
| `1.2` | 是 | 同为主版本 1，安装版本不低于请求版本 |
| `1.3` | 否 | 安装版本低于请求版本 |
| `2.0` | 否 | 主版本不同 |

这是库作者声明的兼容策略，CMake 不会自动检查源码或二进制是否真的保持兼容。

还有一个可选参数 `ARCH_INDEPENDENT`，用于纯头文件库等不包含架构相关二进制的包，使生成的版本文件跳过架构兼容检查。本例包含编译后的静态库，因此不添加该参数。

参数定义参见 [CMake 官方文档：生成包版本文件](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#generating-a-package-version-file)。

### 安装结果与查找过程

最终安装结构为：

```text
install/
├── include/calculator.hpp
└── lib/
    ├── libcalculator.a
    └── cmake/Calculator/
        ├── CalculatorConfig.cmake
        ├── CalculatorConfigVersion.cmake
        ├── CalculatorTargets.cmake
        └── CalculatorTargets-noconfig.cmake
```

请求版本时，查找过程可理解为：

```text
find_package(Calculator 1.0 CONFIG REQUIRED)
    → 查找候选包并检查 ConfigVersion 中的版本规则
    → 加载匹配的 Config 包入口
    → 包入口加载 Targets
    → 提供 Calculator::calculator
```

## 独立使用者工程

`exercises/12_package_consumer` 的结构为：

```text
12_package_consumer/
├── CMakeLists.txt
├── main.cpp
└── README.md
```

`main.cpp` 与前面的使用者程序一样，调用 `add(10, 20)` 并输出结果。其 `CMakeLists.txt` 为：

```cmake
cmake_minimum_required(VERSION 3.16)

project(stage12_package_consumer LANGUAGES CXX)

find_package(Calculator 1.0 CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE Calculator::calculator)
```

这个工程没有通过 `add_subdirectory()` 引入库源码。它只使用安装包，头文件目录、库位置和 C++ 标准要求均来自导入 target。

以下完整流程在**仓库根目录**执行：

```bash
cmake -S exercises/11_testing -B exercises/11_testing/build -DBUILD_TESTS=ON
cmake --build exercises/11_testing/build
ctest --test-dir exercises/11_testing/build --output-on-failure
cmake --install exercises/11_testing/build \
    --prefix "$PWD/exercises/11_testing/install"

cmake -S exercises/12_package_consumer -B exercises/12_package_consumer/build \
    -DCMAKE_PREFIX_PATH="$PWD/exercises/11_testing/install"
cmake --build exercises/12_package_consumer/build
./exercises/12_package_consumer/build/consumer
```

该流程已在临时构建目录验证：库的测试通过，安装成功，独立使用者构建后输出 `30`。

## 包查找路径与本次排错

| 参数 | 应指向的位置 |
|---|---|
| `CMAKE_PREFIX_PATH` | 安装根目录，例如 `/workspace/exercises/11_testing/install` |
| `Calculator_DIR` | 包入口所在目录，例如 `/workspace/exercises/11_testing/install/lib/cmake/Calculator` |

本次容器内查找失败，是因为传入了宿主机的 `/home/demo/study/cmake/...` 路径，而且路径末尾混入了多余字符。工具在容器中执行时，应使用容器能访问的路径。

在容器的 `exercises/12_package_consumer` 目录中，可以这样配置：

```bash
cmake -S . -B build \
    -DCMAKE_PREFIX_PATH=/workspace/exercises/11_testing/install
```

也可以直接指定包目录：

```bash
cmake -S . -B build \
    -DCalculator_DIR=/workspace/exercises/11_testing/install/lib/cmake/Calculator
```

如果仍找不到包，检查该位置是否存在 `CalculatorConfig.cmake`。只安装 Targets 文件还不够；添加包入口规则后，要重新配置并安装。请求版本时，还应检查版本文件及兼容规则。

修正路径时可以再次传入 `-D` 更新缓存，不必直接删除整个构建目录。若曾显式设置错误的 `Calculator_DIR`，也应检查并更新它。

## 更新库后的操作

安装目录保存的是上次安装时的产物副本。修改库实现后，需要：

```text
重新构建库 → 重新安装库 → 重新构建使用者
```

本例使用静态链接，库实现会链接进 `consumer`。仅更新安装目录中的静态库不会改变已经生成的可执行程序，使用者还需要重新链接。

本章完成了从“安装头文件和库”到“导出 target 并通过 find_package 使用”的完整流程。
