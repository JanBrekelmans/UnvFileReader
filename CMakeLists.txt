cmake_minimum_required(VERSION 3.14...3.22)

project(
    UnvFileReader
    VERSION 0.0.1
    LANGUAGES CXX
)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Add dependencies via CPM ----
include(cmake/CPM.cmake)
CPMAddPackage("gh:doctest/doctest@2.4.9")
find_package(doctest REQUIRED)

# ---- Add the matrix library ----
enable_testing()
file(GLOB_RECURSE headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp)
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

add_library(${PROJECT_NAME} STATIC ${headers} ${sources})

target_include_directories(${PROJECT_NAME}
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_link_libraries(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# ---- Add the main executable ----
add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/application/main.cpp)
target_link_libraries(main ${PROJECT_NAME})
set_property(TARGET main PROPERTY CXX_STANDARD 20 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# ---- Add testing ----
set(TEST_PROJECT_NAME ${PROJECT_NAME}Test)
add_executable(${TEST_PROJECT_NAME} ${test_sources})
target_link_libraries(${TEST_PROJECT_NAME} doctest::doctest ${PROJECT_NAME})
set_property(TARGET ${TEST_PROJECT_NAME} PROPERTY CXX_STANDARD 20)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${TEST_PROJECT_NAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# ----Code coverage ----
if(ENABLE_TEST_COVERAGE)
  target_compile_options(${TEST_PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${TEST_PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
endif()