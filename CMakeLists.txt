cmake_minimum_required(VERSION 3.20)
project(CollectPlatform)

if (NOT BINARY_OUT_DIRECTORY)
    set(BINARY_OUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../output")
endif()

set(APP_OUTPUT_DIRECTORY "${BINARY_OUT_DIRECTORY}/app/$<CONFIG>")

list(APPEND CMAKE_PREFIX_PATH "${BINARY_OUT_DIRECTORY}/3rdparty")

add_subdirectory(common)
add_subdirectory(coapp)
add_subdirectory(muapp)