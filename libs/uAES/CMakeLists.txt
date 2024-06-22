cmake_minimum_required(VERSION 3.18)
project(uAES)

# ##############################################################################
# Build files
# ##############################################################################
# C/C++ Sources
file(GLOB SOURCES src/*.c)

# includes
set(INCLUDES include)

# ##############################################################################
# Targets
# ##############################################################################

# Library Target
target_sources(HostService PRIVATE ${SOURCES})
target_include_directories(HostService PUBLIC ${INCLUDES})
target_compile_definitions(
  HostService PUBLIC UAES_ENABLE_ALL=0 UAES_ENABLE_256=1 UAES_KEY_CONFIG=1
                     UAES_SBOX_CONFIG=1 UAES_32BIT_CONFIG=1 UAES_ENABLE_CCM=1
)
