cmake_minimum_required(VERSION 3.18)

# ##############################################################################
# Sources
# ##############################################################################
aux_source_directory(src SOURCES)

# ##############################################################################
# Targets
# ##############################################################################
add_executable(
  precalc EXCLUDE_FROM_ALL
  precalc/crc32_table.c precalc/crc64_table.c
  precalc/precalc.c
)
target_include_directories(precalc PRIVATE precalc include)

add_custom_target(
  generate_tabs
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND $<TARGET_FILE:precalc> --crc32 tab/gentab32.inc
  COMMAND $<TARGET_FILE:precalc> --crc64 tab/gentab64.inc
  BYPRODUCTS tab/gentab32.inc tab/gentab64.inc
)

# Pre-generated tabs
file(COPY gentab32.inc gentab64.inc DESTINATION tab)

# Add Library
target_sources(HostService PRIVATE ${SOURCES})
target_include_directories(HostService PUBLIC include)
