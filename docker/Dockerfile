# syntax=docker/dockerfile:1.7
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV ROS_DISTRO=jazzy
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ENV FOXGLOVE_PORT=8765
ENV APT_FLAGS="-o Acquire::Retries=8 -o Acquire::http::Timeout=45 -o Acquire::https::Timeout=45"
ENV APT_INSTALL_RETRIES=4

SHELL ["/bin/bash", "-c"]

ARG UBUNTU_PORTS_MIRROR=http://ports.ubuntu.com/ubuntu-ports/
ARG ROS_APT_MIRROR=https://mirrors.aliyun.com/ros2/ubuntu
ARG OSRF_APT_MIRROR=http://packages.osrfoundation.org/gazebo/ubuntu-stable

RUN sed -i "s|http://ports.ubuntu.com/ubuntu-ports/|${UBUNTU_PORTS_MIRROR%/}/|g" /etc/apt/sources.list.d/ubuntu.sources

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get ${APT_FLAGS} update && apt-get ${APT_FLAGS} install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    locales \
    lsb-release \
    software-properties-common \
    && locale-gen en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

RUN add-apt-repository universe \
    && curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
      -o /usr/share/keyrings/ros-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] ${ROS_APT_MIRROR} $(. /etc/os-release && echo ${UBUNTU_CODENAME}) main" \
      > /etc/apt/sources.list.d/ros2.list \
    && curl -sSL https://packages.osrfoundation.org/gazebo.gpg \
      -o /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] ${OSRF_APT_MIRROR} $(. /etc/os-release && echo ${UBUNTU_CODENAME}) main" \
      > /etc/apt/sources.list.d/gazebo-stable.list

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    for attempt in $(seq 1 "${APT_INSTALL_RETRIES}"); do \
      apt-get ${APT_FLAGS} update && apt-get ${APT_FLAGS} install -y --no-install-recommends \
        gz-harmonic \
        python3-colcon-common-extensions \
        python3-rosdep \
        ros-jazzy-demo-nodes-cpp \
        ros-jazzy-foxglove-bridge \
        ros-jazzy-ros-base \
        ros-jazzy-rmw-cyclonedds-cpp \
        ros-jazzy-ros-gz \
        && exit 0; \
      if [[ "${attempt}" == "${APT_INSTALL_RETRIES}" ]]; then exit 1; fi; \
      echo "apt install failed, retrying attempt $((attempt + 1))/${APT_INSTALL_RETRIES}" >&2; \
      sleep $((attempt * 15)); \
    done

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    for attempt in $(seq 1 "${APT_INSTALL_RETRIES}"); do \
      apt-get ${APT_FLAGS} update && apt-get ${APT_FLAGS} install -y --no-install-recommends \
        ros-jazzy-moveit \
        && exit 0; \
      if [[ "${attempt}" == "${APT_INSTALL_RETRIES}" ]]; then exit 1; fi; \
      echo "apt install failed, retrying attempt $((attempt + 1))/${APT_INSTALL_RETRIES}" >&2; \
      sleep $((attempt * 15)); \
    done

RUN (rosdep init || true) \
    && rosdep update

WORKDIR /workspace/robot_ws

COPY docker/entrypoint.sh /opt/guinsoo/entrypoint.sh
COPY docker/launch_demo.sh /opt/guinsoo/launch_demo.sh
COPY docker/bridge-config.yaml /opt/guinsoo/bridge-config.yaml

RUN chmod +x /opt/guinsoo/entrypoint.sh /opt/guinsoo/launch_demo.sh

ENTRYPOINT ["/opt/guinsoo/entrypoint.sh"]
CMD ["bash"]
