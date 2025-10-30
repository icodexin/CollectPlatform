cmake_minimum_required(VERSION 3.20)
project(CollectPlatform)

set(APP_OUTPUT_DIRECTORY "${BINARY_OUT_DIRECTORY}/bin/${CMAKE_BUILD_TYPE}")

list(APPEND CMAKE_PREFIX_PATH "${BINARY_OUT_DIRECTORY}/3rdparty")

add_subdirectory(common)
add_subdirectory(coapp)
add_subdirectory(muapp)