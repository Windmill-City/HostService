cmake_minimum_required(VERSION 3.18)
project(frozen)

# ##############################################################################
# Build files
# ##############################################################################
# includes
set(INCLUDES frozen/include)

# ##############################################################################
# Targets
# ##############################################################################

# Library Target
target_include_directories(firmware.elf PUBLIC ${INCLUDES})
