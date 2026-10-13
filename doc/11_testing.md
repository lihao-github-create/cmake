# 第十一阶段：使用 CTest 测试

## 学习目标

- 区分测试程序、构建工具和 CTest 的职责。
- 使用 `enable_testing()` 和 `add_test()` 注册测试。
- 查看测试列表、运行结果及失败输出。
- 使用 `BUILD_TESTS` 控制测试模块。
- 将待测库和测试程序分开组织。

## CMake、测试程序和 CTest 的分工

| 角色 | 职责 |
|---|---|
| CMake | 配置工程，生成构建规则和测试注册信息 |
| 构建工具和编译器 | 构建库及测试可执行程序 |
| 测试程序 | 调用待测代码，检查结果并返回退出码 |
| CTest | 运行注册的测试，汇总成功或失败 |

本阶段使用普通 C++ 程序作为测试程序，无须引入 GoogleTest。在本练习的默认设置下，程序正常退出且退出码为 `0` 表示成功，非零表示失败。程序崩溃或超时也会导致测试失败。

CTest 不会根据输出中的 `Test passed` 或 `Expected 5` 判断计算是否正确；判断逻辑需要写在测试程序中。

## 注册测试

```cmake
enable_testing()

add_executable(test_math test_math.cpp)

add_test(
    NAME math_test
    COMMAND test_math
)
```

- `enable_testing()` 启用当前目录及其子目录的测试，通常放在顶层 `CMakeLists.txt`。
- `add_executable()` 创建可执行 target，描述测试程序如何构建。
- `add_test()` 注册测试，描述 CTest 应运行什么。

`math_test` 是 CTest 中的测试名称，`test_math` 是可执行 target 的名称。当 `COMMAND` 指定可执行 target 时，CMake 会处理它对应的程序路径，无须手写 `./build/test_math`。

仅创建可执行程序不会自动注册测试；仅注册测试也不意味着程序已经编译完成。

## 最终练习结构

本次练习从单个测试程序开始，最终将待测库和测试分离：

```text
exercises/11_testing/
├── CMakeLists.txt
├── calculator.hpp
├── calculator.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_math.cpp
```

`calculator.hpp`：

```cpp
#pragma once

int add(int a, int b);
```

`calculator.cpp`：

```cpp
#include "calculator.hpp"

int add(int a, int b)
{
    return a + b;
}
```

`tests/test_math.cpp`：

```cpp
#include "calculator.hpp"
#include <iostream>

int main()
{
    const int actual = add(2, 3);

    if (actual != 5) {
        std::cerr << "Expected 5, got " << actual << '\n';
        return 1;
    }

    std::cout << "Test passed\n";
    return 0;
}
```

测试通过头文件调用实际库中的实现，不在测试文件中重复定义 `add()`。这里使用明确的条件检查，即使 Release 配置定义了 `NDEBUG`，检查也会执行。

顶层 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.16)

project(stage11_testing LANGUAGES CXX)

add_library(calculator calculator.cpp)

target_include_directories(calculator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(calculator PUBLIC cxx_std_17)

option(BUILD_TESTS "Build tests" ON)

enable_testing()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
```

`tests/CMakeLists.txt`：

```cmake
add_executable(test_math test_math.cpp)

target_link_libraries(test_math PRIVATE calculator)

add_test(
    NAME math_test
    COMMAND test_math
)
```

`calculator` 通过 `PUBLIC` 传递头文件目录和 C++ 标准要求。`test_math` 链接该库后获得这些使用要求，不需要重复配置。

## 配置、构建、运行测试

以下命令均在 `exercises/11_testing` 目录执行：

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

预期输出类似：

```text
1/1 Test #1: math_test ... Passed

100% tests passed, 0 tests failed out of 1
```

`--test-dir build` 指定从哪个构建目录查找并运行测试；`--output-on-failure` 在失败时显示测试程序输出。

本练习这种直接调用 CTest 的方式不会自动编译测试程序。修改 C++ 源码后，必须先重新构建，再运行测试，否则可能运行旧程序。

## 查看测试列表和详细输出

| 命令 | 作用 |
|---|---|
| `ctest --test-dir build -N` | 只列出测试，不执行 |
| `ctest --test-dir build --output-on-failure` | 运行测试，失败时显示程序输出 |
| `ctest --test-dir build -V` | 运行测试，显示实际命令和详细输出 |

本次 `-V` 输出包含：

```text
1: Test passed
1/1 Test #1: math_test ... Passed
```

详细日志还会显示测试程序的绝对路径。拆分到 `tests` 子目录后，在本练习默认布局下，程序位于 `build/tests/test_math`。

成功时，`--output-on-failure` 不显示程序的 `Test passed` 输出，这是正常行为。

## 观察失败

要验证测试能发现错误，可以临时将 `calculator.cpp` 中的实现改为：

```cpp
return a - b;
```

保留测试中的预期值 `5`，重新构建并测试：

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

预期结果类似：

```text
1/1 Test #1: math_test ... ***Failed
Expected 5, got -1

0% tests passed, 1 tests failed out of 1
```

原因是测试检查发现 `-1` 不等于 `5`，返回退出码 `1`，CTest 据此判定失败。观察完成后，将实现恢复为 `a + b`，重新构建并确认通过。

## 用 BUILD_TESTS 控制测试模块

本练习的 `BUILD_TESTS` 是通过 `option()` 自定义的缓存选项。开启时，CMake 处理 `tests` 子目录，创建 `test_math` target 并注册 `math_test`。

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build -N
```

测试数量为：

```text
Total Tests: 1
```

关闭后重新配置：

```bash
cmake -S . -B build -DBUILD_TESTS=OFF
ctest --test-dir build -N
```

测试数量变为：

```text
Total Tests: 0
```

此时运行测试会提示 `No tests were found!!!`。这是没有注册测试的结果，不表示库构建失败，也不能视为测试已验证通过。

即使旧的测试可执行文件仍在构建目录中，它也不再是本次配置注册的测试。CTest 根据注册信息运行测试，不会扫描并执行目录中的所有程序。

## 关闭测试，库仍然参与构建

顶层结构是：

```cmake
add_library(calculator calculator.cpp)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
```

`add_library()` 在条件之外，所以关闭测试不会将 `calculator` 排除出默认构建。

还要区分“参与构建”和“实际重新编译”：构建工具会检查目标是否需要更新，已是最新状态时可以复用产物，没有编译输出并不意味着该目标被排除。

本次关闭测试后执行：

```bash
cmake --build build --clean-first
```

先清理再构建，实际观察到：

```text
Building CXX object CMakeFiles/calculator.dir/calculator.cpp.o
Linking CXX static library libcalculator.a
Built target calculator
```

库正常构建，没有构建 `test_math`。这正是“库正常构建，测试按需开启”的组织方式。

## 后续学习

GoogleTest 可以帮助组织测试用例和编写断言；CTest 负责运行注册的测试，两者可以配合使用。本阶段先掌握普通 C++ 测试程序即可。

练习结束时，可以恢复开启测试的状态：

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

日常修改代码后，保持“先构建，再测试”的顺序。
