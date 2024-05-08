cmake_minimum_required(VERSION 3.18)
project(HostService)

# ##############################################################################
# 编译选项
# ##############################################################################
# 使用UTF-8编码
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_HOST_SERVICE_TEST "Build HostService Test" OFF)

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
add_library(firmware.elf STATIC ${SOURCES})
target_include_directories(firmware.elf PUBLIC ${INCLUDES})

# ##############################################################################
# 依赖
# ##############################################################################
add_subdirectory(libs/libcrc EXCLUDE_FROM_ALL)
add_subdirectory(libs/uAES EXCLUDE_FROM_ALL)
add_subdirectory(libs/frozen EXCLUDE_FROM_ALL)

# ##############################################################################
# 测试
# ##############################################################################
if(BUILD_HOST_SERVICE_TEST)
  include(CTest)
  enable_testing()
  add_subdirectory(test build/test)
endif(BUILD_HOST_SERVICE_TEST)
