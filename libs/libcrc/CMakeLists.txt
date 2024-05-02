cmake_minimum_required(VERSION 3.18)

# ##############################################################################
# Sources
# ##############################################################################
aux_source_directory(libcrc/src SOURCES)

# ##############################################################################
# Targets
# ##############################################################################
add_executable(
  precalc EXCLUDE_FROM_ALL
  libcrc/precalc/crc32_table.c libcrc/precalc/crc64_table.c
  libcrc/precalc/precalc.c
)
target_include_directories(precalc PRIVATE libcrc/precalc libcrc/include)

add_custom_target(
  generate_tabs
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND $<TARGET_FILE:precalc> --crc32 libcrc/tab/gentab32.inc
  COMMAND $<TARGET_FILE:precalc> --crc64 libcrc/tab/gentab64.inc
  BYPRODUCTS libcrc/tab/gentab32.inc libcrc/tab/gentab64.inc
)

# Pre-generated tabs
file(COPY gentab32.inc gentab64.inc DESTINATION libcrc/tab)

# Add Library
target_sources(firmware.elf PRIVATE ${SOURCES})
target_include_directories(firmware.elf PUBLIC libcrc/include)
