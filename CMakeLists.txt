cmake_minimum_required(VERSION 3.24)

project(GuinsooMacSimPlatform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebSockets)

add_library(RobotSimCore
    src/core/AppConfig.cpp
    src/core/ConfigStore.cpp
    src/core/Diagnostics.cpp
    src/core/DockerController.cpp
    src/core/FoxgloveProtocol.cpp
)

target_include_directories(RobotSimCore PUBLIC src)
target_link_libraries(RobotSimCore PUBLIC Qt6::Core Qt6::WebSockets)

add_executable(GuinsooMacSimPlatform
    src/app/main.cpp
    src/gui/MainWindow.cpp
)

target_include_directories(GuinsooMacSimPlatform PRIVATE src)
target_link_libraries(GuinsooMacSimPlatform PRIVATE RobotSimCore Qt6::Widgets)

enable_testing()

add_executable(RobotSimCoreTests
    tests/core_tests.cpp
)

target_include_directories(RobotSimCoreTests PRIVATE src)
target_link_libraries(RobotSimCoreTests PRIVATE RobotSimCore Qt6::Core)

add_test(NAME RobotSimCoreTests COMMAND RobotSimCoreTests)
