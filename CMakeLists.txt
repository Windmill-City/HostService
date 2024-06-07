cmake_minimum_required(VERSION 3.18)
project(HostService)

# ##############################################################################
# 编译选项
# ##############################################################################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Werror -Wall -Wextra)

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
target_compile_definitions(firmware.elf PUBLIC NO_LOCK=1)

# ##############################################################################
# 依赖
# ##############################################################################
add_subdirectory(libs/libcrc EXCLUDE_FROM_ALL)
add_subdirectory(libs/uAES EXCLUDE_FROM_ALL)
add_subdirectory(libs/frozen EXCLUDE_FROM_ALL)

# ##############################################################################
# 测试
# ##############################################################################
include(CTest)
enable_testing()
add_subdirectory(test build/test)
