cmake_minimum_required(VERSION 3.18)
project(HostService)

# ##############################################################################
# 编译选项
# ##############################################################################
# 使用UTF-8编码
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(HOST_SERVICE_TEST "Compile Test" OFF)

# ##############################################################################
# 依赖
# ##############################################################################
add_subdirectory(libs/libcrc EXCLUDE_FROM_ALL)

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
add_library(host_service STATIC ${SOURCES})
target_include_directories(host_service PUBLIC ${INCLUDES})
target_link_libraries(host_service PRIVATE libcrc)

# ##############################################################################
# 测试
# ##############################################################################
if(HOST_SERVICE_TEST)
  include(CTest)
  enable_testing()
  add_subdirectory(test build/test)
endif(HOST_SERVICE_TEST)
