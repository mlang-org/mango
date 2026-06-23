cmake_minimum_required(VERSION 3.28)

project(mango
    VERSION 0.1.0
    DESCRIPTION "The MLang package manager"
    HOMEPAGE_URL "https://github.com/mlang-org/mango"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MANGO_BUILD_TESTS "Build the mango test suite" ON)
option(MANGO_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

add_library(mango_core STATIC
    src/SemVer.cpp
    src/Json.cpp
    src/Manifest.cpp)
target_include_directories(mango_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(NOT MSVC)
    target_compile_options(mango_core PRIVATE -Wall -Wextra -Wpedantic)
    if(MANGO_WARNINGS_AS_ERRORS)
        target_compile_options(mango_core PRIVATE -Werror)
    endif()
endif()

add_executable(mango src/main.cpp)
target_link_libraries(mango PRIVATE mango_core)
target_compile_definitions(mango PRIVATE MANGO_VERSION="${PROJECT_VERSION}")
install(TARGETS mango RUNTIME DESTINATION bin)

if(MANGO_BUILD_TESTS)
    enable_testing()
    add_executable(mango_tests tests/test_main.cpp)
    target_link_libraries(mango_tests PRIVATE mango_core)
    add_test(NAME mango_tests COMMAND mango_tests)
endif()

message(STATUS "mango ${PROJECT_VERSION} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION})")
