cmake_minimum_required(VERSION 3.14)
project(racketpoint_scoring C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Core library ──────────────────────────────────────────────────────────────
add_library(racketpoint_scoring STATIC padel_scoring.c)
target_include_directories(racketpoint_scoring PUBLIC .)

# ── Test executable ───────────────────────────────────────────────────────────
add_executable(test_padel_scoring tests/test_padel_scoring.c)
target_link_libraries(test_padel_scoring racketpoint_scoring)

# Warn aggressively for the test build
target_compile_options(test_padel_scoring PRIVATE -Wall -Wextra -Wpedantic)

# ── Install (for Android CMake integration) ────────────────────────────────
install(TARGETS racketpoint_scoring ARCHIVE DESTINATION lib)
install(FILES scoring.h padel_scoring.h DESTINATION include)
