cmake_minimum_required(VERSION 3.14)
project(racketpoint_scoring VERSION 0.2.0 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(racketpoint_scoring SHARED
  "../padel_scoring.c"
  "../rp_ffi.c"
)
target_include_directories(racketpoint_scoring PUBLIC "..")
set_target_properties(racketpoint_scoring PROPERTIES
  OUTPUT_NAME "racketpoint_scoring"
  C_VISIBILITY_PRESET hidden
)
target_compile_definitions(racketpoint_scoring PRIVATE DART_SHARED_LIB)
