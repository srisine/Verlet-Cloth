cmake_minimum_required(VERSION 3.23)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(engine VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(VCPKG_APPLOCAL_DEPS OFF)

add_executable(engine
    src/main.c
)

# ---- Packages ----
find_package(SDL3 CONFIG REQUIRED)
# ---- Linking ----
target_link_libraries(engine PRIVATE SDL3::SDL3)
