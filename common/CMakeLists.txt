cmake_minimum_required(VERSION 3.20)

project(PlatformCommon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(msgpack-cxx REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)

set(CMAKE_AUTOMOC ON)

set(H_FILES
    src/singleton.h
    src/SettingsManager.h
    src/model/serialize.h
    src/model/EEGData.h
    src/model/WristbandData.h
)

set(CPP_FILES
    src/SettingsManager.cpp
    src/model/EEGData.cpp
    src/model/WristbandData.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${H_FILES} ${CPP_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/model
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC msgpack-cxx Qt6::Core
)