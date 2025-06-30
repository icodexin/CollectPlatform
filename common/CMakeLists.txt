cmake_minimum_required(VERSION 3.20)

project(PlatformCommon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(msgpack-cxx REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)

set(H_FILES
    model/serialize.h
    model/EEGData.h
    model/WristbandData.h
#    model/MsgpackQBufferAdapter.h
)

set(CPP_FILES
    model/EEGData.cpp
    model/WristbandData.cpp
#    model/MsgpackQBufferAdapter.cpp
)

add_library(common STATIC
    ${H_FILES} ${CPP_FILES}
)

target_include_directories(common
    PUBLIC ${PROJECT_SOURCE_DIR}
)

target_link_libraries(common
    PUBLIC msgpack-cxx Qt6::Core
)