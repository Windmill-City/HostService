cmake_minimum_required(VERSION 3.18)
project(HostServiceTest)

# ##############################################################################
# 依赖
# ##############################################################################
add_subdirectory(gtest EXCLUDE_FROM_ALL)

# ##############################################################################
# 源文件
# ##############################################################################
# C/CPP文件
file(GLOB SOURCES src/*.cpp)

# 包含目录
set(INCLUDES include)

# ##############################################################################
# 构建目标
# ##############################################################################
add_executable(host_service_test ${SOURCES})
target_include_directories(host_service_test PRIVATE ${INCLUDES})
target_link_libraries(host_service_test PRIVATE gtest gtest_main host_service)

include(GoogleTest)
gtest_discover_tests(host_service_test)
