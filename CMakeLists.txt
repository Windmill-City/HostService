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
add_library(HostService STATIC ${SOURCES})
target_include_directories(HostService PUBLIC ${INCLUDES})
target_compile_definitions(HostService PUBLIC NO_LOCK=1)

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
add_subdirectory(test/gtest EXCLUDE_FROM_ALL)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "x64 Arch is not supported! Use x86/32bit compiler!")
endif()

# ##############################################################################
# 源文件
# ##############################################################################
# C/CPP文件
file(GLOB TSOURCES test/src/*.cpp)

# 包含目录
set(TINCLUDES test/include)

# ##############################################################################
# 构建目标
# ##############################################################################
add_executable(THostService ${TSOURCES})
target_include_directories(THostService PRIVATE ${TINCLUDES})
target_link_libraries(THostService PRIVATE HostService gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(THostService)
