cmake_minimum_required(VERSION 3.20)
project(CollectPlatform)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}")

add_subdirectory(3rdparty)
add_subdirectory(common)
add_subdirectory(APP_Teacher)
add_subdirectory(APP_Student)