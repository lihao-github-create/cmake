# CMake 学习仓库

本仓库用于按照 [CMake 学习指南](learn_guide.md) 逐步练习现代 CMake，并提供一个基于 Ubuntu 22.04 的通用 C/C++ 开发环境。

容器内预装 GCC、Clang、CMake、GNU Make、Ninja、GDB、Git、pkg-config 和 Python 3。仓库目录会挂载到容器内的 `/workspace`。

## 准备条件

宿主机需要安装 Docker，并且支持 Compose v2。可以使用以下命令检查：

```bash
docker --version
docker compose version
```

## 创建并启动开发环境

在仓库根目录执行：

```bash
DEV_UID=$(id -u) DEV_GID=$(id -g) docker compose up -d --build
```

该命令会依次完成以下工作：

1. 根据 [Dockerfile](Dockerfile) 构建 `cmake-study:ubuntu22.04` 镜像。
2. 根据 [compose.yaml](compose.yaml) 创建并启动 `dev` 服务。
3. 将当前仓库挂载到容器的 `/workspace`。
4. 让容器进程使用当前宿主机用户的 UID 和 GID，避免生成属于 `root` 的文件。

其中：

- `-d` 表示让容器在后台运行。
- `--build` 表示启动前先构建或更新镜像；Docker 仍会使用可用的构建缓存。

查看容器状态：

```bash
docker compose ps
```

## 进入开发环境

```bash
docker compose exec dev bash
```

进入后，当前目录是 `/workspace`。可以先检查工具是否可用：

```bash
cmake --version
g++ --version
clang++ --version
make --version
ninja --version
gdb --version
```

当练习目录中存在 `CMakeLists.txt` 后，可以使用标准构建流程：

```bash
cmake -S . -B build
cmake --build build
```

## 退出和停止环境

退出容器 shell：

```bash
exit
```

停止并删除开发容器及 Compose 创建的网络：

```bash
docker compose down
```

仓库文件位于宿主机，不会随容器删除。构建镜像默认也会保留，以便下次复用。

## 更新或重新构建环境

修改 `Dockerfile` 后，重新构建并启动：

```bash
DEV_UID=$(id -u) DEV_GID=$(id -g) docker compose up -d --build
```

需要完全忽略构建缓存时：

```bash
docker compose build --no-cache
DEV_UID=$(id -u) DEV_GID=$(id -g) docker compose up -d
```

更多说明参见 [CMake 通用学习环境](doc/00_development_environment.md)。
