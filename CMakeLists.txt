cmake_minimum_required(VERSION 3.20)
project(SimbaTickPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(src/lib)
add_subdirectory(src/app)

include(CTest)
enable_testing()
add_subdirectory(src/tests)