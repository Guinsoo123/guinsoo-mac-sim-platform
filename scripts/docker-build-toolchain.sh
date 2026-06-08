#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG="${IMAGE_TAG:-guinsoo/robot-sim:jazzy-arm64}"
UBUNTU_PORTS_MIRROR="${UBUNTU_PORTS_MIRROR:-http://mirrors.aliyun.com/ubuntu-ports/}"
ROS_APT_MIRROR="${ROS_APT_MIRROR:-https://mirrors.aliyun.com/ros2/ubuntu}"
OSRF_APT_MIRROR="${OSRF_APT_MIRROR:-http://packages.osrfoundation.org/gazebo/ubuntu-stable}"

docker build --platform linux/arm64 \
  --build-arg "UBUNTU_PORTS_MIRROR=${UBUNTU_PORTS_MIRROR}" \
  --build-arg "ROS_APT_MIRROR=${ROS_APT_MIRROR}" \
  --build-arg "OSRF_APT_MIRROR=${OSRF_APT_MIRROR}" \
  -t "${IMAGE_TAG}" \
  -f docker/Dockerfile .
