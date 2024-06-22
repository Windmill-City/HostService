cmake_minimum_required(VERSION 3.18)
project(frozen)

# ##############################################################################
# Build files
# ##############################################################################
# includes
set(INCLUDES include)

# ##############################################################################
# Targets
# ##############################################################################

# Library Target
target_include_directories(HostService PUBLIC ${INCLUDES})
target_compile_definitions(HostService PUBLIC FROZEN_NO_EXCEPTIONS=1)
