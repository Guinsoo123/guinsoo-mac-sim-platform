#!/usr/bin/env bash
set -euo pipefail

WORKSPACE_PATH="${1:-$PWD/workspace}"
mkdir -p "${WORKSPACE_PATH}"

docker run --rm -it \
  --name guinsoo-robot-sim \
  -p 8765:8765 \
  -e ROS_DOMAIN_ID=0 \
  -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
  -e FOXGLOVE_PORT=8765 \
  -v "${WORKSPACE_PATH}:/workspace/robot_ws" \
  guinsoo/robot-sim:jazzy-arm64
