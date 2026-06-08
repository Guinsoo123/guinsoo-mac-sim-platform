# Guinsoo Mac 机器人仿真平台

这是 Mac 全栈机器人仿真算法验证平台的工具链环境 MVP。第一版使用 Qt/C++ 桌面 GUI 管理 Docker Desktop 中的 Ubuntu 24.04 ARM64 容器，容器内提供 ROS 2 Jazzy、Gazebo Harmonic、MoveIt 2 Jazzy 和 Foxglove Bridge。

## 本地构建 Qt 应用

```bash
./scripts/build-mac-app.sh
./scripts/run-mac-app.sh
```

## Docker 工具链

Docker Desktop 安装并启动后，可构建工具链镜像：

```bash
./scripts/docker-build-toolchain.sh
```

构建脚本默认使用 `http://mirrors.aliyun.com/ubuntu-ports/` 作为 Ubuntu ARM64 软件源，并使用 `https://mirrors.aliyun.com/ros2/ubuntu` 作为 ROS 2 软件源，减少国内网络下访问官方源的不稳定。需要切换回官方源或自定义源时，可以覆盖环境变量：

```bash
UBUNTU_PORTS_MIRROR=http://ports.ubuntu.com/ubuntu-ports/ ./scripts/docker-build-toolchain.sh

IMAGE_TAG=guinsoo/robot-sim:jazzy-arm64 \
UBUNTU_PORTS_MIRROR=http://mirrors.aliyun.com/ubuntu-ports/ \
ROS_APT_MIRROR=https://mirrors.aliyun.com/ros2/ubuntu \
OSRF_APT_MIRROR=http://packages.osrfoundation.org/gazebo/ubuntu-stable \
./scripts/docker-build-toolchain.sh
```

启动容器：

```bash
docker run --rm -it \
  --name guinsoo-robot-sim \
  -p 8765:8765 \
  -e ROS_DOMAIN_ID=0 \
  -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
  -e FOXGLOVE_PORT=8765 \
  -v "$PWD/workspace:/workspace/robot_ws" \
  guinsoo/robot-sim:jazzy-arm64
# 或者
./scripts/docker-run-toolchain.sh ./workspace
```

容器内启动示例：

```bash
/opt/guinsoo/launch_demo.sh
```

## 当前范围

- 已实现：中文设计文档、Qt/C++ 五页 GUI 骨架、配置合并、诊断规则、Docker 命令预览、Foxglove advertise 消息解析、Docker 工具链文件。
- 暂缓：完整 Gazebo/RViz GUI 嵌入、rosbag 分析、多 Docker 运行时、Intel Mac、macOS 端 DDS 直连 ROS 2 节点。
