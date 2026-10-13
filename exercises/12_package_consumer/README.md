# 使用安装后的 Calculator 包

这个工程通过 `find_package()` 查找安装包，链接导出的
`Calculator::calculator` target。头文件目录和 C++ 标准要求由该 target
传递，无须手动设置 `-I`、`-L` 或添加库的源码目录。

以下命令在仓库根目录执行，适用于本次使用的单配置生成器。

## 构建并安装库

```bash
cmake -S exercises/11_testing -B exercises/11_testing/build -DBUILD_TESTS=ON
cmake --build exercises/11_testing/build
ctest --test-dir exercises/11_testing/build --output-on-failure
cmake --install exercises/11_testing/build --prefix "$PWD/exercises/11_testing/install"
```

## 构建使用者工程

```bash
cmake -S exercises/12_package_consumer -B exercises/12_package_consumer/build \
    -DCMAKE_PREFIX_PATH="$PWD/exercises/11_testing/install"
cmake --build exercises/12_package_consumer/build
./exercises/12_package_consumer/build/consumer
```

预期输出 `30`。

`CMAKE_PREFIX_PATH` 指向安装前缀，CMake 在其下查找
`lib/cmake/Calculator/CalculatorConfig.cmake`。也可以通过
`Calculator_DIR` 直接指定包含该文件的目录。

`find_package(Calculator 1.0 CONFIG REQUIRED)` 请求兼容 1.0 的包；本例
安装版本为 1.0.0。包入口加载 `CalculatorTargets.cmake`，提供 imported
target `Calculator::calculator`。使用者只使用安装产物，不会重新编译库源码。

库实现更新后，需要重新构建并安装库，再重新构建使用者程序。
