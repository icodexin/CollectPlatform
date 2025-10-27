cmake_minimum_required(VERSION 3.20)
project(CollectPlatform)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../output/bin/${CMAKE_BUILD_TYPE}")
set(THIRD_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../output/3rdparty")

add_subdirectory(3rdparty)
add_subdirectory(common)
add_subdirectory(coapp)
add_subdirectory(muapp)